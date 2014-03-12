#pragma once
#include "stdafx.h"
#include "bindings.h"

#include <SFML/Audio.hpp>

bool has_sound_stopped_playing(sf::Sound* s) {
	return s->getStatus() == sf::Sound::Status::Stopped;
}

namespace bindings {
	luabind::scope _sfml_audio() {
		return
			(
			luabind::def("has_sound_stopped_playing", has_sound_stopped_playing),

			luabind::class_<sf::Music>("sfMusic")
			.def(luabind::constructor<>())
			.def("openFromFile", &sf::Music::openFromFile)
			.def("play", &sf::Music::play)
			.def("pause", &sf::Music::pause)
			.def("stop", &sf::Music::stop)
			.def("setVolume", &sf::Music::setVolume)
			.def("setPitch", &sf::Music::setPitch)
			.def("setPlayingOffset", &sf::Music::setPlayingOffset)
			.def("setLoop", &sf::Music::setLoop)
			.def("setRelativeToListener", &sf::Music::setRelativeToListener)
			.def("setPlayingOffset", &sf::Music::setPlayingOffset)
			,

			luabind::class_<sf::SoundBuffer>("sfSoundBuffer")
			.def(luabind::constructor<>())
			.def("loadFromFile", &sf::SoundBuffer::loadFromFile)
			,

			luabind::class_<sf::Sound>("sfSound")
			.def(luabind::constructor<>())
			.def("setAttenuation", &sf::Sound::setAttenuation)
			.def("setBuffer", &sf::Sound::setBuffer)
			.def("setMinDistance", &sf::Sound::setMinDistance)
			.def("setRelativeToListener", &sf::Sound::setRelativeToListener)
			.def("setPlayingOffset", &sf::Sound::setPlayingOffset)
			.def("pause", &sf::Sound::pause)
			.def("setLoop", &sf::Sound::setLoop)
			.def("setVolume", &sf::Sound::setVolume)
			.def("setPitch", &sf::Sound::setPitch)
			.def("play", &sf::Sound::play)
			.def("setPosition", (void (sf::Sound::*)(float, float, float))(&sf::Sound::setPosition))
			);
	}
}