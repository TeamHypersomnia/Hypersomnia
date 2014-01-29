#pragma once

template <class T>
struct ptr_wrapper {
	T* ptr;
	ptr_wrapper(T* ptr) : ptr(ptr) {}
	operator T*() {
		return ptr;
	}
};