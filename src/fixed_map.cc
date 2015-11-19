#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>

#include "fixed_map.h"


int main() {
	History_buffer<> buf;
	buf.print();
	for (auto i : {1, 2, 2, 3, 4, 0, 2, 3, 1} ) {
		buf.insert(i);
		assert(buf.head() == i);
		buf.print();
	}
	Fixed_map<char, 3> map;
	for (auto i : {'0', '1', '3', '2', '1', '2', '3', '9'} ) {
		map.print();
		map.put(i);
		printf("m[%c]=%d\n", i, map.get(i));
		map.print();
		printf("############################\n");
	}
}
