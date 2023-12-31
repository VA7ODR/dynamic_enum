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

#include <iostream>
#include <string>
#include "dynamic_enum.hpp"

DynamicTypedEnumItemInstance(std::string, Bob, "Default");
DynamicEnumItem(Ron);

namespace Test
{
	DynamicEnumItemInstance(Fred, "George");
}

DynamicEnum(Oooo,
	DynamicEnumItemInstance(Computer, "Dell")
	DynamicEnumItemInstance(Laptop, 12345)
);

using namespace Test;

class RealTest
{
	public:
		bool operator[](const std::string & index)
		{
			std::cout << index << " " << (stuff.find(index) != stuff.end()) << std::endl;
			return stuff.find(index) != stuff.end();
		}
		std::unordered_set<std::string> stuff;
};

struct AnotherTest
{
		DynamicEnumItemInstance(Dog, "Cat");
} anotherTest;

namespace dynamic_enum
{
	std::string fix_name(const std::type_info &info, size_t suffixLen);
}

int main() {
	Fred_t();
	Ron_t ron("hungry");
	// ron = "lol";
	Bob = "uncle";

	RealTest t;
	t.stuff.insert(Ron_t());
	if (!t[Bob]) {
		std::cout << "Not Found" << std::endl;
	}
	if (!t[Ron_t()]) {
		std::cout << "Not Found" << std::endl;
	}

	for (auto & item : dynamic_enum::registry())
	{
		std::cout << item.second << ": " << item.type_name() << ": " << item << std::endl;
	}

	std::cout << Bob.name() << ": " << Bob << std::endl;

	std::cout << ron.name() << ": " << ron << std::endl;
	std::cout << Fred_t().name() << ": " << Fred_t() << std::endl;

	return 0;
}


