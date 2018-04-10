#pragma once
#include "augs/ensure.h"
#include "augs/misc/relinked_pool_id.h"

namespace augs {
	struct introspection_access;

	template <class T, template <class> class make_container_type>
	class id_relinking_pool {
		using underlying_key_type = unsigned;

	public:
		using key_type = relinked_pool_id<underlying_key_type>;
		using mapped_type = T;

	private:
		using container_type = make_container_type<T>;

		// GEN INTROSPECTOR class augs::id_relinking_pool class T template<class>class C
		container_type objects;
		// END GEN INTROSPECTOR

		static auto relink(
			const key_type from,
			const key_type to
		) {
			return [from, to](key_type& key) {
				if (key == from) {
					key = to;
				}
			};
		}

		static auto negative_of(const key_type key) {
			using K = underlying_key_type;

			/* -1 is reserved for a never-set key */

			return static_cast<K>(-2) - underlying_key_type(key);
		}

		auto id_of_last() const {
			return key_type(size() - 1);
		}

		template <class F>
		void free_last(F for_each_existent_id) {
			const auto erased_id = id_of_last();
			objects.pop_back();

			for_each_existent_id(relink(erased_id, negative_of(erased_id)));
		}

	public:
		struct allocation_result {
			key_type key;
			mapped_type& object;

			operator key_type() const {
				return key;
			}
		};

		template <class F, class... Args>
		allocation_result allocate(
			F for_each_existent_id,
			Args&&... args
		) {
			const auto allocated_id = static_cast<key_type>(size());
			const auto neg_allocated_id = negative_of(allocated_id);

			/* 
				It might happen that, after an undo, there are some ids whose positives
				would point to this newly allocated object. However, if this object is now freed
				and later this free is undone, those ids would start pointing to this new id out of nowhere. 

				Thus we must clear these ids to invalid ones.
			*/

			for_each_existent_id(relink(neg_allocated_id, {}));

			objects.emplace_back(std::forward<Args>(args)...);

			return { allocated_id, objects.back() };
		}

		template <class F>
		void undo_last_allocate(F&& for_each_existent_id) {
			free_last(std::forward<F>(for_each_existent_id));
		}

		template <class F>
		void free(F&& for_each_existent_id, const key_type key) {
			if (key == id_of_last()) {
				const auto erased_id = id_of_last();
				objects.pop_back();

				for_each_existent_id(relink(erased_id, negative_of(erased_id)));
			}
			else {
				const auto removed_id = key;
				const auto id_that_was_moved = id_of_last();

				const auto neg_id_that_was_moved = negative_of(id_that_was_moved);
				const auto neg_removed_id = negative_of(removed_id);

				objects[removed_id] = std::move(objects[id_that_was_moved]);
				objects.pop_back();

				for_each_existent_id([removed_id, neg_removed_id, id_that_was_moved, neg_id_that_was_moved](key_type& key) {
					if (key == removed_id) {
						/* 
							We set the key of the removed to the negative of what it was, 
							so that on undoing the remove, we will know to reset this key back to the removed_id. 
						*/
						key = neg_id_that_was_moved;
					}
					else if (key == id_that_was_moved) {
						key = removed_id;
					}	
				});
			}
		}

		template <class F, class... Args>
		void undo_free(
			F&& for_each_existent_id,
			const key_type id_of_undone,
			Args&&... removed_content
		) {
			static_assert(sizeof...(Args) > 0, "Specify what to construct.");

			const auto id_of_previous_last = 
				key_type(static_cast<underlying_key_type>(size()))
			;

			LOG_NVPS(id_of_undone, id_of_previous_last);

			if (id_of_undone == id_of_previous_last) {
				/* It was the last one. We need to undo the effect of free_last. */
				const auto neg_id_of_undone = negative_of(id_of_undone);

				for_each_existent_id(relink(neg_id_of_undone, id_of_undone));

				objects.emplace_back(std::forward<Args>(removed_content)...);
			}
			else {
				/* We need to undo the move and pop that occured. */
				{
					auto& new_object_space = objects[id_of_undone];
					objects.emplace_back(std::move(new_object_space));

					new (std::addressof(new_object_space)) mapped_type(std::forward<Args>(removed_content)...);
				}

				const auto new_id = id_of_last();
				const auto neg_new_id = id_of_last();
				const auto neg_id_of_undone = negative_of(id_of_undone);

				for_each_existent_id([id_of_undone, neg_id_of_undone, new_id, neg_new_id](key_type& key) {
					if (key == neg_new_id) {
						key = id_of_undone;
					}
					else if (key == id_of_undone) {
						key = new_id;
					}	
				});
			}
		}

		auto& get(const key_type k) {
			return objects[k];
		}

		const auto& get(const key_type k) const {
			return objects[k];
		}

		mapped_type* find(const key_type k) {
			if (k) {
				return std::addressof(objects[k]);
			}

			return nullptr;
		}

		const mapped_type* find(const key_type k) const {
			if (k) {
				return std::addressof(objects[k]);
			}

			return nullptr;
		}

		auto size() const {
			return objects.size();
		}

		auto max_size() const {
			return objects.max_size();
		}

		auto capacity() const {
			return objects.capacity();
		}

		auto empty() const {
			return objects.empty();
		}

		auto full() const {
			return size() == max_size();
		}
	};
}
