#pragma once
#include "stdafx.h"
//#include "bindings.h"

//#include "game/game/sound_effects.h"
//
//bool has_sound_stopped_playing(sf::Sound* s) {
//	return s->getStatus() == sf::Sound::Status::Stopped;
//}
//
//void sf_Listener_setPosition(float x, float y, float z) {
//	sf::Listener::setPosition(x, y, z);
//}
//
//void sf_Listener_setDirection(float x, float y, float z) {
//	sf::Listener::setDirection(x, y, z);
//}
//
//void sf_Listener_setGlobalVolume(float volume) {
//	sf::Listener::setGlobalVolume(volume);
//}
//
//
//namespace bindings {
//	luabind::scope _sfml_audio() {
//		return
//			(
//			luabind::def("has_sound_stopped_playing", has_sound_stopped_playing),
//
//			luabind::def("sf_Listener_setPosition", sf_Listener_setPosition),
//			luabind::def("sf_Listener_setDirection", sf_Listener_setDirection),
//			luabind::def("sf_Listener_setGlobalVolume", sf_Listener_setGlobalVolume),
//			
//			luabind::class_<sf::Music>("sfMusic")
//			.def(luabind::constructor<>())
//			.def("play", &sf::SoundStream::play)
//			.def("pause", &sf::SoundStream::pause)
//			.def("stop", &sf::SoundStream::stop)
//			.def("openFromFile", &sf::Music::openFromFile)
//			.def("setVolume", &sf::Music::setVolume)
//			.def("setPitch", &sf::Music::setPitch)
//			.def("setPlayingOffset", &sf::Music::setPlayingOffset)
//			.def("setLoop", &sf::Music::setLoop)
//			.def("setRelativeToListener", &sf::Music::setRelativeToListener)
//			.def("setMinDistance", &sf::Music::setMinDistance)
//			.def("setAttenuation", &sf::Music::setAttenuation)
//			.def("setPosition", (void (sf::SoundSource::*)(float x, float y, float z))&sf::Music::setPosition)
//			.def("setPlayingOffset", &sf::Music::setPlayingOffset),
//
//			luabind::class_<helpers::filtered_sound>("filtered_sound")
//			.def(luabind::constructor<>())
//			.def("play", &sf::SoundStream::play)
//			.def("pause", &sf::SoundStream::pause)
//			.def("stop", &sf::SoundStream::stop)
//			.def("setVolume", &sf::SoundStream::setVolume)
//			.def_readwrite("lowpass_cutoff", &helpers::filtered_sound::lowpass_cutoff)
//			,
//
//			luabind::class_<helpers::noise_generator, filtered_sound>("noise_generator")
//			.def(luabind::constructor<>())
//			,
//
//			luabind::class_<sf::SoundBuffer>("sfSoundBuffer")
//			.def(luabind::constructor<>())
//			.def("loadFromFile", &sf::SoundBuffer::loadFromFile)
//			,
//
//			luabind::class_<sf::Sound>("sfSound")
//			.def(luabind::constructor<>())
//			.def("setAttenuation", &sf::Sound::setAttenuation)
//			.def("setBuffer", &sf::Sound::setBuffer)
//			.def("setMinDistance", &sf::Sound::setMinDistance)
//			.def("setRelativeToListener", &sf::Sound::setRelativeToListener)
//			.def("setPlayingOffset", &sf::Sound::setPlayingOffset)
//			.def("pause", &sf::Sound::pause)
//			.def("setLoop", &sf::Sound::setLoop)
//			.def("setVolume", &sf::Sound::setVolume)
//			.def("setPitch", &sf::Sound::setPitch)
//			.def("play", &sf::Sound::play)
//			.def("setPosition", (void (sf::Sound::*)(float, float, float))(&sf::Sound::setPosition))
//			);
//	}
//}