#pragma once
#include <cstring>
#include <optional>

#if !IS_PRODUCTION_BUILD
#include "augs/ensure.h"
#include "augs/ensure_rel.h"
#endif

#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/container_traits.h"
#include "augs/templates/container_templates.h"

#include "augs/misc/pool/pool_structs.h"
#include "augs/misc/pool/pooled_object_id.h"
#include "augs/templates/per_type.h"

namespace augs {
	template <class T, template <class> class make_container_type, class size_type, class synchronized_array_list = type_list<>, class... id_keys>
	class pool {
	public:
		using value_type = T;

		using mapped_type = T;
		using key_type = pooled_object_id<size_type, id_keys...>;
		using unversioned_id_type = unversioned_id<size_type, id_keys...>;
		using undo_free_input_type = pool_undo_free_input<size_type, id_keys...>;

		using used_size_type = size_type;

		using object_pool_type = make_container_type<mapped_type>;
	protected:
		using pool_slot_type = pool_slot<size_type>;
		using pool_indirector_type = pool_indirector<size_type>;

		static constexpr bool constexpr_max_size = has_constexpr_max_size_v<object_pool_type>;
		static constexpr bool has_synchronized_arrays = !std::is_same_v<synchronized_array_list, type_list<>>;

		make_container_type<pool_slot_type> slots;
		object_pool_type objects;
		make_container_type<pool_indirector_type> indirectors;
		make_container_type<size_type> free_indirectors;
		per_type_container<synchronized_array_list, make_container_type> synchronized_arrays;

		auto& get_indirector(const key_type key) {
			return indirectors[key.indirection_index];
		}

		const auto& get_indirector(const key_type key) const {
			return indirectors[key.indirection_index];
		}

		bool correct_range(const key_type key) const {
			return 
				/* Quickly eliminate fresh ids without fetching indirectors.size() */
				key.indirection_index != static_cast<size_type>(-1) 
				&& key.indirection_index < indirectors.size()
			;
		}

		bool correct_range(const unversioned_id_type key) const {
			return 
				/* Quickly eliminate fresh ids without fetching indirectors.size() */
				key.indirection_index != static_cast<size_type>(-1) 
				&& key.indirection_index < indirectors.size()
			;
		}

		static bool versions_match(const pool_indirector_type& indirector, const key_type& key) {
			return indirector.version == key.version && indirector.real_index != static_cast<size_type>(-1);
		}

#if !IS_PRODUCTION_BUILD
		static auto ensure_versions_match(const pool_indirector_type& indirector, const key_type& key) {
			(void)indirector;
			(void)key;
			ensure_eq(indirector.version, key.version);
			ensure(indirector.real_index != static_cast<size_type>(-1));
		}
#endif

	public:
		pool() : pool(0u) {}

		explicit pool(const size_type slot_count) {
			if constexpr(constexpr_max_size) {
				//static_assert(
					//objects.max_size() <= 
					//std::numeric_limits<size_type>::max() - 1,
					//"The container can hold more elements than the pool can index with size_type!"
				//);
			}

			reserve(slot_count);
		}

		void reserve(const size_type new_capacity) {
			if (new_capacity == static_cast<size_type>(-1)) {
				throw std::runtime_error("Last element index is reserved for signifying unused indirectors.");
			}

			const auto old_capacity = capacity();

			if (new_capacity <= old_capacity) {
				return;
			}

			slots.reserve(new_capacity);
			objects.reserve(new_capacity);

			if constexpr(has_synchronized_arrays) {
				synchronized_arrays.reserve(new_capacity);
			}

			indirectors.resize(new_capacity);
			free_indirectors.reserve(new_capacity);

			for (size_type i = 0; i < (new_capacity - old_capacity); ++i) {
				free_indirectors.push_back(new_capacity - i - 1);
			}
		}

		struct allocation_result {
			key_type key;
			mapped_type& object;

			operator key_type() const {
				return key;
			}
		};

		template <
			unsigned expansion_mult = 2, 
			unsigned expansion_add = 1, 
			class... Args
		>
		allocation_result allocate(Args&&... args);
		void undo_last_allocate(const key_type key);

		auto free(const unversioned_id_type key);

		auto free(const key_type key) -> std::optional<undo_free_input_type>;

		template <class... Args>
		allocation_result undo_free(
			const undo_free_input_type in,
			Args&&... removed_content
		);

