#include <array>
#include <cerrno>
#include <chrono>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "event_counter.h"

typedef EventCounter<16, 4> Counter;
typedef Counter::Result Result;
typedef std::array<char, 16> Key;

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


	std::array<std::string, 4> keys;
	keys[0] = "abc";
	keys[1] = "abd";
	keys[2] = "abe";
	keys[3] = "abf";

	int i = 0;
	for (;;) {
		Key key = to_key(keys[i%4]);
		ec->registerEvent(key, i, i + 1);
		ec->registerEvent(key, i , i);
		ec->registerEvent(key, i, i + 1);
		if (i % 50 == 0) {
			Result result;
			ec->query(0, 3, result);
			printf("i = %u\n", i);
			ec->print();
			printf("Query result: ");
			result.print();
		}
		if (i % 133 == 0) {
			std::vector<std::pair<Key, Result>> results;
			ec->getTopKeys(results, 2, 2);
			printf("TOP:i = %u\n", i);
			ec->print();
			printf("Query result:\n");
			for (auto &el : results) {
				printf("\t%s : ", el.first.data());
				el.second.print();
			}
		}
		auto start = std::chrono::system_clock::now();
		for (int j = 0; j < 100000; ++j) {
			ec->registerEvent(key, i, i + 1);
			ec->registerEvent(key, i , i);
			ec->registerEvent(key, i, i + 1);	
		}
		auto end = std::chrono::system_clock::now();
		auto diff = end - start;
		double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
		printf("%fms for 100 000 iterations\n", elapsed);
		i++;
		return 0;
	}

	return 0;
}
