//#include "sound_effects.h"
//#define SAMPLES_TO_STREAM 1000
//
//namespace helpers {
//	sound_filters::sound_filters() {
//		m_filteredSamples.resize(SAMPLES_TO_STREAM);
//	}
//
//	void sound_filters::process_samples(const std::vector<sf::Int16>& samples) {
//		/* mathematic constants for low-pass filtering */
//		
//		if (lowpass_cutoff >= 0.f) {
//			static float smoothing_average_factor = 0.5;
//			static float averages_per_sec = 7;
//
//			static float averaging_constant = static_cast<float>(
//				pow(smoothing_average_factor, averages_per_sec * SAMPLES_TO_STREAM / 44000.f)
//				);
//
//			lowpass_interpolant = lowpass_interpolant  * averaging_constant + lowpass_cutoff * (1.0f - averaging_constant);
//
//			float RC = 1.0 / (lowpass_interpolant * 2 * 3.14);
//			float dt = 1.0 / 44100;
//			float alpha = dt / (RC + dt);
//
//			m_filteredSamples[0] = samples[0];
//
//			/* apply low-pass filter */
//			m_filteredSamples[0] = last_sample + (alpha*(samples[0] - last_sample));
//			for (int i = 1; i < SAMPLES_TO_STREAM; i++)
//				m_filteredSamples[i] = (m_filteredSamples[i - 1] + (alpha*(samples[i] - m_filteredSamples[i - 1])));
//
//			last_sample = m_filteredSamples[SAMPLES_TO_STREAM - 1];
//		}
//	}
//
//	noise_generator::noise_generator() : generator(std::random_device()()), distribution(-0.5f, 0.5f) {
//		initialize(2, 44100);
//
//		/* prepare data containers */
//		m_samples.resize(SAMPLES_TO_STREAM);
//	}
//
//	bool noise_generator::onGetData(Chunk& data)
//	{
//		/* generate 3000 samples of brownian noise */
//		for (int i = 0; i < SAMPLES_TO_STREAM; ++i) {
//			/* random number from -0.5 to 0.5 */
//			float r = distribution(generator);
//			m_brown += r;
//			if (m_brown<-100.0f || m_brown>100.0f) m_brown -= r;
//			m_samples[i] = m_brown*120.f;
//		}
//
//		process_samples(m_samples);
//
//		/* return the results to SFML */
//		data.samples = m_filteredSamples.data();
//		data.sampleCount = SAMPLES_TO_STREAM;
//
//		return true;
//	}
//
//	void noise_generator::onSeek(sf::Time timeOffset) {}
//}