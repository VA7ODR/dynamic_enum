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

#include "dynamic_enum.hpp"
#include <boost/core/demangle.hpp>
#include <typeindex>

namespace dynamic_enum
{
	std::string fix_name(const std::type_info &info, size_t suffixLen)
	{
		std::string ret;
		ret = boost::core::demangle(info.name());
		ret.resize(ret.size() - suffixLen);
		return ret;
	}

	set_ref registry(bool bShared)
	{
		static set reg;
		return set_ref(reg, bShared);
	}

	basic_item::basic_item(const std::type_info &info, streamerType streamerIn, size_t suffuxLen) : index(std::type_index(info).hash_code())
	{
		registry(false).insert(tuple(index, fix_name(info, suffuxLen), std::any()));
	}

	basic_item::basic_item(const std::type_info &info, const std::any &value, streamerType streamerIn, size_t suffuxLen) : index(std::type_index(info).hash_code())
	{
		registry(false).insert(tuple(index, fix_name(info, suffuxLen), value));
	}

	const std::string &basic_item::name() const
	{
		return registry()[index].second;
	}

	std::string basic_item::type_name() const
	{
		return registry()[index].type_name();
	}

	std::any basic_item::value() const
	{
		return registry()[index].third;
	}

	void basic_item::set_value(const std::any &a, streamerType)
	{
		registry(false)[index].third = a;
	}

	basic_item::operator const std::string &()
	{
		return name();
	}

	const tuple &set_ref::operator[](size_t index)
	{
		auto it = m_set.items.find(tuple(index, "", std::any()));
		if (it == m_set.items.end()) {
			it = m_set.items.insert(tuple(index, "", std::any())).first;
		}
		return *it;
	}

	const tuple &set_ref::operator[](const std::string &index)
	{
		auto it = m_set.items.find(tuple(0, index, std::any()));
		if (it == m_set.items.end()) {
			it = m_set.items.insert(tuple(0, index, std::any())).first;
		}
		return *it;
	}

	std::size_t tuple::Hash::operator()(const tuple &tuple) const
	{
		return tuple.first ? std::hash<size_t>()(tuple.first) : (std::hash<std::string>()(tuple.second) << 1);
	}

	bool tuple::Equal::operator()(const tuple &lhs, const tuple &rhs) const
	{
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}

	bool operator==(const tuple &lhs, const tuple &rhs)
	{
		const size_t mx = std::numeric_limits<size_t>::max();
		if (lhs.first < mx && rhs.first < mx) {
			return lhs.first == rhs.first;
		}
		return lhs.second == rhs.second;
	}

	std::string tuple::type_name() const
	{
		return fix_name(third.type());
	}

	std::ostream &operator<<(std::ostream &out, const basic_item &in)
	{
		auto reg = registry();
		auto item = reg[in.index];
		return item.streamer(out, item);
	}

	std::ostream& operator<<(std::ostream& out, const tuple & item)
	{
		return item.streamer(out, item);
	}

	std::ostream &default_streamer(std::ostream &out, const tuple& item)
	{
		auto value = item.third;
		const std::type_info& info = value.type();
		if (info == typeid(int)) {
			out << std::any_cast<int>(value);
		} else if (info == typeid(unsigned int)) {
			out << std::any_cast<unsigned int>(value);
		} else if (info == typeid(long)) {
			out << std::any_cast<long>(value);
		} else if (info == typeid(unsigned long)) {
			out << std::any_cast<unsigned long>(value);
		} else if (info == typeid(long long)) {
			out << std::any_cast<long long>(value);
		} else if (info == typeid(unsigned long long)) {
			out << std::any_cast<unsigned long long>(value);
		} else if (info == typeid(bool)) {
			out << (std::any_cast<bool>(value) ? "true" : "false");
		} else if (info == typeid(float)) {
			out << std::any_cast<float>(value);
		} else if (info == typeid(double)) {
			out << std::any_cast<double>(value);
		} else if (info == typeid(char)) {
			out << std::any_cast<char>(value);
		} else if (info == typeid(const char*)) {
			out << std::any_cast<const char*>(value);
		} else if (info == typeid(std::string)) {
			out << std::any_cast<std::string>(value);
		} else {
			out << "Unsupported type for default streamer: " << fix_name(info);
		}
		return out;
	}
}
