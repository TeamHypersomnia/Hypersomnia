#pragma once
#include <tuple>
#include <vector>
#include <unordered_map>
#include "templates.h"
#include "ensure.h"

namespace detail {
	template<class T>
	struct make_vector { typedef std::vector<T> type; };
}

namespace augs {
	template<class... Queues>
	class storage_for_message_queues {
		std::unordered_map<size_t, std::vector<std::function<void()>>> message_callbacks;
		typedef typename transform_types<std::tuple, ::detail::make_vector, Queues...>::type tuple_type;
		tuple_type queues;

		struct ensure_empty {
			storage_for_message_queues& q;

			template <class Q>
			void operator()() {
				ensure(q.get_queue<Q>().empty());
			}
		};
	public:
		
		template <typename T>
		void post(const T& message_object) {
			get_queue<T>().push_back(message_object);

			for (auto& callback : message_callbacks[typeid(T).hash_code()])
				callback();
		}

		template <typename T>
		std::vector<T>& get_queue() {
			return std::get<std::vector<T>>(queues);
		}

		template <typename T>
		void clear_queue() {
			return get_queue<T>().clear();
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

		void ensure_all_empty() {
			for_each_type<Queues...>(ensure_empty(*this));
		}
	};
}