		auto get_versioned(const unversioned_id_type key) const {
			key_type ver;
			ver.indirection_index = key.indirection_index;
			ver.version = indirectors[key.indirection_index].version;
			return ver;
		}

		auto find_versioned(const unversioned_id_type key) const {
			key_type ver;

			if (correct_range(key)) {
				ver.indirection_index = key.indirection_index;
				ver.version = indirectors[key.indirection_index].version;
			}

			return ver;
		}

		auto to_id(const size_type real_object_index) const {
			const auto& s = slots[real_object_index];

			key_type id;
			id.indirection_index = s.pointing_indirector;
			id.version = indirectors[s.pointing_indirector].version;

			return id;
		}

	private:
		template <class S>
		static auto& get_impl(S& self, const key_type key) {
#if !IS_PRODUCTION_BUILD
			ensure(self.correct_range(key));
#endif

			const auto& indirector = self.get_indirector(key);

#if !IS_PRODUCTION_BUILD
			ensure_versions_match(indirector, key);
#endif

			return self.objects[indirector.real_index];
		}
		
		template <class S>
		static auto& get_no_check_impl(S& self, const unversioned_id_type key) {
			return self.objects[self.indirectors[key.indirection_index].real_index];
		}

		template <class S>
		static auto find_impl(S& self, key_type key) -> maybe_const_ptr_t<std::is_const_v<S>, mapped_type> {
			if (!self.correct_range(key)) {
				return nullptr;
			}

			const auto& indirector = self.get_indirector(key);

			if (!versions_match(indirector, key)) {
				return nullptr;
			}

			return &self.objects[indirector.real_index];
		}

		template <class S>
		static auto find_no_check_impl(S& self, const unversioned_id_type key) -> maybe_const_ptr_t<std::is_const_v<S>, mapped_type> {
			if (key.is_set()) {
				return &self.objects[self.indirectors[key.indirection_index].real_index];
			}

			return nullptr;
		}

	public:
		mapped_type& get_no_check(const unversioned_id_type key) {
			return get_no_check_impl(*this, key);
		}

		const mapped_type& get_no_check(const unversioned_id_type key) const {
			return get_no_check_impl(*this, key);
		}

		mapped_type* find_no_check(const unversioned_id_type key) {
			return find_no_check_impl(*this, key);
		}

		const mapped_type* find_no_check(const unversioned_id_type key) const {
			return find_no_check_impl(*this, key);
		}

		mapped_type& get(const key_type key) {
			return get_impl(*this, key);
		}

		const mapped_type& get(const key_type key) const {
			return get_impl(*this, key);
		}
		
		mapped_type* find(const key_type key) {
			return find_impl(*this, key);
		}

		const mapped_type* find(const key_type key) const {
			return find_impl(*this, key);
		}

		bool alive(const key_type key) const {
			return correct_range(key) && versions_match(get_indirector(key), key);
		}

		bool dead(const key_type key) const {
			return !alive(key);
		}

		mapped_type* data() {
			return objects.data();
		}

		const mapped_type* data() const {
			return objects.data();
		}

		const auto& get_objects() const {
			return objects;
		};

		auto size() const {
			return static_cast<size_type>(slots.size());
		}

		auto get_container_max_size() const {
			return objects.max_size();
		}

		auto max_size() const {
			const auto size_type_limit = std::size_t(std::numeric_limits<size_type>::max() - 1);
			const auto container_limit = std::size_t(get_container_max_size());

			return std::min(size_type_limit, container_limit);
		}

		auto capacity() const {
			return static_cast<size_type>(indirectors.size());
		}

		bool empty() const {
			return size() == 0;
		}

		bool size_at_capacity() const {
			return size() == capacity();
		}

		bool can_still_expand() const {
			return size() < max_size();
		}

		bool full() const {
			return size_at_capacity() && !can_still_expand();
		}

		bool indirectors_equal(const pool& b) const {
			static_assert(std::is_trivially_copyable_v<pool_indirector_type>);

			return 
				indirectors.size() == b.indirectors.size()
				&& !std::memcmp(
					indirectors.data(),
				   	b.indirectors.data(),
				   	indirectors.size() * sizeof(pool_indirector_type)
				)
			;
		}

