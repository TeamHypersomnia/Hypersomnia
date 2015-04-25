#pragma once
#include <SFML/Audio.hpp>
#include <random>

namespace helpers {
	struct sound_filters {
		float lowpass_cutoff = -1.f;
		float lowpass_interpolant = 0.f;

		sound_filters();

		void process_samples(const std::vector<sf::Int16>&);

		sf::Int16 last_sample = 0u;
		std::vector<sf::Int16> m_filteredSamples;
	};

	struct filtered_sound : sound_filters, sf::SoundStream {
		virtual bool onGetData(Chunk& data) { return false;  }
		virtual void onSeek(sf::Time timeOffset) {}
	};

	struct filtered_music : public filtered_sound {


	};

	struct noise_generator : public filtered_sound {
		std::mt19937 generator;
		std::uniform_real_distribution<float> distribution;

		noise_generator();

		virtual bool onGetData(Chunk& data);
		virtual void onSeek(sf::Time timeOffset);

		float m_brown = 0.f;
		std::vector<sf::Int16> m_samples;
	};
}