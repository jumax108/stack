#pragma once

#include <Windows.h>

template <typename T>
class CStack {

public:

	CStack(unsigned int capacity);
	~CStack();

	inline unsigned int size() {
		return _size;
	}

	inline unsigned int capacity() {
		return _capacity;
	}

	bool push(T _data);
	bool pop();
	bool front(T* out);

private:

	HANDLE _heap;
	T* _data;
	unsigned int _size;
	unsigned int _capacity;

};

template<typename T>
CStack<T>::CStack(unsigned int capacity) {

	_heap = HeapCreate(0, 0, 0);
	_data = (T*)HeapAlloc(_heap, HEAP_ZERO_MEMORY, sizeof(T) * capacity);
	_capacity = capacity;
	_size = 0;

}

template<typename T>
CStack<T>::~CStack() {
	HeapFree(_heap, 0, _data);
	HeapDestroy(_heap);
}

template<typename T>
bool CStack<T>::push(T data) {

	if (_size == _capacity) {
		return false;
	}

	_data[_size] = data;
	_size += 1;

	return true;

}

template<typename T>
bool CStack<T>::pop() {

	if (_size == 0) {
		return false;
	}

	_size -= 1;

	return true;

}

template<typename T>
bool CStack<T>::front(T* out) {

	if (_size == 0) {
		return false;
	}
	
	*out = _data[_size - 1];

	return true;

}