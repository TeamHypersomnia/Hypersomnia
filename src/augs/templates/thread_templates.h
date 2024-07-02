#pragma once
#include "augs/misc/future.h"
#include <optional>

#if WEB_SINGLETHREAD
template <class F>
decltype(auto) launch_async(F&& func) {
	if constexpr(std::is_same_v<decltype(func()), void>) {
		func();
		return augs::future<void>();
	}
	else {
		return augs::future<decltype(func())> { func() };
	}
}

template <class T>
bool is_ready(const augs::future<T>&) {
	return true;
}

template <class T>
bool valid_and_is_ready(const augs::future<T>& f) {
	return f.valid() && is_ready(f);
}

template <class T>
bool is_free_slot(const augs::future<T>& f) {
	return !f.valid() || is_ready(f);
}
#else
template <class F>
decltype(auto) launch_async(F&& func) {
	return std::async(std::launch::async, std::forward<F>(func));
}

template <class T>
bool is_ready(const std::future<T>& f) {
	return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template <class T>
bool valid_and_is_ready(const std::future<T>& f) {
	return f.valid() && is_ready(f);
}

template <class T>
bool is_free_slot(const std::future<T>& f) {
	return !f.valid() || is_ready(f);
}
#endif
