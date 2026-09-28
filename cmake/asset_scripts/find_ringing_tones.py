#!/usr/bin/env python3
#
# Finds ringing tones in sounds - narrow frequency peaks sustained through the sound, unlike knocks, which are brief and broadband.
# Prints, for every file, the strongest peaks above 1500 Hz with how persistent they are (1 = in every frame),
# then how many files share each peak - tones found in most files, like the ringing of a casing, are worth notching out,
# see RINGING_TONES in cut_shell_floor_hits.sh.
#
# Usage: cmake/asset_scripts/find_ringing_tones.py hypersomnia/content/sfx/shell_floor_hit_second_*.ogg
# Needs numpy and ffmpeg.

import subprocess
import sys
from collections import Counter

import numpy as np

SAMPLE_RATE = 44100
FRAME = 4096
HOP = 1024

MIN_FREQUENCY = 1500
PEAK_ABOVE_MEDIAN = 10
MERGE_WITHIN_HZ = 150
PEAKS_PER_FILE = 8
PERSISTENT_ENOUGH = 0.5


def load(path):
	raw = subprocess.run(
		['ffmpeg', '-loglevel', 'error', '-i', path, '-ac', '1', '-ar', str(SAMPLE_RATE), '-f', 'f32le', '-'],
		capture_output=True,
		check=True
	).stdout

	return np.frombuffer(raw, np.float32)


def find_peaks(samples):
	window = np.hanning(FRAME)
	spectra = np.array([np.abs(np.fft.rfft(samples[i:i + FRAME] * window)) for i in range(0, len(samples) - FRAME, HOP)])
	frequencies = np.fft.rfftfreq(FRAME, 1 / SAMPLE_RATE)

	median = np.median(spectra, axis=1, keepdims=True) + 1e-9
	persistence = (spectra > median * PEAK_ABOVE_MEDIAN).mean(axis=0)
	energy = (spectra ** 2).mean(axis=0)

	peaks = []

	for i in np.argsort(-persistence * np.sqrt(energy))[:40]:
		frequency = frequencies[i]

		if frequency < MIN_FREQUENCY:
			continue

		if all(abs(frequency - f) > MERGE_WITHIN_HZ for f, _ in peaks):
			peaks.append((frequency, persistence[i]))

	return sorted(peaks)[:PEAKS_PER_FILE]


def main():
	shared = Counter()

	for path in sys.argv[1:]:
		samples = load(path)

		if len(samples) <= FRAME:
			print(f'{path}: too short')
			continue

		peaks = find_peaks(samples)
		print(f'{path}: ' + ', '.join(f'{int(f)} Hz ({p:.2f})' for f, p in peaks))

		for frequency, persistence in peaks:
			if persistence > PERSISTENT_ENOUGH:
				shared[int(round(frequency / 100) * 100)] += 1

	print()
	print('Persistent peaks shared by files: ' + ', '.join(f'~{f} Hz in {n}' for f, n in shared.most_common(12)))


if __name__ == '__main__':
	main()
