#pragma once
#include <future>
#include <optional>

template <class T>
bool is_ready(const std::future<T>& f) {
	return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template <class T>
bool valid_and_is_ready(const std::future<T>& f) {
	return f.valid() && is_ready(f);
}

template <class T>
bool is_ready(const std::shared_future<T>& f) {
	return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}