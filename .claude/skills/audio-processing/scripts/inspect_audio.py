#!/usr/bin/env python3
import sys
import numpy as np
from phrase_loop import SR, decode

BANDS_HZ = [(30, 120), (120, 250), (250, 500), (500, 1000), (1000, 2500), (2500, 8000)]
BASS_BANDS = 2
FRAME = 4096
HOP = 1024
ATTACK_FLUX_OVER_MEDIAN = 1.4
SPECTROGRAM_TOP_HZ = 8000
AUDIBLE_BELOW_LOUDEST_DB = 30
STRONG_ATTACK_OVER_MEDIAN = 1.8
NOTE_RISE_DB = 12
NOTE_RISE_FRAMES = 8
ONSET_MERGE_SECS = 0.15
NEXT_THEME_SEARCH_SECS = 0.6
TURN_BEFORE_NEXT_THEME_SECS = 0.1
FADE_IN_SECS = 3.0
MIN_FORWARD_SECS = 1.9
LAST_NOTE_SEARCH_BEFORE_TURN_SECS = 1.0
RING_OUT_DROPS_DB = [10, 15, 20]
HIGH_PASS_NEEDED_BASS_RISE_DB = 12
SUGGESTED_HIGH_PASS_HZ = 250


def framed_magnitudes(mono, from_secs, to_secs, hop=HOP):
	segment = mono[int(from_secs * SR):int(to_secs * SR)]
	frames = np.lib.stride_tricks.sliding_window_view(segment, FRAME)[::hop] * np.hanning(FRAME)
	times = from_secs + (np.arange(len(frames)) * hop + FRAME / 2) / SR
	return np.abs(np.fft.rfft(frames, axis=1)), times


def band_levels_db(magnitudes):
	power = magnitudes ** 2
	freqs = np.fft.rfftfreq(FRAME, 1 / SR)
	return np.stack([10 * np.log10(power[:, (freqs >= lo) & (freqs < hi)].sum(axis=1) + 1e-12) for lo, hi in BANDS_HZ], axis=1)


def spectral_flux(magnitudes):
	return np.concatenate([[0.0], np.maximum(0, np.diff(np.log(magnitudes + 1e-6), axis=0)).sum(axis=1)])


def print_levels(mono, from_secs, to_secs, step_secs):
	def level_db(t0, t1):
		samples = mono[int(t0 * SR):int(t1 * SR)]
		return 20 * np.log10(np.sqrt(np.mean(samples ** 2)) + 1e-9)

	print(' '.join('%.2f:%.0f' % (t, level_db(t, t + step_secs)) for t in np.arange(from_secs, to_secs, step_secs)))


def print_bands(mono, from_secs, to_secs, every_nth_frame):
	magnitudes, times = framed_magnitudes(mono, from_secs, to_secs)
	bands = band_levels_db(magnitudes)
	print('time   ' + '  '.join('%4d-%-4d' % band for band in BANDS_HZ))

	for i in range(0, len(times), every_nth_frame):
		print('%.2f ' % times[i] + '  '.join('%9.0f' % level for level in bands[i]))


def print_attacks(mono, from_secs, to_secs):
	magnitudes, times = framed_magnitudes(mono, from_secs, to_secs, hop=512)
	flux = spectral_flux(magnitudes)
	median = np.median(flux[1:])
	strong = np.flatnonzero(flux > ATTACK_FLUX_OVER_MEDIAN * median)
	print('attacks (spectral flux / median):', ' '.join('%.2f(%.1f)' % (times[i], flux[i] / median) for i in strong))


def save_spectrogram(mono, from_secs, to_secs, out_png, marks):
	import matplotlib
	matplotlib.use('Agg')
	import matplotlib.pyplot as plt
	magnitudes, times = framed_magnitudes(mono, from_secs, to_secs, hop=512)
	spectrum_db = 20 * np.log10(magnitudes.T + 1e-7)
	freqs = np.fft.rfftfreq(FRAME, 1 / SR)
	top = np.searchsorted(freqs, SPECTROGRAM_TOP_HZ)
	fig, (spectrum_axis, level_axis) = plt.subplots(2, 1, figsize=(14, 8))
	spectrum_axis.imshow(spectrum_db[:top], origin='lower', aspect='auto', extent=[times[0], times[-1], 0, freqs[top]], vmin=spectrum_db.max() - 70, vmax=spectrum_db.max(), cmap='magma')
	hop = int(0.01 * SR)
	segment = mono[int(from_secs * SR):int(to_secs * SR)]
	level = 20 * np.log10(np.sqrt(np.mean(np.lib.stride_tricks.sliding_window_view(segment ** 2, hop)[::hop], axis=1)) + 1e-9)
	level_axis.plot(from_secs + np.arange(len(level)) * 0.01, level, lw=0.8)
	level_axis.set_xlim(from_secs, to_secs)
	level_axis.set_ylim(-80, -10)
	level_axis.grid(True)

	for mark in marks:
		spectrum_axis.axvline(mark, color='c', lw=0.8)
		level_axis.axvline(mark, color='c', lw=0.8)

	plt.tight_layout()
	plt.savefig(out_png, dpi=65)


