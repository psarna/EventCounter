#include "sliding.h"

typedef Sliding<long, 16, std::less<long>> SlidingMax;
typedef Sliding<long, 16, std::greater<long>> SlidingMin;

int main() {
	SlidingMax smax;
	SlidingMin smin;

	for (int i : {5, 11, 16, 9, 24}) {
		smax.update(i);
		smax.print();
	}

	printf("---------------\n");

	for (int i : {9, 8, 7, 5, 4, 3, 6, 5, 7, 2}) {
		smin.update(i);
		smin.print();
	}

}
