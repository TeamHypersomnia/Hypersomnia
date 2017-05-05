#include "augs/ensure.h"
#include "augs/filesystem/file.h"

#include "augs/audio/sound_data.h"
#include "augs/audio/sound_samples_from_file.h"

#include "augs/build_settings/setting_log_audio_files.h"

#include <vorbis/vorbisfile.h>

#define OGG_BUFFER_SIZE 32768 // 32 KB buffers

namespace augs {
	sound_data get_sound_samples_from_file(const std::string& path) {
		augs::ensure_existence(path);

		const auto extension = augs::get_extension(path);

		sound_data new_data;
		new_data.channels = 1;

		if (extension == ".ogg") {
			std::vector<char> buffer;
			int endian = 0;             // 0 for Little-Endian, 1 for Big-Endian
			int bitStream = 0xdeadbeef;
			long bytes = 0xdeadbeef;
			char array[OGG_BUFFER_SIZE]; 
			FILE *f = nullptr;

			f = fopen(path.c_str(), "rb");

			OggVorbis_File oggFile;
			ov_open(f, &oggFile, NULL, 0);

			const auto* pInfo = ov_info(&oggFile, -1);
			new_data.channels = pInfo->channels;
			new_data.frequency = pInfo->rate;

			do {
				bytes = ov_read(&oggFile, array, OGG_BUFFER_SIZE, endian, 2, 1, &bitStream);
				buffer.insert(buffer.end(), array, array + bytes);
			} while (bytes > 0);

			new_data.samples.resize(buffer.size() / sizeof(sound_sample_type));
			std::memcpy(new_data.samples.data(), buffer.data(), buffer.size());

			ov_clear(&oggFile);
		}
		else if (extension == ".wav") {
			FILE* const wavFile = fopen(path.c_str(), "rb");

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

			wav_hdr wavHeader;
			ensure(fread(&wavHeader, 1, sizeof(wav_hdr), wavFile) > 0);
			ensure_eq(16, wavHeader.bitsPerSample);
			//ensure(wavHeader.SamplesPerSec == 44100);

			new_data.channels = wavHeader.NumOfChan;
			new_data.frequency = wavHeader.SamplesPerSec;
			new_data.samples.resize(wavHeader.Subchunk2Size / sizeof(sound_sample_type));
			fread(new_data.samples.data(), sizeof(char), wavHeader.Subchunk2Size, wavFile);

			fclose(wavFile);
		}

#if LOG_AUDIO_BUFFERS
		LOG("Sound: %x\nSample rate: %x\nChannels: %x\nFormat: %x\nLength in seconds: %x",
			path,
			info.samplerate,
			info.channels,
			info.format,
			new_data.compute_length_in_seconds());
#endif

		return new_data;
	}
}