def note_onsets(bands, flux, flux_median, times):
	audible = bands > bands.max(axis=1, keepdims=True) - AUDIBLE_BELOW_LOUDEST_DB
	onsets = []

	for i in range(NOTE_RISE_FRAMES, len(times)):
		rise = bands[i] - bands[i - NOTE_RISE_FRAMES:i].min(axis=0)
		band_rises = bool(np.any(audible[i] & (rise >= NOTE_RISE_DB)))
		strong_attack = flux[i] > STRONG_ATTACK_OVER_MEDIAN * flux_median

		if (band_rises or strong_attack) and (not onsets or times[i] - onsets[-1] > ONSET_MERGE_SECS):
			onsets.append(times[i])

	return onsets


def suggest_riff(mono, flux_median, riff_from, next_theme_near, total_fade_secs):
	magnitudes, times = framed_magnitudes(mono, max(0.0, riff_from - FADE_IN_SECS), next_theme_near + NEXT_THEME_SEARCH_SECS + FRAME / SR)
	bands = band_levels_db(magnitudes)
	onsets = note_onsets(bands, spectral_flux(magnitudes), flux_median, times)
	first_note = next((t for t in onsets if t >= riff_from), riff_from)
	near_next = [t for t in onsets if abs(t - next_theme_near) <= NEXT_THEME_SEARCH_SECS]
	next_theme = near_next[0] if near_next else next_theme_near
	turn = next_theme - TURN_BEFORE_NEXT_THEME_SECS
	last_notes = [t for t in onsets if first_note <= t < turn - LAST_NOTE_SEARCH_BEFORE_TURN_SECS]
	last_note = last_notes[-1] if last_notes else first_note
	overall_db = 10 * np.log10((10 ** (bands / 10)).sum(axis=1))
	after_last = times >= last_note
	peak_index = np.flatnonzero(after_last)[0] + int(np.argmax(overall_db[after_last][:int(1.5 * SR / HOP)]))
	ring_out = {}

	for drop in RING_OUT_DROPS_DB:
		below = np.flatnonzero((times > times[peak_index]) & (overall_db <= overall_db[peak_index] - drop))
		ring_out[drop] = times[below[0]] if len(below) else None

	latest_fade_from = turn - MIN_FORWARD_SECS
	backward_reaches = turn - (total_fade_secs - (turn - latest_fade_from))
	turn_index = int(np.argmin(np.abs(times - turn)))
	reached = (times >= backward_reaches) & (times <= turn)
	bass_rise = float((bands[reached, :BASS_BANDS] - bands[turn_index, :BASS_BANDS]).max())
	audible_bass = bool(np.any(bands[reached, :BASS_BANDS] > bands[reached].max(axis=1, keepdims=True) - AUDIBLE_BELOW_LOUDEST_DB))
	print('riff from %.2f, next theme near %.2f:' % (riff_from, next_theme_near))
	print('  first note %.2f -> "start": %.2f, "fade_in_secs": %.1f' % (first_note, max(0.0, first_note - FADE_IN_SECS), min(FADE_IN_SECS, first_note)))
	print('  next theme %.2f -> "turn": %.2f' % (next_theme, turn))
	print('  last note %.2f, then the overall level falls %s -> "fade_from" around there, at most %.2f' % (
		last_note,
		', '.join('%d dB by %s' % (drop, '%.2f' % t if t is not None else 'never') for drop, t in ring_out.items()),
		latest_fade_from
	))
	print('  backward part reaches %.2f: bass up to %+.0f dB over the turn -> %s' % (
		backward_reaches,
		bass_rise,
		'"backward_high_pass_hz": %d' % SUGGESTED_HIGH_PASS_HZ if bass_rise > HIGH_PASS_NEEDED_BASS_RISE_DB and audible_bass else 'no high-pass needed'
	))


def main():
	command, path = sys.argv[1], sys.argv[2]
	mono = decode(path).mean(axis=1)

	if command == 'suggest':
		riffs = [tuple(float(x) for x in r.split('-')) for r in sys.argv[3].split(',')]
		total_fade_secs = float(sys.argv[4]) if len(sys.argv) > 4 else 6.0
		whole, _ = framed_magnitudes(mono, 0.0, len(mono) / SR - FRAME / SR)
		flux_median = float(np.median(spectral_flux(whole)[1:]))

		for riff_from, next_theme_near in riffs:
			suggest_riff(mono, flux_median, riff_from, next_theme_near, total_fade_secs)

		return

	from_secs, to_secs = float(sys.argv[3]), float(sys.argv[4])

	if command == 'levels':
		print_levels(mono, from_secs, to_secs, float(sys.argv[5]) if len(sys.argv) > 5 else 0.5)
	elif command == 'bands':
		print_bands(mono, from_secs, to_secs, int(sys.argv[5]) if len(sys.argv) > 5 else 2)
		print_attacks(mono, from_secs, to_secs)
	elif command == 'spectrogram':
		save_spectrogram(mono, from_secs, to_secs, sys.argv[5], [float(x) for x in sys.argv[6].split(',')] if len(sys.argv) > 6 else [])


if __name__ == '__main__':
	main()
