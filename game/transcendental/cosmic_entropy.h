#pragma once
#include "augs/misc/streams.h"
#include "augs/window_framework/event.h"
#include <map>
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/messages/intent_message.h"

#include "game/detail/item_slot_transfer_request.h"
#include "augs/templates/container_templates.h"

#include "augs/misc/machine_entropy.h"

class cosmos;

template <class key>
struct basic_cosmic_entropy {
	std::map<key, std::vector<key_and_mouse_intent>> entropy_per_entity;
	std::vector<item_slot_transfer_request_data> transfer_requests;
	
	void override_transfers_leaving_other_entities(
		const cosmos&,
		std::vector<item_slot_transfer_request_data> new_transfers
	);

	size_t length() const;

	basic_cosmic_entropy& operator+=(const basic_cosmic_entropy& b);
};

struct cosmic_entropy;

struct guid_mapped_entropy : basic_cosmic_entropy<unsigned> {
	guid_mapped_entropy() = default;
	explicit guid_mapped_entropy(const cosmic_entropy&, const cosmos&);
	
	guid_mapped_entropy& operator+=(const guid_mapped_entropy& b) {
		basic_cosmic_entropy<unsigned>::operator+=(b);
		return *this;
	}

	bool operator!=(const guid_mapped_entropy&) const;
};

struct cosmic_entropy : basic_cosmic_entropy<entity_id> {
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
		basic_cosmic_entropy<entity_id>::operator+=(b);
		return *this;
	}
};

namespace augs {
	template<class A>
	auto read_object(A& ar, guid_mapped_entropy& storage) {
		unsigned char num_entropied_entities;

		if (!augs::read_object(ar, num_entropied_entities)) {
			return false;
		}
		
		while (num_entropied_entities--) {
			unsigned guid;
			
			if (!augs::read_object(ar, guid)) {
				return false;
			}

			auto& new_entity_entropy = storage.entropy_per_entity[guid];

			ensure(new_entity_entropy.empty());

			if (!augs::read_vector_of_objects(ar, new_entity_entropy, unsigned short())) {
				return false;
			}
		}

		if (!augs::read_vector_of_objects(ar, storage.transfer_requests, unsigned short())) {
			return false;
		}

		return true;
	}

	template<class A>
	void write_object(A& ar, const guid_mapped_entropy& storage) {
		ensure(storage.entropy_per_entity.size() < std::numeric_limits<unsigned char>::max());

		const auto num_entropied_entities = static_cast<unsigned char>(storage.entropy_per_entity.size());
		augs::write_object(ar, num_entropied_entities);

		for (const auto& per_entity : storage.entropy_per_entity) {
			augs::write_object(ar, per_entity.first);
			augs::write_vector_of_objects(ar, per_entity.second, unsigned short());
		}

		augs::write_vector_of_objects(ar, storage.transfer_requests, unsigned short());
	}
}