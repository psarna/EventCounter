#ifndef __SLIDING_H
#define __SLIDING_H

#include <array>
#include <cassert>

template<typename Value, int N, typename Comp = std::less<Value>>
class static_list {
public:
	struct node {
		node() : value(), next(kNull) {}

		Value value;
		int prev;
		int next;
	};

	static_list() : nodes_(), size_(0), head_(0), free_head_(0) {
		for (int i = 0; i < N; ++i) {
			nodes_[i].next = i + 1;
		}
	}

	void push_front(const Value &value) {
		int index = get_free();

		if (size_ == 0) {
			head_ = index;
			nodes_[head_].value = value;
			nodes_[head_].next = kNull;
			tail_ = index;
		} else {
			nodes_[index].value = value;
			nodes_[index].next = head_;
			head_ = index;
		}
		size_++;
	}

	void push_back(const Value &value) {
		int index = get_free();

		if (size_ == 0) {
			head_ = index;
			nodes_[head_].value = value;
			nodes_[head_].next = kNull;
			tail_ = index;
		} else {
			nodes_[index].value = value;
			nodes_[tail_].next = index;
			tail_ = index;
			nodes_[tail_].next = kNull;
		}
		size_++;
	}

	void pop_front() {
		assert(size_ > 0);
		if (size_ == 1) {
			add_free(head_);
			head_ = kNull;
		} else {
			int prev_head = nodes_[head_].next;
			add_free(head_);
			head_ = prev_head;
			
		}
		size_--;
	}

	void pop_until(const Value &value) {
		while (size_ > 0 && Comp()(front(), value)) {
			pop_front();
		}
	}

	void insert_sorted(const Value &value) {
		int last = kNull;
		int current = head_;

		if (size_ == 0) {
			return push_front(value);
		}

		while (current != kNull && Comp()(nodes_[current].value, value)) {
			last = current;
			current = nodes_[current].next;
		}

		int index = get_free();
		nodes_[index].value = value;

		// Inserting to the back
		if (current == kNull) {
			tail_ = index;
			nodes_[index].next = kNull;
			nodes_[last].next = index;
		} else {
			nodes_[index].next = current;
			// Inserting to the front
			if (last == kNull) {
				head_ = index;
			// Inserting in the middle
			} else {
				nodes_[last].next = index;
			}
		}
		size_++;
	}

	const Value &front() const {
		return nodes_[head_].value;
	}

	const Value &back() const {
		return nodes_[tail_].value;
	}

	const int size() const {
		return size_;
	}

	void print() const {
		int current = head_;
		while (current != kNull) {
			printf("(%d)", nodes_[current].value);
			current = nodes_[current].next;
		}
		printf("(null)\n");
	}
private:
	inline int get_free() {
		assert(free_head_ != kNull);
		int index = free_head_;
		free_head_ = nodes_[free_head_].next;	
		return index;
	}

	inline void add_free(int index) {
		nodes_[index].next = free_head_;
		free_head_ = index;	
	}

	static const int kNull = N;

	std::array<struct node, N> nodes_;
	int size_;
	int head_;
	int tail_;
	int free_head_;
};

template<typename Value, int N, typename Comp>
class Sliding {
public:

	Value top() const {
		assert(data_.size() > 0);
		return data_.back();
	}

	void update(const Value &value) {
		remove(value);
		return data_.push_front(value);
	}

	void print() const {
		data_.print();
		printf("Top = %d\n", this->top());
	}

private:
	void remove(const Value &value) {
		data_.pop_until(value);
		while (data_.size() > 0 && data_.front() == value) {
			data_.pop_front();
		}
	}

	static_list<Value, N, Comp> data_;
};

#endif
