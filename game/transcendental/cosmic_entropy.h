#pragma once
#include "augs/misc/streams.h"
#include "augs/window_framework/event.h"
#include <map>
#include "game/transcendental/entity_id.h"
#include "game/messages/intent_message.h"

namespace augs {
	struct machine_entropy;
}

class cosmos;
struct input_context;

bool make_entity_intent(const input_context&, const augs::window::event::state&, entity_intent& mapped_intent);

template <class key>
struct basic_cosmic_entropy {
	std::map<key, std::vector<entity_intent>> entropy_per_entity;
	augs::stream delta_to_apply;
	
	size_t length() const {
		size_t total = 0;

		for (const auto& ent : entropy_per_entity)
			total += ent.second.size();

		return total;
	}

	basic_cosmic_entropy& operator+=(const basic_cosmic_entropy& b) {
		for (const auto& ent : b.entropy_per_entity) {
			auto& vec = entropy_per_entity[ent.first];
			vec.insert(vec.end(), ent.second.begin(), ent.second.end());
		}

		return *this;
	}
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
	explicit cosmic_entropy(const guid_mapped_entropy&, const cosmos&);

	cosmic_entropy& operator+=(const cosmic_entropy& b) {
		basic_cosmic_entropy<entity_id>::operator+=(b);
		return *this;
	}
};

namespace augs {
	template<class A>
	auto read_object(A& ar, guid_mapped_entropy& storage) {
		unsigned char num_entropied_entities;

		if(!augs::read_object(ar, num_entropied_entities))
			return false;
		
		while (num_entropied_entities--) {
			unsigned guid;
			
			if (!augs::read_object(ar, guid))
				return false;

			auto& new_entity_entropy = storage.entropy_per_entity[guid];

			ensure(new_entity_entropy.empty());

			if (!augs::read_vector_of_objects(ar, new_entity_entropy, unsigned short()))
				return false;
		}

		return true;
	}

	template<class A>
	void write_object(A& ar, const guid_mapped_entropy& storage) {
		ensure(storage.entropy_per_entity.size() < std::numeric_limits<unsigned char>::max());

		unsigned char num_entropied_entities = static_cast<unsigned char>(storage.entropy_per_entity.size());
		augs::write_object(ar, num_entropied_entities);

		for (const auto& per_entity : storage.entropy_per_entity) {
			augs::write_object(ar, per_entity.first);
			augs::write_vector_of_objects(ar, per_entity.second, unsigned short());
		}
	}
}