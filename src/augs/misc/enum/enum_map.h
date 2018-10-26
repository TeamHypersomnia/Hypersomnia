#pragma once
#include "augs/ensure.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/is_comparable.h"
#include "augs/templates/container_templates.h"
#include "augs/misc/enum/enum_boolset.h"
#include "augs/misc/simple_pair.h"

#include "augs/misc/declare_containers.h"

namespace augs {
	template <class Enum, class T>
	class enum_map_base {
	public:
		using key_type = Enum;
		using mapped_type = T;
		using value_type = std::pair<const Enum, T>;

	protected:
		using size_type = std::size_t;

		static constexpr size_type max_n = static_cast<size_type>(key_type::COUNT);
		static constexpr bool is_trivially_copyable = std::is_trivially_copyable_v<mapped_type>;
		static constexpr bool is_default_constructible = std::is_default_constructible_v<mapped_type>;
		
#if TODO
		static_assert(
			!is_trivially_copyable || std::is_default_constructible_v<mapped_type>,
			"No support for a type that is trivially copyable but not default constructible."
		);
#endif

		using storage_type = std::aligned_storage_t<
			sizeof(mapped_type) * max_n, alignof(mapped_type)
		>;

		//using storage_type = std::array<
		//	std::aligned_storage_t<sizeof(mapped_type), alignof(mapped_type)>,
		//	max_n
		//>;

		using flagset_type = augs::enum_boolset<Enum>;

		flagset_type is_value_set;
		storage_type data = storage_type();
		
		bool is_set(const size_type index) const {
			return is_value_set.test(static_cast<Enum>(index));
		}

		template <class... Args>
		void set(const size_type index, Args&&... args) {
			is_value_set.set(static_cast<Enum>(index));
			new (&nth(index)) mapped_type(std::forward<Args>(args)...);
		}

		size_type find_first_set_index(size_type from) const {
			while (from < max_n && !is_set(from)) {
				++from;
			}

			return from;
		}

		size_type reverse_find_first_set_index(size_type from) const {
			while (from != static_cast<size_type>(-1) && !is_set(from)) {
				--from;
			}

			return from;
		}

		auto& nth(const size_type n) {
			return reinterpret_cast<mapped_type*>(&data)[n];
		}

		const auto& nth(const size_type n) const {
			return reinterpret_cast<const mapped_type*>(&data)[n];
		}

	public:
		template <bool is_const, bool R>
		class basic_iterator : public std::iterator<
			std::forward_iterator_tag,   // iterator_category
			maybe_const_t<is_const, T>                      // value_type
		> {
			using owner_ptr_type = maybe_const_ptr_t<is_const, enum_map_base>;
			using pair_type = simple_pair<const key_type, maybe_const_ref_t<is_const, mapped_type>>;
			
			owner_ptr_type ptr = nullptr;
			size_type idx = 0;

			friend class enum_map_base<Enum, T>;

		public:
			basic_iterator(const owner_ptr_type ptr, const size_type idx) : ptr(ptr), idx(idx) {}

			basic_iterator& operator++() {
				if constexpr(R) {
					idx = ptr->reverse_find_first_set_index(idx - 2) + 1;
				}
				else {
					idx = ptr->find_first_set_index(idx + 1);
				}

				return *this;
			}

			const basic_iterator operator++(int) {
				const iterator temp = *this;
				++*this;
				return temp;
			}

			bool operator==(const basic_iterator& b) const {
				return idx == b.idx;
			}

			bool operator!=(const basic_iterator& b) const {
				return idx != b.idx;
			}

			pair_type operator*() const {
				if constexpr(R) {
					ensure(idx != 0);
					return { static_cast<key_type>(idx - 1), ptr->nth(idx - 1) };
				}
				else {
					ensure(idx < ptr->capacity());
					return { static_cast<key_type>(idx), ptr->nth(idx) };
				}
			}
		};

		using iterator = basic_iterator<false, false>;
		using const_iterator = basic_iterator<true, false>;
		using reverse_iterator = basic_iterator<false, true>;
		using const_reverse_iterator = basic_iterator<true, true>;

		template <bool, bool>
		friend class basic_iterator;

		iterator begin() {
			return { this, find_first_set_index(0u) };
		}

		iterator end() {
			return { this, max_n };
		}

		const_iterator begin() const {
			return { this, find_first_set_index(0u) };
		}

		const_iterator end() const {
			return { this, max_n };
		}

		reverse_iterator rbegin() {
			const auto idx = 1 + reverse_find_first_set_index(max_size()); 
			return { this, idx };
		}

