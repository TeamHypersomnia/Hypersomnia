#pragma once
#include "stdafx.h"
#include "bindings.h"

#include <SFML/Audio.hpp>

namespace bindings {
	luabind::scope _sfml_audio() {
		return
			(
			luabind::class_<sf::Music>("sfMusic")
			.def(luabind::constructor<>())
			.def("openFromFile", &sf::Music::openFromFile)
			.def("play", &sf::Music::play)
			.def("pause", &sf::Music::pause)
			.def("setVolume", &sf::Music::setVolume)
			.def("setPitch", &sf::Music::setPitch)
			,

			luabind::class_<sf::SoundBuffer>("sfSoundBuffer")
			.def(luabind::constructor<>())
			.def("loadFromFile", &sf::SoundBuffer::loadFromFile)
			,

			luabind::class_<sf::Sound>("sfSound")
			.def(luabind::constructor<>())
			.def("play", &sf::Sound::play)
			.def("pause", &sf::Sound::pause)
			.def("setLoop", &sf::Sound::setLoop)
			.def("setVolume", &sf::Sound::setVolume)
			.def("setPitch", &sf::Sound::setPitch)
			);
	}
}