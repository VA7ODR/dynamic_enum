/*
MIT License

Copyright (c) 2023 James Baker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <typeinfo>
#include <any>
#include <unordered_set>
#include <variant>
#include <ostream>
#include <sstream>

namespace dynamic_enum
{
	std::string fix_name(const std::type_info &info, size_t suffixLen = 0);

	class tuple;
	typedef std::function<std::ostream&(std::ostream&, const tuple &item)> streamerType;
	std::ostream & default_streamer(std::ostream &out, const tuple &item);
	struct tuple {
			size_t first;
			std::string second;
			mutable std::any third;
			mutable streamerType streamer = default_streamer;

			tuple(size_t f, const std::string& s, const std::any& t) : first(f), second(s), third(t) {}

			std::string type_name() const;

			struct Hash {
					std::size_t operator()(const tuple& tuple) const;
			};

			struct Equal {
					bool operator()(const tuple& lhs, const tuple& rhs) const;
			};
	};

	bool operator==(const tuple& lhs, const tuple& rhs);
}

namespace std
{
	// Template specialization for std::unordered_set and dynamic_enum::tuple
	template <>
	struct std::hash<dynamic_enum::tuple> {
			std::size_t operator()(const dynamic_enum::tuple& tuple) const {
				return dynamic_enum::tuple::Hash()(tuple);
			}
	};
}

namespace dynamic_enum
{

	class set
	{
		protected:
			friend class set_ref;
			std::unordered_set<tuple> items;
			mutable std::shared_mutex mtx;
	};

	class set_ref
	{
		public:
			set_ref(set & in, bool bShared) : m_set(in), lock(bShared ? std::make_unique<std::shared_lock<std::shared_mutex>>(in.mtx) : std::make_unique<std::shared_lock<std::shared_mutex>>(in.mtx)) {}
			set_ref(const set_ref & rhs) : m_set(rhs.m_set), lock(rhs.lock.index() == 0 ? std::make_unique<std::shared_lock<std::shared_mutex>>(rhs.m_set.mtx) : std::make_unique<std::shared_lock<std::shared_mutex>>(rhs.m_set.mtx)) {}
			set_ref(set_ref && rhs) : m_set(rhs.m_set), lock(std::move(rhs.lock)) {}
			operator set&() { return m_set; }

			const tuple & operator[](size_t index);
			const tuple & operator[](const std::string& index);

			auto insert(const tuple& in)
			{
				return m_set.items.insert(in);
			}

			auto begin()
			{
				return m_set.items.begin();
			}

			auto end()
			{
				return m_set.items.end();
			}

		protected:

			set & m_set;
			std::variant<std::unique_ptr<std::shared_lock<std::shared_mutex>>, std::unique_ptr<std::unique_lock<std::shared_mutex>>> lock;
	};

	set_ref registry(bool bShared = true);

	class basic_item {
	public:
		basic_item() = default;

		const std::string& name() const;
		std::string type_name() const;
		std::any value() const;

		template <class T>
		std::optional<T> value() const
		{
			try {
				return std::any_cast<T>(registry()[index].third);
			} catch (const std::bad_any_cast& e) {
				return std::nullopt;
			}
		}

		void set_value(const std::any & a, streamerType streamerIn = default_streamer);

		std::ostream& operator<<(std::ostream& out)
		{
			auto reg = registry();
			auto item = reg[index];
			return item.streamer(out, item);
		}

		std::string string()
		{
			std::ostringstream out;
			auto reg = registry();
			auto item = reg[index];
			return out.str();
		}

		operator const std::string&();

	protected:
		friend std::ostream& operator<<(std::ostream& out, const basic_item & in);
		basic_item(const std::type_info & info, streamerType streamerIn = default_streamer, size_t suffuxLen = 0);
		basic_item(const std::type_info & info, const std::any & value, streamerType streamerIn = default_streamer, size_t suffuxLen = 0);

		size_t index{typeid(*this).hash_code()};
	};

	std::ostream& operator<<(std::ostream& out, const basic_item & in);
	std::ostream& operator<<(std::ostream& out, const tuple & in);
}

#define DynamicEnum(X, Y) namespace X { Y } using namespace X;

#define DynamicEnumItem(X) struct X##_t : public dynamic_enum::basic_item \
	{ \
		X##_t() : dynamic_enum::basic_item(typeid(*this), dynamic_enum::default_streamer, 2) {} \
		X##_t(const std::any & v) : dynamic_enum::basic_item(typeid(*this), v, dynamic_enum::default_streamer, 2) {} \
	}
#define DynamicEnumItemInstance(X, Y) DynamicEnumItem(X) X {Y};

#define DynamicEnumStreamer(W) [](std::ostream& out, const dynamic_enum::tuple &item) -> std::ostream& \
	{ \
		W value; \
		try { \
			value = std::any_cast<W>(item.third); \
		} catch (const std::bad_any_cast& e) { \
			return out << "Invalid Type conversion: std::any<" << dynamic_enum::fix_name(item.third.type()) << " to " << #W; \
		} \
		if constexpr (std::is_same_v<decltype(out << value), std::ostream&>) { \
			return out << value; \
		} else { \
			return out << "Unstreamable Type: " << #W; \
		} \
	}

#define DynamicTypedEnumItem(W, X) \
	struct X##_t : public dynamic_enum::basic_item \
	{ \
		X##_t() : dynamic_enum::basic_item(typeid(*this), DynamicEnumStreamer(W), 2) {} \
		X##_t(const W & v) : dynamic_enum::basic_item(typeid(*this), v, DynamicEnumStreamer(W), 2) {} \
		X##_t& operator=(const W & a) { set_value(a); return *this; } \
		operator W() { return std::any_cast<W>(value()); } \
	}
#define DynamicTypedEnumItemInstance(W, X, Y) DynamicTypedEnumItem(W, X) X {Y};

