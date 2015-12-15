#include <array>
#include <cerrno>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "event_counter.h"

typedef EventCounter<1024, 32> Counter;
typedef Counter::Result Result;

Counter *create_counter() {
	int fd = shm_open("/event_counter", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("shm_open failed (%d)\n", errno);
		exit(1);
	}
	
	if (ftruncate(fd, sizeof(Counter)) == -1) {
		printf("ftruncate failed (%d)\n", errno);
		exit(1);
	}
	
	void *region = mmap(nullptr, sizeof(Counter),
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (region == MAP_FAILED) {
		printf("mmap failed (%d)\n", errno);
		exit(1);
	}

	return new(region) Counter;
}

int main() {
	Counter *ec = create_counter();

	printf("Sizeof counter is %lu\n", sizeof(Counter));	
	int i = 0;
	for (;;) {
		usleep(1000000);
//
//		ec->registerEvent(key0, i, i + 1);
//		ec->registerEvent(key0, i , i);
//		ec->registerEvent(key0, i, i + 1);
//		if (i % 50 == 0) {
//			Result result;
//			ec->query(0, 3, result);
//			printf("i = %u\n", i);
//			ec->print();
//			printf("Query result: ");
//			result.print();
//		}
//		i++;
	}

	return 0;
}
