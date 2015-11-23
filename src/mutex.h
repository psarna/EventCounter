#ifndef __EVENTCOUNTER_MUTEX_H
#define __EVENTCOUNTER_MUTEX_H

#include <stdexcept>
#include <pthread.h>

class Mutex {
public:
	Mutex() {
		int ret = -1;
		printf("thread safety for the win!\n");

		pthread_mutexattr_t mutex_attr;

		if ((ret = pthread_mutexattr_init(&mutex_attr)) != 0)
			throw std::runtime_error("pthreas_mutexattr_init");

		if ((ret = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED)) != 0)
			throw std::runtime_error("PTHREAD_PROCESS_SHARED");
#ifdef __USE_XOPEN2K
		if ((ret = pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST)) != 0)
			throw std::runtime_error("PTHREAD_MUTEX_ROBUST");
#endif

		if ((ret = pthread_mutex_init(&mutex_, &mutex_attr)) != 0) {
			throw std::runtime_error("PTHREAD_MUTEX_ROBUST");
		}

	}

	~Mutex() throw () {
		int ret = -1;

		if ((ret = pthread_mutex_destroy(&mutex_)) != 0) {
			fprintf(stderr, "destroy failed\n");
		}
		printf("mutex destroyed\n");
	}

	void lock() {
		int ret = -1;

		if ((ret = pthread_mutex_lock(&mutex_)) != 0) {

#ifdef __USE_XOPEN2K
			if (ret == EOWNERDEAD)
				ret = pthread_mutex_consistent(&mutex_);
#endif

			if (ret != 0)
				fprintf(stderr, "lock failed\n");
		}
		printf("mutex locked\n");
	}

	bool trylock() {
		int ret = -1;

		if ((ret = pthread_mutex_trylock(&mutex_)) != 0) {

			if (ret == EBUSY)
				return false;

#ifdef __USE_XOPEN2K
			if (ret == EOWNERDEAD)
				ret = pthread_mutex_consistent(&mutex_);
#endif
		}

		return true;
	}

	void unlock() {
		int ret = -1;

		if ((ret = pthread_mutex_unlock(&mutex_)) != 0) {
			fprintf(stderr, "unlock failed\n");
		}
		printf("mutex unlocked\n");
	}

private:
	pthread_mutex_t mutex_;
};

#endif
