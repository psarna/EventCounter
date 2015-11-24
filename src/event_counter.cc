#include "event_counter.h"

int main(void) {
	Counter ec;
	printf("\n\n=== registerEvent() tests for new events ===\n\n");
	for (int i = 0; i < 102; ++i) {
		ec.registerEvent(0, i + 3, i + 1);
	}
	ec.print();

	printf("\n\n=== query() tests ===\n\n");
	Result result;

	ec.print();
	ec.query(0, 2, result);
	printf("Query result: ");
	result.print();

	Result res2;
	ec.query(0, 1, res2);

	printf("Query result: ");
	res2.print();

	printf("\n\n=== registerEvent() tests for old events ===\n\n");
	ec.print();
	ec.registerEvent(0, 5, 5);
	ec.registerEvent(0, 1000, 99);
	ec.print();

	Result res3;
	ec.query(0, 3, res3);
	printf("Query result: ");
	res3.print();

	ec.registerEvent(0, 20000, 105);
	ec.print();

	Result res4;
	ec.query(0, 3, res4);
	printf("Query result: ");
	res4.print();

	std::array<char, 16> key1 = {'a', 'b', 'c'};
	std::array<char, 16> key0 = {'a', 'b', 'd'};
	std::array<char, 16> key00 = {'a', 'b', 'a'};

	ec.registerEvent(key1, 4321, 106);
	ec.registerEvent(key0, 1234, 107);
	ec.registerEvent(key1, 9876, 106);

	Mutex mutex;
	mutex.lock();
	ec.registerEvent(key00, 5555, 108);
	ec.print();
	mutex.unlock();

}