		template <class F>
		void for_each_id_and_object(F f) {
			key_type id;

			for (size_type i = 0; i < size(); ++i) {
				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(id, objects[i]);
			}
		}

		template <class F>
		void for_each_id_and_object(F f) const {
			for (size_type i = 0; i < size(); ++i) {
				key_type id;

				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(id, objects[i]);
			}
		}

		auto begin() {
			return objects.begin();
		}

		auto begin() const {
			return objects.begin();
		}

		auto end() {
			return objects.end();
		}

		auto end() const {
			return objects.end();
		}

		auto get_nth_id(const size_type i) const {
			key_type id;

			const auto& s = slots[i];
			id.indirection_index = s.pointing_indirector;
			id.version = indirectors[s.pointing_indirector].version;

			return id;
		}

		auto get_id_of(const value_type& object) const {
			return get_nth_id(index_in(objects, object));
		}

		auto find_nth_id(const size_type i) const {
			key_type id;

			if (i < slots.size()) {
				const auto& s = slots[i];

				id.indirection_index = s.pointing_indirector;

				if (s.pointing_indirector < indirectors.size()) {
					id.version = indirectors[s.pointing_indirector].version;
				}
			}

			return id;
		}

		const auto& get_indirectors() const {
			return indirectors;
		}

		void clear() {
			objects.clear();
			slots.clear();
			indirectors.clear();
			free_indirectors.clear();

			if constexpr(has_synchronized_arrays) {
				synchronized_arrays.clear();
			}
		}

		template <class C>
		auto& get_corresponding_array() {
			return synchronized_arrays.template get_for<C>();
		}

		template <class C>
		const auto& get_corresponding_array() const {
			return synchronized_arrays.template get_for<C>();
		}

		template <class C>
		C& get_corresponding(mapped_type& object) {
			const auto idx = index_in(objects, object);
			return synchronized_arrays.template get_for<C>()[idx];
		}

		template <class C>
		const C& get_corresponding(const mapped_type& object) const {
			const auto idx = index_in(objects, object);
			return synchronized_arrays.template get_for<C>()[idx];
		}

		template <class Archive>
		void write_object_bytes(Archive& ar) const;

		template <class Archive>
		void read_object_bytes(Archive& ar);

		template <class Archive>
		void write_object_lua(Archive& ar) const;

		template <class Archive>
		void read_object_lua(const Archive& ar);

		/* Synonyms for compatibility with other containers */

		template <class... Args>
		decltype(auto) at(Args&&... args) {
			return get(std::forward<Args>(args)...);
		}

		template <class... Args>
		decltype(auto) at(Args&&... args) const {
			return get(std::forward<Args>(args)...);
		}

		template <class... Args>
		decltype(auto) operator[](Args&&... args) {
			return get(std::forward<Args>(args)...);
		}

		template <class... Args>
		decltype(auto) operator[](Args&&... args) const {
			return get(std::forward<Args>(args)...);
		}

		void might_allocate_objects(const std::size_t count) {
			/* 
				Prereserve objects before potential allocations so that existing pointers
				aren't invalidated.
			*/

			const auto required_capacity = objects.size() + count;

			if (required_capacity > objects.capacity()) {
				objects.reserve(required_capacity * 2);
			}
		}

		bool next_allocation_preserves_pointers() const {
			return objects.size() < objects.capacity();
		}
	};
}


namespace augs {
	template <class A, class M, template <class> class C, class S, class SA, class... K>
	void read_object_bytes(A& ar, pool<M, C, S, SA, K...>& storage) {
		storage.read_object_bytes(ar);
	}
	
	template <class A, class M, template <class> class C, class S, class SA, class... K>
	void write_object_bytes(A& ar, const pool<M, C, S, SA, K...>& storage) {
		storage.write_object_bytes(ar);
	}

	template <class A, class M, template <class> class C, class S, class SA, class... K>
	void read_object_lua(const A& ar, pool<M, C, S, SA, K...>& storage) {
		storage.read_object_lua(ar);
	}

	template <class A, class M, template <class> class C, class S, class SA, class... K>
	void write_object_lua(A& ar, const pool<M, C, S, SA, K...>& storage) {
		storage.write_object_lua(ar);
	}
}

/* A more generic approach just in case */

template <class P, class F>
void for_each_id_and_object(P& p, F&& callback) {
	p.for_each_id_and_object(std::forward<F>(callback));
}
