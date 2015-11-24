
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
int main() {
	shm_unlink("/event_counter");

}
