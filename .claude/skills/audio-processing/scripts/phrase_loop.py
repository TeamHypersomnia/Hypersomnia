#!/usr/bin/env python3
import json
import subprocess
import sys
import numpy as np

SR = 44100
QUIETEST_POINT_SEARCH_SECS = 0.6
FADE_IN_FLOOR_DB = -40.0
FADE_IN_MAX_SHARE_OF_PHRASE = 0.3
TAIL_FLOOR_DB = -50.0
DEFAULT_FADE_POWER = 1.5
ENVELOPE_SMOOTHING_SECS = 0.25
MIRROR_SPLICE_SECS = 0.02
MIRROR_DECAY_FIT_SECS = 1.0
PEAK_LIMIT = 0.98


def decode(path):
	raw = subprocess.check_output(['ffmpeg', '-v', 'error', '-i', path, '-ac', '2', '-ar', str(SR), '-f', 'f32le', '-'])
	return np.frombuffer(raw, dtype=np.float32).reshape(-1, 2)


def encode(samples, path):
	subprocess.run(
		['ffmpeg', '-v', 'error', '-y', '-f', 'f32le', '-ar', str(SR), '-ac', '2', '-i', '-', '-c:a', 'libvorbis', '-q:a', '6', path],
		input=samples.astype(np.float32).tobytes(),
		check=True
	)


def envelope_db(mono, hop_secs):
	hop = int(hop_secs * SR)
	frames = np.lib.stride_tricks.sliding_window_view(mono, hop)[::hop]
	return 20 * np.log10(np.sqrt(np.mean(frames ** 2, axis=1)) + 1e-9)


def quietest_point(mono, around_secs, search_secs):
	hop_secs = 0.01
	env = envelope_db(mono, hop_secs)
	lo = int((around_secs - search_secs) / hop_secs)
	hi = int((around_secs + search_secs) / hop_secs)
	smoothed = np.convolve(np.pad(env, 4, mode='edge'), np.ones(9) / 9, mode='same')[4:-4]
	return (lo + int(np.argmin(smoothed[lo:hi]))) * hop_secs


def faded_in(segment, fade_in_secs):
	out = segment.copy()
	n_in = int(fade_in_secs * SR)

	if n_in > 0:
		t = np.linspace(0, 1, n_in)
		out[:n_in] *= (10 ** (FADE_IN_FLOOR_DB * (1 - t) / 20) * t)[:, None]

	return out


