#pragma once
#include <tuple>
#include <vector>

#include "augs/templates/unfold.h"
#include "augs/templates/folded_finders.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/remove_cref.h"

namespace augs {
	struct introspection_access;

	template <class... Queues>
	class storage_for_message_queues {
		template <class Q>
		using make_vector = std::vector<Q>;

		using tuple_type = std::tuple<make_vector<Queues>...>;

		friend introspection_access;

		// GEN INTROSPECTOR class augs::storage_for_message_queues class... Queues
		tuple_type queues;
		// END GEN INTROSPECTOR

		template <class T>
		static void check_valid() {
			static_assert(is_one_of_v<T, Queues...>, "Unknown message type!");
		}

	public:

		template <class T>
		void post(T&& message_object) {
			using M = remove_cref<T>;
			check_valid<M>();
			get_queue<M>().emplace_back(std::forward<T>(message_object));
		}

		template <class T>
		void post(const std::vector<T>& messages) {
			check_valid<T>();
			concatenate(get_queue<T>(), messages);
		}

		template <class T>
		std::vector<T>& get_queue() {
			check_valid<T>();
			return std::get<std::vector<T>>(queues);
		}

		template <class T>
		const std::vector<T>& get_queue() const {
			check_valid<T>();
			return std::get<std::vector<T>>(queues);
		}

		template <class T>
		void clear_queue() {
			check_valid<T>();
			return get_queue<T>().clear();
		}

		void flush_queues() {
			::unfold<make_vector, Queues...>(queues, [&](auto& q) {
				q.clear();
			});
		}

		auto& operator+=(const storage_for_message_queues& b) {
			auto c = [&](auto& q) {
				concatenate(q, std::get<remove_cref<decltype(q)>>(b.queues));
			};

			::unfold<make_vector, Queues...>(queues, c);

			return *this;
		}
	};
}