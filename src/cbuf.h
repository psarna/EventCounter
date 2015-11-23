#ifndef _CBUF_H
#define _CBUF_H

#include <array>
#include <cassert>
#include <cstring>

template<typename T, long Size>
struct cbuf {
	std::array<T, Size> data;
	static const long size = Size;
	long start;
	long end;

	cbuf() : data(), start(0), end(0) {
	}

	cbuf(long size) : data(), start(0), end(size) {
		assert(size >= 0 && size < Size);
	}

	int empty() {
		return start == end;
	}

	int full() {
		return ((end + 1) % Size) == start;
	}

	void put(const T &value) {
		data[end] = value;
		end = (end + 1) % Size;
	}

	T &get() {
		T &value = data[start];
		start = (start + 1) % Size;
		return value;
	}

	T &operator[](long pos) {
		assert(pos >= 0 && pos < Size);
		pos = (end + Size - pos - 1) % Size;
		return data[pos];
	}

	const T &operator[](long pos) const {
		assert(pos >= 0 && pos < Size);
		pos = (end + Size - pos - 1) % Size;
		return data[pos];
	}
};

#endif