def smoothed_rms(samples):
	power = np.mean(samples ** 2, axis=1)
	width = int(ENVELOPE_SMOOTHING_SECS * SR)
	padded = np.pad(power, (width // 2, width - width // 2), mode='edge')
	sums = np.concatenate([[0.0], np.cumsum(padded)])
	return np.sqrt((sums[width:width + len(power)] - sums[:len(power)]) / width) + 1e-9


def overall_fade_gain(samples, fade_power):
	x = np.linspace(0, 1, samples)
	return 10 ** (TAIL_FLOOR_DB * x ** fade_power / 20) * (1 - x ** 8)


def high_passed(samples, cutoff_hz):
	padded = np.concatenate([samples, np.zeros_like(samples)])
	freqs = np.fft.rfftfreq(len(padded), 1 / SR)
	ramp = np.clip((freqs - 0.6 * cutoff_hz) / (0.4 * cutoff_hz), 0.0, 1.0)
	response = (0.5 - 0.5 * np.cos(np.pi * ramp))[:, None]
	return np.fft.irfft(np.fft.rfft(padded, axis=0) * response, n=len(padded), axis=0)[:len(samples)]


def mirrored_around(audio, from_sample, turn_sample, samples, backward_high_pass_hz):
	forward_length = turn_sample - from_sample
	half_splice = int(MIRROR_SPLICE_SECS / 2 * SR)
	k = np.arange(samples)
	forward = audio[np.minimum(from_sample + k, turn_sample + half_splice)]
	backward = audio[np.maximum(0, turn_sample - (k - forward_length))]

	if backward_high_pass_hz > 0:
		backward = high_passed(backward, backward_high_pass_hz)

	weight = np.clip((k - (forward_length - half_splice)) / (2 * half_splice), 0.0, 1.0)[:, None]
	return forward * (1 - weight) + backward * weight, forward_length


def mirrored_part_never_louder_than_the_decay(extension, forward_length):
	env_db = 20 * np.log10(smoothed_rms(extension))
	fit_from = forward_length - int(MIRROR_DECAY_FIT_SECS * SR)
	slope = min(0.0, np.polyfit(np.arange(fit_from, forward_length), env_db[fit_from:forward_length], 1)[0])
	k = np.arange(len(extension))
	continued_decay_db = env_db[forward_length] + slope * (k - forward_length)
	correction_db = np.where(k < forward_length, 0.0, np.minimum(0.0, continued_decay_db - env_db))
	return extension * (10 ** (correction_db / 20))[:, None]


def tail_of_own_sustain(audio, fade_start_sample, turn_secs, samples, backward_high_pass_hz, fade_power):
	extension, forward_length = mirrored_around(audio, fade_start_sample, int(turn_secs * SR), samples, backward_high_pass_hz)
	extension = mirrored_part_never_louder_than_the_decay(extension, forward_length)
	return extension * overall_fade_gain(samples, fade_power)[:, None]


def phrase_bounds(config, mono):
	bounds = []
	previous_end = 0.0

	for phrase in config['phrases']:
		start = phrase.get('start', previous_end)

		if 'fade_from' in phrase:
			end = phrase['fade_from'] + config['riff_fade_secs']
		elif 'end' in phrase:
			end = phrase['end']
		else:
			end = quietest_point(mono, phrase['end_quietest_near'], QUIETEST_POINT_SEARCH_SECS)

		bounds.append((start, end))
		previous_end = end

	return bounds


def build_loop(config, audio):
	mono = audio.mean(axis=1)
	riff_fade = int(config['riff_fade_secs'] * SR)
	pieces = []
	tails = []
	position = 0

	for phrase, (start, end) in zip(config['phrases'], phrase_bounds(config, mono)):
		samples = audio[int(start * SR):int(end * SR)]
		tail_after_riff_secs = phrase.get('tail_after_riff_secs', config['tail_after_riff_secs'])
		tail_samples = riff_fade + int(tail_after_riff_secs * SR)
		gap = np.zeros((int((tail_after_riff_secs + config['silence_after_tail_secs']) * SR), 2))
		fade_in_secs = phrase.get('fade_in_secs', min(config['default_fade_in_secs'], FADE_IN_MAX_SHARE_OF_PHRASE * (end - start)))
		body = faded_in(samples, fade_in_secs)
		body[-riff_fade:] = 0.0
		fade_start_sample = int(start * SR) + len(samples) - riff_fade
		tail = tail_of_own_sustain(
			audio,
			fade_start_sample,
			phrase['turn'],
			tail_samples,
			phrase.get('backward_high_pass_hz', 0.0),
			phrase.get('fade_power', DEFAULT_FADE_POWER)
		)
		tails.append((position + len(samples) - riff_fade, tail))
		pieces += [body, gap]
		print('phrase %.2f - %.2f s, fade in %.2f s, fade from %.2f s, turn %.2f s -> loop %.2f - %.2f s' % (start, end, fade_in_secs, fade_start_sample / SR, phrase['turn'], position / SR, (tails[-1][0] + tail_samples) / SR))
		position += len(samples) + len(gap)

	loop = np.concatenate(pieces)

	for at, tail in tails:
		loop[at:at + len(tail)] += tail

	peak = np.abs(loop).max()

	if peak > PEAK_LIMIT:
		loop *= PEAK_LIMIT / peak

	return loop


def main():
	config_path, source, out = sys.argv[1], sys.argv[2], sys.argv[3]
	config = json.load(open(config_path))
	loop = build_loop(config, decode(source))
	encode(loop, out)
	print(out, 'length %.2f s, peak %.2f' % (len(loop) / SR, np.abs(loop).max()))


if __name__ == '__main__':
	main()
