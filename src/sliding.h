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
			nodes_[head_].prev = kNull;
			tail_ = index;
		} else {
			nodes_[index].value = value;
			nodes_[index].next = head_;
			nodes_[index].prev = kNull;
			nodes_[head_].prev = index;
			head_ = index;
		}
		size_++;
	}

	void pop_front() {
		assert(size_ > 0);
		if (size_ == 1) {
			add_free(head_);
			head_ = kNull;
		} else {
			int new_head = nodes_[head_].next;
			add_free(head_);
			head_ = new_head;
			nodes_[head_].prev = kNull;
		}
		size_--;
	}

	void erase(int index) {
		assert(index != kNull);

		if (index == free_head_ || index == free_tail_) {
			return;
		}

		int prev_node = nodes_[index].prev;
		int next_node = nodes_[index].next;
		if (prev_node == kNull) {
			head_ = next_node;
		} else {
			nodes_[prev_node].next = next_node;
		}
		if (next_node == kNull) {
			tail_ = prev_node;
		} else {
			nodes_[next_node].prev = prev_node;
		}
		add_free(index);
		size_--;
	}

	void pop_until(const Value &value) {
		while (size_ > 0 && Comp()(front(), value)) {
			pop_front();
		}
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

		if (free_head_ == kNull) {
			free_tail_ = kNull;
		}

		return index;
	}

	inline void add_free(int index) {
		nodes_[index].next = free_head_;
		nodes_[index].prev = kNull;

		if (free_head_ == kNull) {
			free_tail_ = index;
		} else {
			nodes_[free_head_].prev = index;
		}
		free_head_ = index;	
	}

	static const int kNull = N;

	std::array<struct node, N> nodes_;
	int size_;
	int head_;
	int tail_;
	int free_head_;
	int free_tail_;
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
