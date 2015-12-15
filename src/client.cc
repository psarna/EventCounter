#include <array>
#include <cerrno>
#include <chrono>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "event_counter.h"

typedef EventCounter<1024, 32> Counter;
typedef Counter::Result Result;
typedef std::array<char, 8> Key;

Counter *get_counter() {
	int fd = shm_open("/event_counter", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("shm_open failed (%d)\n", errno);
		exit(1);
	}
	
	/* Map shared memory object */
	void *region = mmap(nullptr, sizeof(Counter),
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (region == MAP_FAILED) {
		printf("mmap failed (%d)\n", errno);
		exit(1);
	}

	return (Counter *)region;
}

int main() {
	Counter * ec = get_counter();

	printf("Sizeof counter is %lu\n", sizeof(Counter));	

	int i = 0;
	for (;;) {
		Key key1 = to_key(std::to_string(i % 129));
		Key key2 = to_key(std::to_string(i + 1 % 129));
		Key key3 = to_key(std::to_string(i + 2 % 129));
		//ec->registerEvent(key, i, i + 1);
		//ec->registerEvent(key, i , i);
		//ec->registerEvent(key, i, i + 1);
		//if (i % 50 == 0) {
		//	Result result;
		//	ec->query(0, 3, result);
		//	printf("i = %u\n", i);
		//	ec->print();
		//	printf("Query result: ");
		//	result.print();
		//}
		//if (i % 133 == 0) {
		//	std::vector<std::pair<Key, Result>> results;
		//	ec->getTopKeys(results, 2, 2);
		//	printf("TOP:i = %u\n", i);
		//	ec->print();
		//	printf("Query result:\n");
		//	for (auto &el : results) {
		//		printf("\t%s : ", el.first.data());
		//		el.second.print();
		//	}
		//}
		auto start = std::chrono::system_clock::now();
		for (int j = 0; j < 100000; ++j) {
			ec->registerEvent(key1, i, i + 1);
			ec->registerEvent(key2, i , i);
			ec->registerEvent(key3, i, i + 1);	
			ec->registerEvent(key1, i, i);	
		}
		auto end = std::chrono::system_clock::now();
		auto diff = end - start;
		double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
		printf("%ldms for 400 000 registerEvent\n", (long)elapsed);
		Result result;
		ec->query(key2, 7, result);
		printf("  [%d]: ", i);
		result.print();
		i++;
	}

	return 0;
}
