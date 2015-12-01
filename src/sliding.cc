#include "sliding.h"

typedef Sliding<long, 16, std::less<long>> SlidingMax;
typedef Sliding<long, 16, std::greater<long>> SlidingMin;

int main() {
	SlidingMax smax;
	SlidingMin smin;

	for (int i : {5, 9, 11, 16, 24}) {
		smax.update(i);
		smax.print();
	}
	smax.remove(11);
	smax.print();
	smax.update(33);
	smax.print();
	smax.remove(16);
	smax.print();

	printf("---------------\n");

	for (int i : {9, 8, 7, 6, 6, 6,  5, 5, 4}) {
		smin.update(i);
		smin.print();
	}

	smin.remove(6);
	smin.print();
	smin.update(3);
	smin.update(2);
	smin.update(1);
	smin.print();
	smin.remove(2);
	smin.print();
	smin.remove(1);
	smin.update(19);
	smin.print();
	smin.update(14);
	smin.print();
}
