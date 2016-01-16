#pragma once
#include "memory_pool.h"
#include "unsafe_type_collection.h"
#include <cassert>

namespace augs {
	simple_pool<memory_pool*> memory_pool::pool_locations;

	void memory_pool::pool_id::unset() {
		pointer_id = -1;
	}

	memory_pool* memory_pool::pool_id::operator->() const {
		return pool_locations.get(pointer_id);
	}

	memory_pool& memory_pool::pool_id::operator*() const {
		return *pool_locations.get(pointer_id);
	}

	memory_pool::pool_id::operator bool() const {
		return pointer_id != -1;
	}

	memory_pool::memory_pool(int slot_count, int slot_size) { 
		initialize(slot_count, slot_size);  
		this_pool_pointer_location.pointer_id = pool_locations.allocate(this);
	}

	memory_pool::~memory_pool() {
		free_all();
		pool_locations.destroy(this_pool_pointer_location.pointer_id);
	}
	
	memory_pool::id::id() {}

	memory_pool::byte* memory_pool::id::ptr() { return owner->get(*this); }
	const memory_pool::byte* memory_pool::id::ptr() const { return owner->get(*this); }

	memory_pool& memory_pool::id::get_pool() { return *owner; }

	bool memory_pool::id::operator<(const id& b) const { return ptr() < b.ptr(); }
	bool memory_pool::id::operator!() const { return !alive(); }
	bool memory_pool::id::operator==(const id& b) const { return !memcmp(this, &b, sizeof(id)); }
	bool memory_pool::id::operator!=(const id& b) const { return !operator==(b); }

	bool memory_pool::id::alive() const { return owner && owner->alive(*this); }
	bool memory_pool::id::dead() const { return !alive(); }

	void memory_pool::id::unset() { owner.unset(); }

	void memory_pool::initialize(int slot_count, int slot_size) {
		this->slot_size = slot_size;
		resize(slot_count);
	}
	
	void memory_pool::resize(int slot_count) {
		// TODO: actually do the resizing instead of reinitialization

		pool.clear();
		indirectors.clear();
		slots.clear();
		free_indirectors.clear();

		pool.resize(slot_count * slot_size);
		slots.resize(slot_count);

		indirectors.resize(slot_count);

		free_indirectors.resize(slot_count);
		for (int i = 0; i < slot_count; ++i)
			free_indirectors[i] = i;
	}

	bool memory_pool::alive(id object) const {
		return indirectors[object.indirection_index].version == object.version;
	}

	memory_pool::id memory_pool::get_id(byte* address) {
		id found_id;

		if (address < pool.data() || address >= pool.data() + size() * slot_size)
			found_id.owner.unset();
		else {
			int slot_index = (address - &*pool.begin()) / slot_size;
			int indirector_index = slots[slot_index].pointing_indirector;

			found_id.owner = this_pool_pointer_location;
			found_id.indirection_index = indirector_index;
			found_id.version = indirectors[indirector_index].version;
		}

		return found_id;
	}

	memory_pool::byte* memory_pool::get(id object) {
		if (!alive(object))
			return nullptr;

		return pool.data() + slot_size * indirectors[object.indirection_index];
	}

	const memory_pool::byte* memory_pool::get(id object) const {
		if (!alive(object))
			return nullptr;

		return pool.data() + slot_size * indirectors[object.indirection_index].real_index;
	}

	memory_pool::id memory_pool::allocate() {
		if (free_indirectors.empty())
			throw std::runtime_error("Pool is full!");

		int next_free_indirection = free_indirectors.back();
		free_indirectors.pop_back();
		indirector& indirector = indirectors[next_free_indirection];

		int new_slot_index = size();
		slots[new_slot_index].pointing_indirector = next_free_indirection;
		indirector.real_index = new_slot_index;

		id allocated_id;
		allocated_id.owner = this_pool_pointer_location;
		allocated_id.version = indirector.version;
		allocated_id.indirection_index = next_free_indirection;

		++count;
		return allocated_id;
	}

	std::pair<int, int> memory_pool::internal_free(id object) {
		auto result = std::make_pair(-1, -1);
		
		if (!alive(object))
			return result;

		int dead_index = indirectors[object.indirection_index].real_index;

		// add dead object's indirector to the free indirection list
		free_indirectors.push_back(slots[dead_index].pointing_indirector);

		// therefore we must increase version of the dead indirector
		++indirectors[object.indirection_index].version;

		result = std::make_pair(dead_index, dead_index);

		if (dead_index != size() - 1) {
			int indirector_of_last_element = slots[size() - 1].pointing_indirector;

			// mark last as dead
			slots[size() - 1].pointing_indirector = -1;

			// change last element's indirector - set it to the dead element's index
			indirectors[indirector_of_last_element].real_index = dead_index;

			// because of this, indirector number at dead_index metadata needs to be updated as well
			slots[dead_index].pointing_indirector = indirector_of_last_element;

			result.second = size() - 1;
		}

		--count;
		return result;
	}

	bool memory_pool::free(id object) {
		auto swap_elements = internal_free(object);

		if (swap_elements.first == -1 && swap_elements.second == -1)
			return false;

		if (swap_elements.first != swap_elements.second) {
			int dead_offset = swap_elements.first * slot_size;
			int last_offset = swap_elements.second * slot_size;

			// move bytes from the last element to the dead element's position
			memcpy(pool.data() + dead_offset, pool.data() + last_offset, slot_size);
		}

		return true;
	}

	memory_pool::id memory_pool::allocate_with_default_construct(size_t type_hash) {
		// TODO: typed memory allocation but only if needed
		assert(0);
		return memory_pool::id();
	}
	
	bool memory_pool::free_with_destructor(id object, size_t type_hash) {
		unsafe_type_collection::destructors[type_hash](object.ptr());

		auto swap_elements = memory_pool::internal_free(object);

		if (swap_elements.first == -1 && swap_elements.second == -1)
			return false;

		if (swap_elements.first != swap_elements.second) {
			int dead_offset = swap_elements.first * slot_size;
			int last_offset = swap_elements.second * slot_size;

			// move construct at the dead element's position from the last element's position
			unsafe_type_collection::move_constructors[type_hash](pool.data() + dead_offset, pool.data() + last_offset);
		}

		return true;
	}

	void memory_pool::destruct_all(size_t type_hash) {
		auto destructor = unsafe_type_collection::destructors[type_hash];

		for (int i = 0; i < size(); ++i)
			destructor((*this)[i]);
	}

	void memory_pool::free_all() {
		while (size() > 0)
			free(get_id(pool.data() + (size() - 1) * slot_size));
	}

	memory_pool::byte* memory_pool::data() {
		return pool.data();
	}

	memory_pool::byte* memory_pool::operator[](int index) {
		return data() + index * slot_size;
	}

	int memory_pool::size() const {
		return count;
	}

	int memory_pool::capacity() const { return slots.size(); }
}