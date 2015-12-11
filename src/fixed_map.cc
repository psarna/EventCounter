#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>

#include "fixed_map.h"
#include "immortal_container.h"

int main() {
	lru_buffer<> buf;
	buf.print();
	for (auto i : {1, 2, 2, 3, 4, 0, 2, 3, 1} ) {
		buf.insert(i);
		assert(buf.head() == i);
		buf.print();
	}
	fixed_map<char, int, 3> map;
	for (auto i : {'a', 'b', 'c', 'b', 'a', 'b', 'c', 'f'} ) {
		map.print();
		map.put(i);
		printf("m[%c]=%d\n", i, map.get(i));
		printf("m[%d]=%c\n", 2, map.getKey(2));
		map.print();
		printf("############################\n");
	}

	Immortal<fixed_map<>> immortal_map;
	for (auto i : {'0', '1', '3', '2', '1', '2', '3', '9'} ) {
		printf("m[%c]=%d\n", i, map.get(i));
		immortal_map.print();
		immortal_map.put(i);
		printf("m[%c]=%d\n", i, map.get(i));
		immortal_map.print();
		printf("############################\n");
	}
}
