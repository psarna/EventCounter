#ifndef __IMMORTAL_CONTAINER_H
#define __IMMORTAL_CONTAINER_H

#include <atomic>

/*
 * Immortal container is a wrapper around data structures
 * used within shared memory regions.
 * If a process dies while performing data modification,
 * data is always recoverable by using a backup structure.
 *
 * Right now values are of trivial types (int, long),
 * so they are returned by value
 */
template<typename Container>
class Immortal {
public:
	typedef Container container_type;
	typedef typename container_type::key_type key_type;
	typedef typename container_type::value_type value_type;

	enum status {
		kOk,          // data: consitent;   backup: consistent
		kDataDirty,   // data: inconsitent; backup: consistent
		kBackupDirty, // data: consitent;   backup: inconsistent
	};

	Immortal() : data_(), backup_(), status_(kOk) {
	}

	value_type get(const key_type &key) {
		repair();
		return data_.get(key);
	}

	void put(const key_type &key) {
		repair();
		status_.store(kDataDirty);
		data_.put(key);
		status_.store(kBackupDirty);
		backup_.put(key);
		status_.store(kOk);
	}

	void print() {
		printf("Status: %s\n",
			status_ == kOk ? "ok" : (status_ == kDataDirty ? "data_dirty" : "backup_dirty"));
		data_.print();
	}

	void repair() {
		switch(status_) {
		case kOk:
			break;
		case kDataDirty:
			data_ = backup_;
			status_.store(kOk);
			break;
		case kBackupDirty:
			backup_ = data_;
			status_.store(kOk);
			break;
		default:
			break;
		}
		assert(status_.load() == kOk);
	}

private:
	container_type data_;
	container_type backup_;
	std::atomic<enum status> status_;
};

#endif