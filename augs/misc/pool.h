#pragma once
#include <vector>
#include "augs/ensure.h"
#include "pool_handle.h"
#include "pool_handlizer.h"

namespace augs {
	template<class T>
	class basic_pool : public pool_handlizer<basic_pool<T>> {
	public:
		typedef augs::pool_id<T> id_type;
		typedef augs::unversioned_id<T> unversioned_id_type;
		typedef augs::pool_handle<T> handle_type;
		typedef augs::const_pool_handle<T> const_handle_type;

		typedef std::vector<T> pooled_container_type;

		typedef T element_type;
	
	protected:
		struct metadata {
			int pointing_indirector = -1;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(CEREAL_NVP(pointing_indirector));
			}
		};

		struct indirector {
			int real_index = -1;
			unsigned version = 0;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(
					CEREAL_NVP(real_index),
					CEREAL_NVP(version)
				);
			}
		};

		pooled_container_type pooled;
		std::vector<metadata> slots;
		std::vector<indirector> indirectors;
		std::vector<int> free_indirectors;

	public:
		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(pooled),
				CEREAL_NVP(slots),
				CEREAL_NVP(indirectors),
				CEREAL_NVP(free_indirectors)
			);
		}

		template <class Archive>
		void write_object(Archive& ar) const {
			augs::write_with_capacity(ar, pooled);
			augs::write_with_capacity(ar, slots);
			augs::write_with_capacity(ar, indirectors);
			augs::write_with_capacity(ar, free_indirectors);
		}

		template <class Archive>
		void read_object(Archive& ar) {
			augs::read_with_capacity(ar, pooled);
			augs::read_with_capacity(ar, slots);
			augs::read_with_capacity(ar, indirectors);
			augs::read_with_capacity(ar, free_indirectors);
		}

	protected:
		void initialize_space(int slot_count) {
			pooled.clear();
			indirectors.clear();
			slots.clear();
			free_indirectors.clear();

			pooled.reserve(slot_count);
			slots.reserve(slot_count);

			indirectors.resize(slot_count);

			free_indirectors.resize(slot_count);
			for (int i = 0; i < slot_count; ++i)
				free_indirectors[i] = i;
		}

		template<class F>
		bool internal_free(id_type object, F custom_mover) {
			if (!alive(object))
				return false;

			int dead_index = indirectors[object.pool.indirection_index].real_index;

			// add dead object's indirector to the free indirection list
			free_indirectors.push_back(slots[dead_index].pointing_indirector);

			// therefore we must increase version of the dead indirector
			++indirectors[object.pool.indirection_index].version;

			if (dead_index != size() - 1) {
				int indirector_of_last_element = slots[size() - 1].pointing_indirector;

				// change last element's indirector - set it to the dead element's index
				indirectors[indirector_of_last_element].real_index = dead_index;

				slots[dead_index] = std::move(slots[size() - 1]);
				pooled[dead_index] = std::move(pooled[size() - 1]);
				custom_mover(dead_index, size() - 1);
			}

			slots.pop_back();
			pooled.pop_back();

			return true;
		}

		template<typename... Args>
		id_type allocate(Args... args) {
			if (free_indirectors.empty())
				throw std::runtime_error("Pool is full!");

			int next_free_indirection = free_indirectors.back();
			free_indirectors.pop_back();
			indirector& indirector = indirectors[next_free_indirection];

			int new_slot_index = size();

			metadata new_slot;
			new_slot.pointing_indirector = next_free_indirection;
			indirector.real_index = new_slot_index;

			id_type allocated_id;
			allocated_id.pool.version = indirector.version;
			allocated_id.pool.indirection_index = next_free_indirection;

			slots.push_back(new_slot);
			pooled.emplace_back(args...);

			return get_handle(allocated_id);
		}

		bool free(id_type object) {
			return internal_free(object, [](auto...){});
		}

	public:
		basic_pool(int slot_count = 0) {
			initialize_space(slot_count);
		}

		id_type make_versioned(unversioned_id_type unv) const {
			id_type ver;
			ver.pool.indirection_index = unv.pool.indirection_index;
			ver.pool.version = indirectors[unv.pool.indirection_index].version;
			return ver;
		}

		handle_type get_handle(id_type from_id) {
			return{ *this, from_id };
		}

		const_handle_type get_handle(id_type from_id) const {
			return{ *this, from_id };
		}

		template<class Pred>
		void for_each_with_id(Pred f) {
			id_type id;

			for (size_t i = 0; i < size(); ++i) {
				metadata& s = slots[i];
				id.pool.indirection_index = s.pointing_indirector;
				id.pool.version = indirectors[s.pointing_indirector].version;

				f(pooled[i], id);
			}
		}

		template<class Pred>
		void for_each_with_id(Pred f) const {
			id_type id;

			for (size_t i = 0; i < size(); ++i) {
				const metadata& s = slots[i];
				id.pool.indirection_index = s.pointing_indirector;
				id.pool.version = indirectors[s.pointing_indirector].version;

				f(pooled[i], id);
			}
		}
		template<class Pred>
		void for_each_id(Pred f) {
			id_type id;

			for (size_t i = 0; i < size(); ++i) {
				metadata& s = slots[i];
				id.pool.indirection_index = s.pointing_indirector;
				id.pool.version = indirectors[s.pointing_indirector].version;

				f(id);
			}
		}

		template<class Pred>
		void for_each_id(Pred f) const {
			id_type id;

			for (size_t i = 0; i < size(); ++i) {
				const metadata& s = slots[i];
				id.pool.indirection_index = s.pointing_indirector;
				id.pool.version = indirectors[s.pointing_indirector].version;

				f(id);
			}
		}

		template<class Pred>
		void for_each(Pred f) {
			std::for_each(pooled.begin(), pooled.end(), f);
		}

		template<class Pred>
		void for_each(Pred f) const {
			std::for_each(pooled.begin(), pooled.end(), f);
		}

		T& get(id_type object) {
			ensure(alive(object));
			return pooled[indirectors[object.pool.indirection_index].real_index];
		}

		const T& get(id_type object) const {
			ensure(alive(object));
			return pooled[indirectors[object.pool.indirection_index].real_index];
		}

		bool alive(id_type object) const {
			return object.pool.indirection_index >= 0 && indirectors[object.pool.indirection_index].version == object.pool.version;
		}

		basic_pool& get_pool(id_type) {
			return *this;
		}

		const basic_pool& get_pool(id_type) const {
			return *this;
		}

		T* data() {
			return pooled.data();
		}

		const T* data() const {
			return pooled.data();
		}

		size_t size() const {
			return slots.size();
		}

		size_t capacity() const {
			return indirectors.size();
		}

		bool empty() const {
			return size() == 0;
		}
	};

	template<class T>
	class pool : public basic_pool<T> {
	public:
		using basic_pool<T>::initialize_space;
		using basic_pool<T>::allocate;
		using basic_pool<T>::free;
		
		template <class Archive>
		void write_object(Archive& ar) const {
			augs::write_object(ar, static_cast<const basic_pool<T>&>(*this));
		}

		template <class Archive>
		void read_object(Archive& ar) {
			augs::read_object(ar, static_cast<basic_pool<T>&>(*this));
		}
	};

	template<class T, typename... meta>
	class pool_with_meta : public basic_pool<T> {
		std::vector<std::tuple<meta...>> metas;
	public:
		template <class Archive>
		void serialize(Archive& ar) {
			basic_pool<T>::serialize(ar);

			ar(CEREAL_NVP(metas));
		}

		template <class Archive>
		void write_object(Archive& ar) const {
			augs::write_object(ar, static_cast<const basic_pool<T>&>(*this));
			augs::write_with_capacity(ar, metas);
		}

		template <class Archive>
		void read_object(Archive& ar) {
			augs::read_object(ar, static_cast<basic_pool<T>&>(*this));
			augs::read_with_capacity(ar, metas);
		}

		void initialize_space(int slot_count) {
			basic_pool<T>::initialize_space(slot_count);

			metas.clear();
			metas.reserve(slot_count);
		}

		template<typename... Args>
		typename basic_pool<T>::id_type allocate(Args... args) {
			auto result = basic_pool<T>::allocate(args...);
			metas.emplace_back(std::tuple<meta...>());
			return result;
		}

		bool free(typename basic_pool<T>::id_type object) {
			auto result = basic_pool<T>::internal_free(object, [this](size_t to, size_t from){
				metas[to] = std::move(metas[from]);
			});

			if (result) {
				metas.pop_back();
				return true;
			}

			return false;
		}

		template <typename M>
		M& get_meta(typename basic_pool<T>::id_type object) {
			ensure(alive(object));
			return std::get<M>(metas[basic_pool<T>::indirectors[object.pool.indirection_index].real_index]);
		}

		template <typename M>
		const M& get_meta(typename basic_pool<T>::id_type object) const {
			ensure(alive(object));
			return std::get<M>(metas[basic_pool<T>::indirectors[object.pool.indirection_index].real_index]);
		}
	};

	template<class T>
	struct make_pool { typedef pool<T> type; };
}