		reverse_iterator rend() {
			return { this, 0 };
		}

		const_reverse_iterator rbegin() const {
			return { this, find_first_set_index(0u) };
		}

		const_reverse_iterator rend() const {
			return { this, 0 };
		}

		iterator find(const key_type enum_idx) {
			const size_type i = static_cast<size_type>(enum_idx);
			ensure(i < capacity());

			if (is_set(i)) {
				return { this, i };
			}

			return end();
		}

		void erase(const key_type k) {
			const auto i = static_cast<size_type>(k);
			
			ensure(i < capacity());
			
			if (is_set(i)) {
				is_value_set.set(k, false);
				nth(i).~mapped_type();
			}
		}

		template <class... Args>
		simple_pair<iterator, bool> try_emplace(const key_type k, Args&&... args) {
			if (is_set(static_cast<size_type>(k))) {
				return { find(k), false };
			}
			else {
				set(static_cast<size_type>(k), std::forward<Args>(args)...);
				return { find(k), true };
			}
		}

		template <class... Args>
		auto emplace(const key_type k, Args&&... args) {
			return try_emplace(k, std::forward<Args>(args)...);
		}

		const_iterator find(const key_type enum_idx) const {
			const size_type i = static_cast<size_type>(enum_idx);
			ensure(i < capacity());

			if (is_set(i)) {
				return { this, i };
			}

			return end();
		}

		mapped_type& at(const key_type enum_idx) {
			const auto i = static_cast<size_type>(enum_idx);
			ensure(i < capacity());
			ensure(is_set(i));
			return nth(i);
		}

		const mapped_type& at(const key_type enum_idx) const {
			const auto i = static_cast<size_type>(enum_idx);
			ensure(i < capacity());
			ensure(is_set(i));
			return nth(i);
		}

		template <bool C = std::is_default_constructible_v<mapped_type>, class = std::enable_if_t<C>>
		mapped_type& operator[](const key_type enum_idx) {
			const auto i = static_cast<size_type>(enum_idx);
			ensure(i < capacity());

			if (!is_set(i)) {
				set(i);
			}

			return nth(i);
		}

		constexpr size_type capacity() const {
			return max_n;
		}

		constexpr size_type max_size() const {
			return max_n;
		}

		auto size() const {
			return is_value_set.count();
		}

		auto empty() const {
			return is_value_set.none();
		}

		void clear() {
			if constexpr(!is_trivially_copyable) {
				for (auto&& v : *this) {
					v.second.~mapped_type();
				}
			}

			is_value_set = flagset_type();
		}
	};

	template <class Enum, class T>
	class enum_map<Enum, T, std::enable_if_t<std::is_trivially_copyable_v<T>>>
		: public enum_map_base<Enum, T> {
	public:
		using base = enum_map_base<Enum, T>;
		using typename base::key_type;
		using typename base::mapped_type;
	};

	template <class Enum, class T>
	class enum_map<Enum, T, std::enable_if_t<!std::is_trivially_copyable_v<T>>>
		: public enum_map_base<Enum, T> {
	public:
		using base = enum_map_base<Enum, T>;
		using typename base::key_type;
		using typename base::mapped_type;
		using typename base::flagset_type;
		using base::clear;
		using base::emplace;
		using base::is_value_set;
		using base::begin;
		using base::end;

		enum_map() = default;
		
		enum_map(const enum_map& b) {
			for (auto&& v : b) {
				emplace(v.first, v.second);
			}
		}

		enum_map& operator=(const enum_map& b) {
			clear();

			for (const auto&& v : b) {
				emplace(v.first, v.second);
			}

			return *this;
		}

		enum_map(enum_map&& b) {
			for (auto&& v : b) {
				emplace(v.first, std::move(v.second));
			}

			b.is_value_set = flagset_type();
		}

		enum_map& operator=(enum_map&& b) {
			clear();

			for (auto&& v : b) {
				emplace(v.first, std::move(v.second));
			}

			b.is_value_set = flagset_type();

			return *this;
		}

		~enum_map() {
			clear();
		}
	};

	template <class Enum, class T, class = std::enable_if_t<is_comparable_v<T, T>>>
	bool operator==(
		const enum_map<Enum, T>& left, 
		const enum_map<Enum, T>& right
	) {
		for (const auto& it : left) {
			const auto ptr = mapped_or_nullptr(right, it.first);

			if (ptr == nullptr) {
				return false;
			}

			if (!(it.second == *ptr)) {
				return false;
			}
		}

		return true;
	}
}