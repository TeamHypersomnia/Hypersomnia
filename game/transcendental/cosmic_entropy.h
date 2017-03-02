#pragma once
#include "augs/misc/streams.h"
#include "augs/window_framework/event.h"
#include <map>
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/messages/intent_message.h"

#include "game/detail/inventory/item_slot_transfer_request.h"
#include "augs/templates/container_templates.h"

#include "augs/misc/machine_entropy.h"

#include "game/enums/spell_type.h"

class cosmos;

template <class key>
struct basic_cosmic_entropy {
	typedef key key_type;

	std::map<key, spell_type> cast_spells;
	std::map<key, std::vector<key_and_mouse_intent>> intents_per_entity;
	std::vector<basic_item_slot_transfer_request_data<key>> transfer_requests;
	
	void override_transfers_leaving_other_entities(
		const cosmos&,
		std::vector<basic_item_slot_transfer_request_data<key>> new_transfers
	);

	size_t length() const;

	basic_cosmic_entropy& operator+=(const basic_cosmic_entropy& b);
};

struct cosmic_entropy;

struct guid_mapped_entropy : basic_cosmic_entropy<entity_guid> {
	typedef basic_cosmic_entropy<entity_guid> base;

	guid_mapped_entropy() = default;
	explicit guid_mapped_entropy(const cosmic_entropy&, const cosmos&);
	
	guid_mapped_entropy& operator+=(const guid_mapped_entropy& b) {
		base::operator+=(b);
		return *this;
	}

	bool operator!=(const guid_mapped_entropy&) const;
};

struct cosmic_entropy : basic_cosmic_entropy<entity_id> {
	typedef basic_cosmic_entropy<entity_id> base;

	cosmic_entropy() = default;
	
	explicit cosmic_entropy(
		const guid_mapped_entropy&, 
		const cosmos&
	);
	
	explicit cosmic_entropy(
		const const_entity_handle controlled_entity, 
		const std::vector<key_and_mouse_intent>&
	);

	cosmic_entropy& operator+=(const cosmic_entropy& b) {
		base::operator+=(b);
		return *this;
	}
};

namespace augs {
	template<class A, class K>
	auto read_object(A& ar, basic_cosmic_entropy<K>& storage) {
		{
			unsigned char num_entropied_entities = 0;

			if (!augs::read_object(ar, num_entropied_entities)) {
				return false;
			}
			
			while (num_entropied_entities--) {
				K guid;
				
				if (!augs::read_object(ar, guid)) {
					return false;
				}

				auto& new_entity_entropy = storage.intents_per_entity[guid];

				ensure(new_entity_entropy.empty());

				if (!augs::read_object(ar, new_entity_entropy, unsigned short())) {
					return false;
				}
			}
		}

		if (!augs::read_object(ar, storage.cast_spells, unsigned char())) {
			return false;
		}

		if (!augs::read_object(ar, storage.transfer_requests, unsigned short())) {
			return false;
		}

		return true;
	}

	template<class A, class K>
	void write_object(A& ar, const basic_cosmic_entropy<K>& storage) {
		ensure(storage.intents_per_entity.size() < std::numeric_limits<unsigned char>::max());

		{
			const auto num_entropied_entities = static_cast<unsigned char>(storage.intents_per_entity.size());
			augs::write_object(ar, num_entropied_entities);

			for (const auto& per_entity : storage.intents_per_entity) {
				augs::write_object(ar, per_entity.first);
				augs::write_object(ar, per_entity.second, unsigned short());
			}
		}

		augs::write_object(ar, storage.cast_spells, unsigned char());
		augs::write_object(ar, storage.transfer_requests, unsigned short());
	}

	template<class A>
	void write_object(A& ar, const guid_mapped_entropy& storage) {
		return write_object(ar, static_cast<const typename std::decay_t<decltype(storage)>::base&>(storage));
	}

	template<class A>
	bool read_object(A& ar, guid_mapped_entropy& storage) {
		return read_object(ar, static_cast<typename std::decay_t<decltype(storage)>::base&>(storage));
	}

	template<class A>
	void write_object(A& ar, const cosmic_entropy& storage) {
		return write_object(ar, static_cast<const typename std::decay_t<decltype(storage)>::base&>(storage));
	}

	template<class A>
	bool read_object(A& ar, cosmic_entropy& storage) {
		return read_object(ar, static_cast<typename std::decay_t<decltype(storage)>::base&>(storage));
	}
}