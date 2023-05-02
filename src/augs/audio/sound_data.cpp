#if PLATFORM_UNIX
/* Necessary for some stuff in ogg library */
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include <cstring>

#if BUILD_SOUND_FORMAT_DECODERS
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#define OGG_BUFFER_SIZE 4096
#endif

#include "augs/misc/scope_guard.h"
#include "augs/audio/sound_data.h"
#include "augs/ensure.h"
#include "augs/filesystem/file.h"
#include "augs/audio/sound_data.h"
#include "augs/build_settings/setting_log_audio_files.h"

template <class T>
auto fclosed_unique(T* const ptr) {
	return std::unique_ptr<T, decltype(fclose)*>(ptr, fclose);
}

namespace augs {
	sound_data::sound_data(const path_type& path) {
		channels = 1;

		if (path.empty()) {
			throw sound_decoding_error("Failed to decode a sound file: empty path was passed.");
		}

#if BUILD_SOUND_FORMAT_DECODERS
		const auto extension = path.extension();
		const auto path_str = path.string();

		if (extension == ".ogg") {
			std::vector<char> buffer;
			// TODO: throw if the file fails to load as OGG
			// TODO: detect endianess
			int endian = 0;             // 0 for Little-Endian, 1 for Big-Endian
			int bitStream = 0xdeadbeef;
			long bytes = 0xdeadbeef;
			char array[OGG_BUFFER_SIZE]; 

			OggVorbis_File oggFile;

			if (0 != ov_fopen(path_str.c_str(), &oggFile)) {
				throw sound_decoding_error("Error! Failed to load %x.", path);
			}
			
			auto scope = scope_guard([&oggFile](){
				ov_clear(&oggFile);
			});

			const auto* pInfo = ov_info(&oggFile, -1);
			channels = pInfo->channels;
			frequency = pInfo->rate;

			do {
				bytes = ov_read(&oggFile, array, OGG_BUFFER_SIZE, endian, 2, 1, &bitStream);
				buffer.insert(buffer.end(), array, array + bytes);
			} while (bytes > 0);

			samples.resize(buffer.size() / sizeof(sound_sample_type));
			std::memcpy(samples.data(), buffer.data(), buffer.size());
		}
		else if (extension == ".wav") {
#if PLATFORM_UNIX
			auto wav_file = fclosed_unique(fopen(path_str.c_str(), "rbe"));
#else
			auto wav_file = fclosed_unique(fopen(path_str.c_str(), "rb"));
#endif

			if (!wav_file) {
				throw sound_decoding_error("Failed to decode %x: could not open the file for reading.", path);
			}

			typedef struct WAV_HEADER {
				/* RIFF Chunk Descriptor */
				uint8_t         RIFF[4];        // RIFF Header Magic header
				uint32_t        ChunkSize;      // RIFF Chunk Size
				uint8_t         WAVE[4];        // WAVE Header
												/* "fmt" sub-chunk */
				uint8_t         fmt[4];         // FMT header
				uint32_t        Subchunk1Size;  // Size of the fmt chunk
				uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
				uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
				uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
				uint32_t        bytesPerSec;    // bytes per second
				uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
				uint16_t        bitsPerSample;  // Number of bits per sample
												/* "data" sub-chunk */
				uint8_t         Subchunk2ID[4]; // "data"  string
				uint32_t        Subchunk2Size;  // Sampled data length
			} wav_hdr;

			wav_hdr wav_header = {};

			if (fread(&wav_header, 1, sizeof(wav_hdr), wav_file.get()) > 0) {
				if (wav_header.bitsPerSample == 16) {
					channels = wav_header.NumOfChan;
					frequency = wav_header.SamplesPerSec;
					samples.resize(wav_header.Subchunk2Size / sizeof(sound_sample_type));
					
					if (const auto bytes_read = fread(samples.data(), sizeof(char), wav_header.Subchunk2Size, wav_file.get());
						bytes_read != wav_header.Subchunk2Size
					) {
						throw sound_decoding_error("Failed to decode %x as WAV file.", path);
					}
				}
				else {
					throw sound_decoding_error(
						"%x is a %x-bit WAV. Only supporting 16-bit WAVs.", 
						path, 
						wav_header.bitsPerSample
					);
				}
			}
			else {
				throw sound_decoding_error("Failed to decode %x as WAV file.", path);
			}
			//ensure_eq(wav_header.SamplesPerSec, 44100);
		}
		else {
			throw sound_decoding_error("Failed to decode %x as a sound file: unknown extension.", path);
		}

#define MONO_TO_STEREO 1

#if MONO_TO_STEREO
		if (channels == 1) {
			channels = 2;

			decltype(samples) new_samples;
			new_samples.resize(samples.size() * 2);

			for (std::size_t i = 0; i < samples.size(); ++i) {
				new_samples[i * 2] = samples[i];
				new_samples[i * 2 + 1] = samples[i];
			}

			samples = std::move(new_samples);
		}
#endif

#if LOG_AUDIO_BUFFERS
		LOG("Sound: %x\nFrequency: %x\nChannels: %x\nLength in seconds: %x",
			path,
			frequency,
			channels,
			compute_length_in_seconds()
		);
#endif
#endif
	}
	
	double sound_data::compute_length_in_seconds() const {
		return static_cast<double>(samples.size()) / (frequency * channels);
	}
}
