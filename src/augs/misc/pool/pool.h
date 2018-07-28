#pragma once
#include <optional>

#include "augs/ensure.h"
#include "augs/misc/pool/pool_structs.h"
#include "augs/misc/pool/pooled_object_id.h"

#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/container_traits.h"
#include "augs/templates/container_templates.h"

#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/readwrite/lua_readwrite_declaration.h"

namespace augs {
	template <class T, template <class> class make_container_type, class size_type, class... id_keys>
	class pool {
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
		allocation_result allocate(Args&&... args) {
			if (full_capacity()) {
				if constexpr(constexpr_max_size) {
					throw std::runtime_error("This static pool cannot be further expanded.");
				}
				else {
					const auto old_size = size();
					const auto new_size = std::size_t(old_size) * expansion_mult + expansion_add;

					ensure(new_size <= std::numeric_limits<size_type>::max());

					reserve(static_cast<size_type>(new_size));
				}
			}

			const auto next_free_indirector = free_indirectors.back();
			free_indirectors.pop_back();
			
			const auto new_slot_index = size();

			pool_indirector_type& allocated_indirector = indirectors[next_free_indirector];
			allocated_indirector.real_index = new_slot_index;

			key_type allocated_id;
			allocated_id.version = allocated_indirector.version;
			allocated_id.indirection_index = next_free_indirector;

			pool_slot_type allocated_slot;
			allocated_slot.pointing_indirector = next_free_indirector;

			slots.push_back(allocated_slot);
			objects.emplace_back(std::forward<Args>(args)...);

			return { allocated_id, objects.back() };
		}

		auto free(const unversioned_id_type key) {
			return free(to_versioned(key));
		}

		void undo_last_allocate(const key_type key) {
			if (!correct_range(key)) {
				return;
			}

			auto& indirector = get_indirector(key);

			if (!versions_match(indirector, key)) {
				return;
			}

			if (const auto removed_at_index = indirector.real_index;
				removed_at_index != size() - 1
			) {
				throw std::runtime_error("Undoing allocations in bad order.");
			}
			else {
				/* add dead key's indirector to the list of free indirectors */
				free_indirectors.push_back(key.indirection_index);

				/* we do not increase version of the dead indirector, as we are undoing. */

				/* ...and mark it as unused. */
				indirector.real_index = static_cast<size_type>(-1);

				slots.pop_back();
				objects.pop_back();
			}
		}

		std::optional<undo_free_input_type> free(const key_type key) {
			if (!correct_range(key)) {
				return std::nullopt;
			}

			auto& indirector = get_indirector(key);

			if (!versions_match(indirector, key)) {
				return std::nullopt;
			}

			/*
				before we mess with the to-be-deleted indirector,
				save the stored real index 
			*/

			const auto removed_at_index = indirector.real_index;

			/* Prepare also the return value */
			undo_free_input_type result;
			result.indirection_index = key.indirection_index;
			result.real_index = removed_at_index;

			/* add dead key's indirector to the list of free indirectors */
			free_indirectors.push_back(key.indirection_index);

			/* therefore we must increase version of the dead indirector... */
			++indirector.version;

			/* ...and mark it as unused. */
			indirector.real_index = static_cast<size_type>(-1);

			if (removed_at_index != size() - 1) {
				{
					const auto indirector_of_last_element = slots.back().pointing_indirector;

					/* change last element's indirector - set it to the removed element's index */
					indirectors[indirector_of_last_element].real_index = removed_at_index;
				}

				slots[removed_at_index] = std::move(slots.back());
				objects[removed_at_index] = std::move(objects.back());
			}

			slots.pop_back();
			objects.pop_back();

			return result;
		}

		template <class... Args>
		allocation_result undo_free(
			const undo_free_input_type in,
			Args&&... removed_content
		) {
			static_assert(sizeof...(Args) > 0, "Specify what to construct.");

			const auto indirection_index = in.indirection_index;
			const auto real_index = in.real_index;

			if (free_indirectors.back() == indirection_index) {
				/* This will be usually the case */
				free_indirectors.pop_back();
			}
			else {
				/* This might happen if a reserve happened after free */
				erase_element(free_indirectors, indirection_index);
			}

			auto& indirector = indirectors[indirection_index];

			indirector.real_index = real_index;
			--indirector.version;

			auto get_new_key = [&](){
				key_type new_key;
				new_key.version = indirector.version;
				new_key.indirection_index = indirection_index;
				return new_key;
			};

			if (real_index < size()) {
				auto& new_slot_space = slots[real_index];
				auto& new_object_space = objects[real_index];

				{
					const auto indirector_of_moved_element = new_slot_space.pointing_indirector;

					/* change moved element's indirector - set it back to the last index */
					indirectors[indirector_of_moved_element].real_index = size();
				}

				slots.emplace_back(std::move(new_slot_space));
				objects.emplace_back(std::move(new_object_space));

				new_slot_space.pointing_indirector = indirection_index;
				new_object_space.~mapped_type();
				new (std::addressof(new_object_space)) mapped_type(std::forward<Args>(removed_content)...);

				return { get_new_key(), new_object_space };
			}
			else {
				pool_slot_type slot; 
				slot.pointing_indirector = indirection_index;

				objects.emplace_back(std::forward<Args>(removed_content)...);
				slots.emplace_back(std::move(slot));

				return { get_new_key(), objects.back() };
			}
		}

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
