---
name: audio-processing
description: Process music and ambience for Hypersomnia maps - cut a track into phrases (riffs) separated by silence, give every riff a long natural fadeout made of its own sustain (forward, then mirrored), loop it seamlessly, find each riff's first note, last note and where the next theme starts, and put the result into an editor project (file + BLAKE3 file_hash). Use for "obróbka audio", "zrób pętlę z muzyki", "potnij utwór na riffy", "fadeout/ogon riffu", "dodaj riff", "muzyka do mapy/tutoriala", "ambient loop", or any change to a map's music file.
---

# Audio processing

Tools: python3 with numpy (no scipy), ffmpeg, matplotlib, rubberband if a stretch is ever needed. General tools live in
`scripts/` here; **what is specific to one track** (its phrase config and a script that builds it into the game) goes
to `cmake/asset_scripts/` - e.g. `cut_tutorial_music.sh` with `tutorial_music_phrases.json` is a complete worked example.
Keep the downloaded source and experiments in a persistent folder outside the scratchpad.

## When this method fits

Tracks whose phrases end in a **held chord or pad** (ambient, synthwave pads, slow piano with pedal): each riff's own
sustain is long and steady enough to carry its fadeout. It does **not** fit tracks with a continuous beat (drums,
hi-hats, arpeggiated basslines) - the mirrored part would be reversed hits; those need cuts on the beat instead.

## Phrase loop

`scripts/phrase_loop.py <config.json> <source> <out.ogg>` - riffs cut out of the source, each with a fade-in, a long
fadeout and silence, the last one wrapping back to the first from silence:

```json
{
	"riff_fade_secs": 2.5,
	"tail_after_riff_secs": 3.5,
	"silence_after_tail_secs": 2.5,
	"default_fade_in_secs": 3.0,
	"phrases": [
		{"start": 0.0, "end_quietest_near": 16.4, "fade_in_secs": 1.5, "turn": 16.3, "backward_high_pass_hz": 250},
		{"fade_from": 26.8, "turn": 28.7, "backward_high_pass_hz": 250},
		{"start": 25.8, "end": 37.55, "turn": 37.25}
	]
}
```

- A phrase starts where the previous one ended unless it has `start` - it may start inside the previous one's sustain,
  so the same audio plays twice (one riff's tail, the next one's fade-in); that is fine.
- Its fadeout starts at `fade_from`, or `riff_fade_secs` before `end`, or before the quietest point within 0.6 s of
  `end_quietest_near`; it lasts `riff_fade_secs + tail_after_riff_secs` (a phrase may set its own
  `tail_after_riff_secs`) with gain `10^(-50 x^p / 20) * (1 - x^8)`, then `silence_after_tail_secs` of silence.
  `p` is the phrase's `fade_power`, 1.5 by default; a higher one (e.g. 2.5) keeps the louder part of the fadeout longer
  and drops faster at the end - what "a longer fadeout" usually means, a longer `tail_after_riff_secs` alone mostly
  lengthens the quiet end.
- Fade-in: dB-linear from -40 dB over `fade_in_secs`, or `min(default_fade_in_secs, 30% of the phrase)`.
- **The tail is the riff's own audio**: from `fade_from` it keeps playing to `turn`, then plays backward from that same
  sample (20 ms splice) - one continuous waveform, nothing overlaps, nothing is synthesized. The backward part is never
  louder than the forward decay continued, and `backward_high_pass_hz` removes its bass.

## Choosing the points

Start with `scripts/inspect_audio.py suggest <source> <riff_from>-<next_theme_near>,...  [fadeout secs, default 6]` -
rough ranges in, per riff: first note -> `start`/`fade_in_secs`, next theme onset -> `turn`, last note and how fast
the level falls after it -> `fade_from` (and its latest value), and whether the backward part needs the high-pass.
Then confirm with `inspect_audio.py bands <source> <from> <to>` (levels of 30-120 / 120-250 / 250-500 / 500-1000 /
1000-2500 / 2500-8000 Hz and the attacks):

- **Fade-in ends right before the first note** (`start` = first note - fade-in, over the previous riff's sustain).
  A fade-in still running at the first notes makes them nearly inaudible - the ear must catch the first note.
- **The fadeout starts after the last note rang out**: find the band the last note lives in (often the bass) and
  start when it is ~15-20 dB below its peak. Starting on a note's peak sounds "too quiet too fast, the riff doesn't
  play fully".
- **`turn` = the last moment of steady sustain before the next theme's first rise** - a band jumping 12-20 dB or a
  strong attack. Even 0.2 s of the next theme before the turn is heard at the end of the fadeout.
- Keep `turn - fade_from` >= ~1.9 s; the backward part covers the rest of the fadeout.
- **High-pass the backward part** whenever it reaches back to where the riff still had its bass (the low bands far
  louder than at the turn) - otherwise the bass swells back up and sounds like a new theme coming in. At the turn the
  bass has already decayed, so the filter is inaudible there.

## Check every build

- `inspect_audio.py bands <out> <turn> <fadeout end> 4`: after the turn no audible band keeps rising (short wobbles of a
  few dB are the music's own).
- `inspect_audio.py levels <out> 0 <length> 2`: riffs at a similar level.
- `inspect_audio.py spectrogram <out> <from> <to> <png> <marks>` (up to 8 kHz): no vertical line (a step) where a tail
  starts, no new horizontal lines (notes) in a tail.
- When only some riffs change, the rest must stay bit-identical: decode both versions, compare samples (max difference
  0.0; a riff that only moved in time differs by Vorbis noise).
- Open just the file for the user: `audacity <out.ogg> &`, no doubled preview copies.

## Tried and rejected - don't offer these again

- An **ambience bed** (another track) in the gaps, or always on and ducked under the riffs.
- **Spectral freeze with random phases** (Paulstretch-like) - grainy, "vibrating"; a tail starting louder than the riff
  at that point also makes an audible step.
- **Phase-locked phase-vocoder freeze** - sounds like the audio hung.
- **Reverb tail** - "an ugly echo".
- **Looping a sustain by crossfading two copies of it** - chorus-like trembling during the overlap.
- **Donor sustain from elsewhere in the track** - other parts carry extra layers or notes.
- **Neural generation** (MusicGen continuation etc.) - MusicGen weights are CC-BY-NC, Steam requires an AI-content
  disclosure, and it adds its own notes instead of holding the chord.

## Putting it into a map

`scripts/swap_audio_resource.py <project.json> <resource id> <new.ogg>` copies the file over the resource's `path`
(unless it already is that file) and rewrites its `file_hash` (BLAKE3; the helper is built once from
`src/3rdparty/blake3` into `~/.cache`). It refuses to run while `Hypersomnia` is running - the editor would overwrite
the project. Back up or commit the project JSON first. Map music is a sound resource with `"loop": true,
"spatialize": false` placed as a node; sounds longer than ~4-5 s use the music volume.

Sources must be CC0 or credited at hypersomnia.io/credits (`views/credits.ejs` in the site repo, "External assets").
