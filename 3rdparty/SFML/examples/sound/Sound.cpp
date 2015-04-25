
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio.hpp>
#include <iomanip>
#include <iostream>

#include <random>

float white() {
	static std::mt19937 generator = std::mt19937(std::random_device()());
	static std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
	return dist(generator);
}

////////////////////////////////////////////////////////////
/// Play a sound
///
////////////////////////////////////////////////////////////

class MyStream : public sf::SoundStream
{
public:

	MyStream() : sf::SoundStream() {
		initialize(2, 44100);
	}

private:

	virtual bool onGetData(Chunk& data)
	{
		const int samplesToStream = 3000;

		/* mathematic constants for low-pass filtering */
		float CUTOFF = 250.f;
		float RC = 1.0 / (CUTOFF * 2 * 3.14);
		float dt = 1.0 / 44100;
		float alpha = dt / (RC + dt);

		/* prepare data containers */
		m_samples.resize(samplesToStream);
		m_filteredSamples.resize(samplesToStream);

		m_filteredSamples[0] = m_samples[0];

		/* generate 3000 samples of brownian noise */
		for (int i = 0; i < samplesToStream; ++i) {
			/* random number from -0.5 to 0.5 */
			float r = white();
			m_brown += r;
			if (m_brown<-100.0f || m_brown>100.0f) m_brown -= r;
			m_samples[i] = m_brown*120.f;
		}
		
		/* apply low-pass filter */
		m_filteredSamples[0] = last_sample + (alpha*(m_samples[0] - last_sample));
		for (int i = 1; i < samplesToStream; i++)
			m_filteredSamples[i] = (m_filteredSamples[i - 1] + (alpha*(m_samples[i] - m_filteredSamples[i - 1])));
		
		last_sample = m_filteredSamples[samplesToStream - 1];

		/* return the results to SFML */
		data.samples = m_filteredSamples.data();
		data.sampleCount = samplesToStream;

		return true;
	}

	virtual void onSeek(sf::Time timeOffset)
	{
		// compute the corresponding sample index according to the sample rate and channel count
		//m_currentSample = static_cast<std::size_t>(timeOffset.asSeconds() * getSampleRate() * getChannelCount());
	}

	float m_brown = 0.f;
	sf::Int16 last_sample = 0u;
	std::vector<sf::Int16> m_samples;
	std::vector<sf::Int16> m_filteredSamples;
	//std::size_t m_currentSample;
};


void playSound()
{
    // Load a sound buffer from a wav file
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("resources/canary.wav"))
        return;

    // Display sound informations
    std::cout << "canary.wav:" << std::endl;
    std::cout << " " << buffer.getDuration().asSeconds() << " seconds"       << std::endl;
    std::cout << " " << buffer.getSampleRate()           << " samples / sec" << std::endl;
    std::cout << " " << buffer.getChannelCount()         << " channels"      << std::endl;

    // Create a sound instance and play it
    sf::Sound sound(buffer);
    //sound.play();
	
	MyStream mys;
	mys.play();
	sf::sleep(sf::milliseconds(4100000));

    // Loop while the sound is playing
    while (sound.getStatus() == sf::Sound::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        std::cout << "\rPlaying... " << std::fixed << std::setprecision(2) << sound.getPlayingOffset().asSeconds() << " sec   ";
        std::cout << std::flush;
    }
    std::cout << std::endl << std::endl;
}


////////////////////////////////////////////////////////////
/// Play a music
///
////////////////////////////////////////////////////////////
void playMusic()
{
    // Load an ogg music file
    sf::Music music;
    if (!music.openFromFile("resources/orchestral.ogg"))
        return;

    // Display music informations
    std::cout << "orchestral.ogg:" << std::endl;
    std::cout << " " << music.getDuration().asSeconds() << " seconds"       << std::endl;
    std::cout << " " << music.getSampleRate()           << " samples / sec" << std::endl;
    std::cout << " " << music.getChannelCount()         << " channels"      << std::endl;

    // Play it
    music.play();

    // Loop while the music is playing
    while (music.getStatus() == sf::Music::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        std::cout << "\rPlaying... " << std::fixed << std::setprecision(2) << music.getPlayingOffset().asSeconds() << " sec   ";
        std::cout << std::flush;
    }
    std::cout << std::endl;
}


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    // Play a sound
    playSound();

    // Play a music
    playMusic();

    // Wait until the user presses 'enter' key
    std::cout << "Press enter to exit..." << std::endl;
    std::cin.ignore(10000, '\n');

    return EXIT_SUCCESS;
}
