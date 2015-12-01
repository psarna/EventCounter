#include "sliding.h"

typedef Sliding<long, 16, std::less<long>> SlidingMax;
typedef Sliding<long, 16, std::greater<long>> SlidingMin;

int main() {
	SlidingMax smax;
	SlidingMin smin;

	int max = 0;
	for (int i : {5, 11, 16, 9, 24}) {
		max = std::max(i, max);
		smax.update(i);
		smax.print();
		assert(max == smax.top());
	}

	printf("---------------\n");

	int min = 100;
	for (int i : {9, 8, 7, 6, 6, 6, 5, 4, 3, 6, 5, 7, 2}) {
		min = std::min(i, min);
		smin.update(i);
		smin.print();
		assert(smin.top() == min);
	}
	auto x = smin.update(4);
	smin.update(6);
	auto y = smin.update(9);
	smin.print();
	smin.erase(x);
	smin.print();
	auto z = smin.update(7);
	smin.update(7);
	printf("x = %d, y = %d, z = %d\n", x, y, z);
	smin.print();
	smin.erase(y);
	smin.print();
	smin.erase(z);
	smin.print();
}
