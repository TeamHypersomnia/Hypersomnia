#pragma once
#include <optional>

#include "augs/ensure.h"

#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/container_traits.h"
#include "augs/templates/container_templates.h"

#include "augs/misc/pool/pool_structs.h"
#include "augs/misc/pool/pooled_object_id.h"

#include "augs/misc/prevent_trivial_copy.h"

#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/readwrite/lua_readwrite_declaration.h"

namespace augs {
	template <class T, template <class> class make_container_type, class size_type, class... id_keys>
	class pool {
		prevent_trivial_copy guard;

	public:
		using value_type = T;

		using mapped_type = T;
		using key_type = pooled_object_id<size_type, id_keys...>;
		using unversioned_id_type = unversioned_id<size_type, id_keys...>;
		using undo_free_input_type = pool_undo_free_input<size_type, id_keys...>;

		using used_size_type = size_type;

	protected:
		using pool_slot_type = pool_slot<size_type>;
		using pool_indirector_type = pool_indirector<size_type>;

		using object_pool_type = make_container_type<mapped_type>;
		static constexpr bool constexpr_max_size = has_constexpr_max_size_v<object_pool_type>;

		make_container_type<pool_slot_type> slots;
		object_pool_type objects;
		make_container_type<pool_indirector_type> indirectors;
		make_container_type<size_type> free_indirectors;

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

		static bool versions_match(const pool_indirector_type& indirector, const key_type key) {
			return indirector.version == key.version && indirector.real_index != static_cast<size_type>(-1);
		}

		static auto ensure_versions_match(const pool_indirector_type& indirector, const key_type key) {
			ensure_eq(indirector.version, key.version);
			ensure(indirector.real_index != static_cast<size_type>(-1));
		}

	public:
		pool(const size_type slot_count = 0u) {
			if constexpr(constexpr_max_size) {
				(void)slot_count;
				reserve(objects.max_size());
			}
			else {
				reserve(slot_count);
			}
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

		key_type to_versioned(const unversioned_id_type key) const {
			key_type ver;
			ver.indirection_index = key.indirection_index;
			ver.version = indirectors[key.indirection_index].version;
			return ver;
		}

		key_type to_id(const size_type real_object_index) const {
			const auto& s = slots[real_object_index];

			key_type id;
			id.indirection_index = s.pointing_indirector;
			id.version = indirectors[s.pointing_indirector].version;

			return id;
		}

	private:
		template <class S>
		static auto& get_impl(S& self, const key_type key) {
			ensure(self.correct_range(key));

			const auto& indirector = self.get_indirector(key);

			ensure_versions_match(indirector, key);

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

		auto size() const {
			return static_cast<size_type>(slots.size());
		}

		auto max_size() const {
			return static_cast<size_type>(objects.max_size());
		}

		auto capacity() const {
			return static_cast<size_type>(indirectors.size());
		}

		bool empty() const {
			return size() == 0;
		}

		bool full_capacity() const {
			return size() == capacity();
		}

		bool full() const {
			return size() == max_size();
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

		void clear() {
			const auto c = capacity();

			objects.clear();
			slots.clear();
			indirectors.clear();
			free_indirectors.clear();

			reserve(c);
		}

		template <class Archive>
		void write_object_bytes(Archive& ar) const {
			auto w = [&ar](const auto& object) {
				augs::write_capacity_bytes(ar, object);
				augs::write_container_bytes(ar, object);
			};

			w(objects);
			w(slots);
			w(indirectors);
			w(free_indirectors);
		}

		template <class Archive>
		void read_object_bytes(Archive& ar) {
			auto r = [&ar](auto& object) {
				augs::read_capacity_bytes(ar, object);
				augs::read_container_bytes(ar, object);
			};

			r(objects);
			r(slots);
			r(indirectors);
			r(free_indirectors);
		}

		template <class Archive>
		void write_object_lua(Archive& ar) const {
			write_lua_table(ar, objects);
			write_lua_table(ar, slots);
			write_lua_table(ar, indirectors);
			write_lua_table(ar, free_indirectors);
		}

		template <class Archive>
		void read_object_lua(Archive& ar) {
			read_lua_table(ar, objects);
			read_lua_table(ar, slots);
			read_lua_table(ar, indirectors);
			read_lua_table(ar, free_indirectors);
		}

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
	};
}

#if READWRITE_OVERLOAD_TRAITS_INCLUDED || LUA_READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	template <class A, class M, template <class> class C, class S, class... K>
	void read_object_bytes(A& ar, pool<M, C, S, K...>& storage) {
		storage.read_object_bytes(ar);
	}
	
	template <class A, class M, template <class> class C, class S, class... K>
	void write_object_bytes(A& ar, const pool<M, C, S, K...>& storage) {
		storage.write_object_bytes(ar);
	}

	template <class A, class M, template <class> class C, class S, class... K>
	void read_object_lua(A ar, pool<M, C, S, K...>& storage) {
		storage.read_object_lua(ar);
	}

	template <class A, class M, template <class> class C, class S, class... K>
	void write_object_lua(A ar, const pool<M, C, S, K...>& storage) {
		storage.write_object_lua(ar);
	}
}

/* A more generic approach just in case */

template <class P, class F>
void for_each_id_and_object(P& p, F&& callback) {
	p.for_each_id_and_object(std::forward<F>(callback));
}
