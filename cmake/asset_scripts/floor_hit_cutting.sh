#!/usr/bin/env bash
#
# Shared by the scripts cutting recordings of shell drops into floor hit sounds - source it, don't run it.
#
# Expects SOURCE, OUT_DIR, PREFIX, PRE_ROLL, FIRST_FADE_OUT, SECOND_FADE_IN, SECOND_FADE_OUT, QUALITY and DROPS,
# then cut_all_drops writes, for every drop N:
#
#   ${PREFIX}_first_N.ogg  - from just before the first hit until just before the second one, faded out
#   ${PREFIX}_second_N.ogg - the rest of the drop, faded in and out
#
# DROPS lists for every drop: the onset of its first hit, the onset of the second hit, the end of the drop - in seconds,
# and optionally where the first part ends earlier - to leave out faint bounces between the two hits,
# which would sound like hits out of sync with the shell.
# Optional FILTERS - an ffmpeg filter chain - further processes both parts, e.g. to notch out ringing.
# The outputs are mono, so that the game can position them.
#
# cut and cut_to_peak cut single sounds too - cut_to_peak also brings the loudest sample to a peak in dB,
# for recordings much quieter or louder than the rest.

calc() {
	awk "BEGIN { printf \"%.4f\", $1 }"
}

cut_filters() {
	local start="$1" end="$2" fade_in="$3" fade_out="$4" extra_filters="${5:-}"
	local duration fade_out_start

	duration=$(calc "$end - $start")
	fade_out_start=$(calc "$duration - $fade_out")

	# Mono and floating point from the start, so that the peaks measured by cut_to_peak are those of the output -
	# otherwise the formats negotiated with and without its volume could differ, and so the filters in between.
	local filters="aformat=sample_fmts=flt:channel_layouts=mono,atrim=start=$start:end=$end,asetpts=PTS-STARTPTS"

	# afade with a zero duration would fall back to its default of a whole second.
	if [[ "$fade_in" != "0" ]]; then
		filters+=",afade=t=in:st=0:d=$fade_in"
	fi

	filters+=",afade=t=out:st=$fade_out_start:d=$fade_out"

	if [[ -n "$extra_filters" ]]; then
		filters+=",$extra_filters"
	fi

	echo "$filters"
}

cut() {
	local output="$1" start="$2" end="$3" fade_in="$4" fade_out="$5" extra_filters="${6:-}"

	ffmpeg -loglevel error -y -i "$SOURCE" -af "$(cut_filters "$start" "$end" "$fade_in" "$fade_out" "$extra_filters")" -ac 1 -c:a libvorbis -q:a "$QUALITY" "$output"

	echo "$output: $start - $end"
}

cut_to_peak() {
	local output="$1" start="$2" end="$3" fade_in="$4" fade_out="$5" extra_filters="$6" peak_db="$7"
	local unnormalized measured_peak gain

	# Measured on what actually comes out, in floating point - so also above 0 dB, where loud recordings clip.
	unnormalized=$(mktemp --suffix=.wav)

	ffmpeg -loglevel error -y -i "$SOURCE" -af "$(cut_filters "$start" "$end" "$fade_in" "$fade_out" "$extra_filters")" -ac 1 -c:a pcm_f32le "$unnormalized"
	measured_peak=$(ffmpeg -i "$unnormalized" -af astats=measure_perchannel=none -f null - 2>&1 | awk '/Peak level dB:/ { print $NF }' | tail -1)
	rm -f "$unnormalized"

	gain=$(calc "$peak_db - ($measured_peak)")

	cut "$output" "$start" "$end" "$fade_in" "$fade_out" "${extra_filters:+$extra_filters,}volume=${gain}dB"
}

cut_all_drops() {
	mkdir -p "$OUT_DIR"

	local index=1

	for drop in "${DROPS[@]}"; do
		local first second end first_end
		read -r first second end first_end <<< "$drop"

		local first_start cut_point

		first_start=$(calc "($first - $PRE_ROLL) < 0 ? 0 : $first - $PRE_ROLL")
		cut_point=$(calc "$second - $PRE_ROLL")
		first_end="${first_end:-$cut_point}"

		cut "$OUT_DIR/${PREFIX}_first_$index.ogg" "$first_start" "$first_end" 0 "$FIRST_FADE_OUT" "${FILTERS:-}"
		cut "$OUT_DIR/${PREFIX}_second_$index.ogg" "$cut_point" "$end" "$SECOND_FADE_IN" "$SECOND_FADE_OUT" "${FILTERS:-}"

		index=$((index + 1))
	done
}
