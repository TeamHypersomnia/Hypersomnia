#pragma once
#include <unordered_map>

#include "augs/misc/step_player.h"
#include "cosmic_entropy.h"

class cosmic_movie_director {
	void load_recording_from_file(const std::string);

public:
	struct single_step_for_entity {
		std::vector<entity_intent> intents;
	};

	std::unordered_map<unsigned, augs::step_player<single_step_for_entity>> guid_to_recording;

	void load_recordings_from_directory(const std::string);
};

namespace augs {
	template<class A>
	auto read_object(A& ar, cosmic_movie_director::single_step_for_entity& storage) {
		if (!augs::read_vector_of_objects(ar, storage.intents, unsigned short())) {
			return false;
		}

		return true;
	}

	template<class A>
	void write_object(A& ar, const cosmic_movie_director::single_step_for_entity& storage) {
		augs::write_vector_of_objects(ar, storage.intents, unsigned short());
	}
}