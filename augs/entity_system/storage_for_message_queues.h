#pragma once
#include <tuple>
#include <vector>
#include "templates.h"

namespace detail {
	template<class T>
	struct make_vector { typedef std::vector<T> type; };
}

namespace augs {
	template<class... Queues>
	class storage_for_message_queues {
		std::unordered_map<size_t, std::vector<std::function<void()>>> message_callbacks;
		typename transform_types<std::tuple, ::detail::make_vector, Queues...>::type queues;
	public:
		
		template <typename T>
		void post(const T& message_object) {
			get_queue<T>().push_back(message_object);

			for (auto& callback : message_callbacks[typeid(T).hash_code()])
				callback();
		}

		template <typename T>
		std::vector<T>& get_queue() {
			return queues.get<T>();
		}

		template <typename T>
		void delete_marked(std::vector<T>& messages) {
			messages.erase(std::remove_if(messages.begin(), messages.end(), [](const T& msg) {
				return msg.delete_this_message;
			}), messages.end());
		}

		template <typename T>
		void delete_marked() {
			delete_marked_messages(get_queue<T>());
		}

		template<class T>
		void register_callback(std::function<void()> callback) {
			message_callbacks[typeid(T).hash_code()].push_back(callback);
		}
	};
}