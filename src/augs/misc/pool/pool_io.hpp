#pragma once
#include "augs/misc/pool/pool.h"

#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/readwrite/lua_readwrite_declaration.h"

#if READWRITE_OVERLOAD_TRAITS_INCLUDED || LUA_READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	template <class A, template <class> class B, class C, class D, class... E>
	template <class Archive>
	void pool<A, B, C, D, E...>::write_object_bytes(Archive& ar) const {
		auto w = [&ar](const auto& object) {
			augs::write_capacity_bytes(ar, object);
			augs::write_bytes(ar, object);
		};

		w(objects);
		w(slots);
		w(indirectors);
		w(free_indirectors);
	}

	template <class A, template <class> class B, class C, class D, class... E>
	template <class Archive>
	void pool<A, B, C, D, E...>::read_object_bytes(Archive& ar) {
		auto r = [&ar](auto& object) {
			augs::read_capacity_bytes(ar, object);
			augs::read_bytes(ar, object);
		};

		r(objects);
		r(slots);
		r(indirectors);
		r(free_indirectors);

		if constexpr(has_synchronized_arrays) {
			synchronized_arrays.for_each_container(
				[&](auto& container) {
					container.resize(objects.size());
				}
			);
		}
	}

	/* 
		Lua exports/imports don't need to be deterministic so we rebuild the free indirectors and slots manually.
	*/

	template <class A, template <class> class B, class C, class D, class... E>
	template <class Archive>
	void pool<A, B, C, D, E...>::write_object_lua(Archive& into) const {
		auto objects_table = into.create();
		auto indirectors_table = into.create();

		for (std::size_t i = 0; i < objects.size(); ++i) {
			const auto lua_table_index = static_cast<int>(i + 1);

			write_table_or_field(objects_table, objects[i], lua_table_index);

			const auto pointing = slots[i].pointing_indirector;
			const auto version = indirectors[pointing].version;

			auto meta_entry_table = indirectors_table.create();

			write_table_or_field(meta_entry_table, pointing, "pointing");
			write_table_or_field(meta_entry_table, version, "version");

			indirectors_table[lua_table_index] = meta_entry_table;
		}

		into["objects"] = objects_table;
		into["indirectors"] = indirectors_table;
	}

	template <class A, template <class> class B, class C, class D, class... E>
	template <class Archive>
	void pool<A, B, C, D, E...>::read_object_lua(const Archive& from) {
		objects.clear();
		slots.clear();
		indirectors.clear();
		free_indirectors.clear();

		auto objects_table = from["objects"];
		auto indirectors_table = from["indirectors"];

		int counter = 1;

		using P = pool<A, B, C, D, E...>;
		using size_type = typename P::used_size_type;

		while (true) {
			auto object_entry = objects_table[counter];
			auto meta_entry = indirectors_table[counter];

			if (object_entry.valid() && meta_entry.valid()) {
				{
					A object;
					read_lua(object_entry, object);
					objects.emplace_back(std::move(object));
				}
				
				size_type pointing = 0;
				size_type version = 0;

				general_from_lua_value(meta_entry["pointing"], pointing);
				general_from_lua_value(meta_entry["version"], version);

				indirectors.resize(std::max(indirectors.size(), static_cast<std::size_t>(pointing) + 1));

				indirectors[pointing].real_index = objects.size() - 1;
				indirectors[pointing].version = version;

				using slot_type = typename P::pool_slot_type;

				slot_type slot;
				slot.pointing_indirector = pointing;
				slots.emplace_back(std::move(slot));
			}
			else {
				break;
			}

			++counter;
		}

		for (std::size_t i = 0; i < indirectors.size(); ++i) {
			if (indirectors[i].real_index == static_cast<size_type>(-1)) {
				free_indirectors.push_back(static_cast<size_type>(i));
			}
		}

		if constexpr(has_synchronized_arrays) {
			synchronized_arrays.for_each_container(
				[&](auto& container) {
					container.resize(objects.size());
				}
			);
		}
	}
}
