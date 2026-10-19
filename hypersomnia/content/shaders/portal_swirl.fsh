precision highp int;
precision highp float;

/*
	A portal's swirl. The game uses portal_mode 1 (the data below);
	portal_mode 0 is the analytic version - the fallback without the data, and a base for making new effects -
	fitted to the statistics of the pixel particle version, sampled in polar coordinates (r in the portal's radii):

	- the mean opacity: a linear ramp inside, a saturated rim at 0.95-1.05, an exponential halo outside,
	- times 1 + contrast * noise, the noise stretched along the circle, rotating, and changing over time,
	- in world pixels (pixel art).

	The noise is value noise on a polar lattice: each ring of cells has a whole number of cells around,
	so there is no seam at the angle's wrap.

	With portal_mode 1, the statistics come from data instead (portal_swirl_data.png, angles along x, radii up to 2R along y):
	three independent snapshots of the particles' grain, normalized by the local deviation, in RGB
	(a byte b stands for b / 255 * data_value_scale - data_value_offset),
	and in the last row the exact mean and deviation profiles in R and G, in bytes:
	alpha = (mean + deviation * grain) / data_profile_scale. All three snapshots are always mixed,
	with weights going around in turns (their squares always sum up to 1, keeping the deviation),
	each snapshot coming back at a random angle whenever its weight is zero, all co-rotating.
*/

smooth in vec4 theColor;
flat in vec4 portal;
out vec4 outputColor;

uniform float global_time;
uniform float zoom;
uniform int portal_mode;
uniform sampler2D basic_texture;

const float PI = 3.14159265;

const float pixel_size = 2.0;

/* The mean opacity profile. */
const float interior_amount = 1.80;
const float interior_decline = 0.565;
const float rim_from = 0.895;
const float rim_full = 0.95;
const float rim_level = 0.975;
const float rim_until = 1.065;
const float rim_falloff = 0.038;
const float halo_at_117 = 0.08;
const float halo_decay = 0.22;

/* The grain. */
const float interior_contrast = 0.17;
const float rim_contrast = 0.03;

/* The grain past the rim, until the halo takes over. */
const float rim_outside_contrast = 0.3;
const float noise_normalization = 2.6;

/* The finer octave of the interior grain - the single particles' streaks. */
const float fine_cells_mult = 0.33;
const float fine_amount = 0.6;

/* The halo is streaky: its grain is strong, never going below zero. */
const float halo_contrast = 0.85;

/* Where the halo's grain takes over from the rim's, blended over +-halo_blend. */
const float halo_from = 1.09;
const float halo_blend = 0.03;

/*
	The streaks keep a constant angular width, so a ring of cells has the same number of them around at any radius.
*/

const float interior_cells_around = 17.0;
const float interior_cell_radial = 0.055;
const float halo_cells_around = 15.0;
const float halo_cell_radial = 0.076;

/*
	The grain decorrelates over time exponentially (time constant ~0.45 s inside, ~0.35 s in the halo) -
	approximated by two noises changing at different paces: fast_share of the variance in the fast one.
*/

const float interior_cell_secs = 0.45;
const float halo_cell_secs = 0.35;
const float slow_cell_secs_mult = 4.5;
const float fast_share = 0.6;

const float interior_spin_degrees = 9.5;
const float halo_spin_degrees = 7.0;

uint hash3(uvec3 v) {
	v = v * 1664525u + 1013904223u;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v ^= v >> 16u;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	return v.x ^ v.y ^ v.z;
}

float lattice(int i, int j, int k, uint seed) {
	return float(hash3(uvec3(uint(i), uint(j) ^ seed, uint(k))) & 0xFFFFu) / 32767.5 - 1.0;
}

float smooth01(float t) {
	return t * t * (3.0 - 2.0 * t);
}

/*
	One ring of cells (index j) at the angle theta: value noise along the circle (periodic, cells_around cells)
	and over time.
*/

float ring_noise(int j, float theta, float spin, float t_cells, float cells_around, uint seed) {
	int n = int(cells_around);

	float rotated = theta - spin * t_cells;
	float u = fract(rotated / (2.0 * PI)) * float(n);

	int i0 = int(floor(u));
	float fu = smooth01(u - float(i0));
	int i1 = i0 + 1;
	i0 = i0 % n;
	i1 = i1 % n;

	int k0 = int(floor(t_cells));
	float fk = smooth01(t_cells - float(k0));

	float a = mix(lattice(i0, j, k0, seed), lattice(i1, j, k0, seed), fu);
	float b = mix(lattice(i0, j, k0 + 1, seed), lattice(i1, j, k0 + 1, seed), fu);

	return mix(a, b, fk);
}

float polar_noise_paced(float r, float theta, float cells_around, float cell_radial, float cell_secs, float spin_degrees, uint seed) {
	float r_cells = r / cell_radial;
	int j0 = int(floor(r_cells));
	float fj = smooth01(r_cells - float(j0));

	float t_cells = global_time / cell_secs;

	/* The spin per one time cell. */
	float spin = radians(spin_degrees) * cell_secs;

	float a = ring_noise(j0, theta, spin, t_cells, cells_around, seed);
	float b = ring_noise(j0 + 1, theta, spin, t_cells, cells_around, seed);

	return mix(a, b, fj);
}

float polar_noise(float r, float theta, float cells_around, float cell_radial, float cell_secs, float spin_degrees, uint seed) {
	float fast = polar_noise_paced(r, theta, cells_around, cell_radial, cell_secs, spin_degrees, seed);
	float slow = polar_noise_paced(r, theta, cells_around, cell_radial, cell_secs * slow_cell_secs_mult, spin_degrees, seed + 15485863u);

	return (sqrt(fast_share) * fast + sqrt(1.0 - fast_share) * slow) * noise_normalization;
}

float mean_opacity(float r) {
	float interior = interior_amount * (1.0 - exp(-r)) - interior_decline * r;

	if (r < rim_until) {
		return mix(interior, rim_level, smoothstep(rim_from, rim_full, r));
	}

	float falloff = rim_level * exp(-(r - rim_until) / rim_falloff);
	float halo = halo_at_117 * exp(-(r - 1.17) / halo_decay);

	return max(falloff, halo);
}

/*
	portal_mode 1: the data.

	The texture's last row holds the profiles, the rest - the snapshots, angles along x, radii up to data_max_r along y.
	Sampled bilinearly (the angle wrapping around), so that zoomed in it does not look blocky.

	While a snapshot is mixed in (from one zero of its weight to the next), its grain flows outwards at data_outflow
	(in the portal's radii per second), as the particles drifting out and leaking past the rim do -
	inside and in the halo, never across the rim's edge, where it would pull the inside's grain into the steep edge.
	The flow fades out and in over wide ranges of the radius, so that the sampled radius never folds back.
*/

const float data_max_r = 2.0;
const float data_cycle_secs = 1.1;
const float data_spin_degrees = 9.5;
const float data_halo_spin_degrees = 7.0;
const float data_outflow = 0.13;
const float data_pixel_size = 1.0;
const float data_edge_grain_cut = 0.85;

/*
	global_time comes wrapped at data_period_cycles * data_cycle_secs = 7920 s (portal_swirl_period_secs),
	a whole number of turns of both spins (209 and 154), so the swirl flows on with no jump,
	and the time keeps its float precision however long the game runs.
*/

const int data_period_cycles = 7200;

/*
	The brightest grain leans towards its own color (see draw_portal_swirls).
*/

const float data_grain_color_max = 0.5;
const float data_grain_color_from_grain = 0.5;
const float data_grain_color_full_grain = 2.0;

const float data_value_scale = 8.0;
const float data_value_offset = 3.0;
const float data_profile_scale = 248.0;

/*
	The weights' squares sum up to 1, so their sum is at most sqrt(3) - times the largest grain value.
*/

const float data_max_abs_grain = 1.7321 * (data_value_scale - data_value_offset);

float data_row(float r, int radii) {
	return clamp(r / data_max_r * float(radii) - 0.5, 0.0, float(radii - 1));
}

float data_channel(vec3 texel, int layer) {
	return layer == 0 ? texel.r : (layer == 1 ? texel.g : texel.b);
}

/*
	Bilinear in hardware: the texture repeats around the circle,
	and the row stays at most at the last snapshot row's center, never blending in the profile's row.
*/

float data_layer(int layer, float r, float theta_turns, ivec2 size) {
	int radii = size.y - 1;

	vec2 uv = vec2(fract(theta_turns), (data_row(r, radii) + 0.5) / float(size.y));

	return data_channel(texture(basic_texture, uv).rgb, layer) * data_value_scale - data_value_offset;
}

float appearance_offset(int k, uint seed) {
	return float(hash3(uvec3(uint(k), seed, 977u)) & 0xFFFFu) / 65536.0;
}

float data_alpha(float r, float theta, uint seed, out float grain) {
	ivec2 size = textureSize(basic_texture, 0);
	int radii = size.y - 1;

	float row = data_row(r, radii);
	int row0 = int(floor(row));
	int row1 = min(row0 + 1, radii - 1);
	vec2 profile = mix(
		texelFetch(basic_texture, ivec2(row0, radii), 0).rg,
		texelFetch(basic_texture, ivec2(row1, radii), 0).rg,
		row - float(row0)
	) * 255.0;

	/* Below a quarter of the 8-bit step whatever the grain - skips the snapshots' fetches where the profile fades out. */
	if (profile.x + data_max_abs_grain * profile.y < data_profile_scale / 1020.0) {
		return 0.0;
	}

	grain = 0.0;

	float spin = radians(mix(data_spin_degrees, data_halo_spin_degrees, smoothstep(1.0, 1.15, r)));
	float turns = (theta - spin * global_time) / (2.0 * PI);

	float outflow_weight = 1.0 - smoothstep(0.7, 0.9, r) + smoothstep(1.2, 1.4, r);

	float z = 0.0;

	for (int layer = 0; layer < 3; ++layer) {
		float phase = global_time / data_cycle_secs - float(layer) / 3.0;
		float since_zero = phase + 0.5;
		/* floor(since_zero) is at least -1. */
		int appearance = ((int(floor(since_zero)) + data_period_cycles) % data_period_cycles) * 3 + layer;
		float age_secs = fract(since_zero) * data_cycle_secs;
		float weight = sqrt((1.0 + cos(2.0 * PI * phase)) / 3.0);

		float flow = data_outflow * age_secs * outflow_weight;
		float flowed_r = max(0.0, r - flow);

		z += weight * data_layer(layer, flowed_r, turns + appearance_offset(appearance, seed), size);
	}

	/*
		Across the rim's outer edge the brightness drops steeply, so the deviation there is huge, and the snapshots,
		each with its edge somewhere else, would add up into sharp contours - there the mean profile alone, already soft.
	*/

	float edge_band = smoothstep(0.95, 1.0, r) * (1.0 - smoothstep(1.12, 1.2, r));
	z *= 1.0 - data_edge_grain_cut * edge_band;
	grain = z;

	return (profile.x + profile.y * z) / data_profile_scale;
}

void main() {
	/*
		See draw_portal_swirls: the radius is whole screen pixels with the seed (12 bits) in the fraction,
		and the grain's color comes packed as 0xRRGGBB.
	*/

	vec2 center = portal.xy;
	float radius_px = floor(portal.z);
	uint seed = uint(round((portal.z - radius_px) * 4096.0)) & 4095u;
	uint grain_packed = uint(portal.w);
	vec3 grain_color = vec3(uvec3(grain_packed >> 16u, grain_packed >> 8u, grain_packed) & 255u) / 255.0;

	/* Pixel art: the center of the world pixel this fragment is in. */
	float pixel_px = max(zoom * (portal_mode == 1 ? data_pixel_size : pixel_size), 1.0);
	vec2 rel = (floor((gl_FragCoord.xy - center) / pixel_px) + 0.5) * pixel_px;

	/* The world's y goes down. */
	vec2 q = vec2(rel.x, -rel.y) / radius_px;
	float r = length(q);

	if (r > 2.0) {
		discard;
	}

	float theta = atan(q.y, q.x);

	if (portal_mode == 1) {
		float grain = 0.0;
		float alpha = clamp(data_alpha(r, theta, seed, grain), 0.0, 1.0);

		if (alpha <= 0.0) {
			discard;
		}

		float grain_t = data_grain_color_max * smoothstep(data_grain_color_from_grain, data_grain_color_full_grain, grain);

		outputColor = vec4(mix(theColor.rgb, grain_color, grain_t), theColor.a * alpha);
		return;
	}

	float base = mean_opacity(r);

	/*
		Inside the rim's grain, outside the halo's, blended across halo_from.
	*/

	float halo_t = smoothstep(halo_from - halo_blend, halo_from + halo_blend, r);
	float alpha = 0.0;

	if (halo_t < 1.0) {
		float n = polar_noise(r, theta, interior_cells_around, interior_cell_radial, interior_cell_secs, interior_spin_degrees, seed);

		n += fine_amount * polar_noise(
			r, theta,
			floor(interior_cells_around / fine_cells_mult), interior_cell_radial * fine_cells_mult, interior_cell_secs * 0.5,
			interior_spin_degrees, seed + 104729u
		);

		float rim_t = smoothstep(rim_from, rim_full, r);
		float contrast = mix(interior_contrast, rim_contrast, rim_t);
		contrast = mix(contrast, rim_outside_contrast, smoothstep(rim_until - 0.01, rim_until + 0.04, r));

		alpha += (1.0 - halo_t) * base * (1.0 + contrast * n);
	}

	if (halo_t > 0.0) {
		float n = polar_noise(r, theta, halo_cells_around, halo_cell_radial, halo_cell_secs, halo_spin_degrees, seed + 7919u);
		alpha += halo_t * base * max(0.0, 1.0 + halo_contrast * n);
	}

	alpha = clamp(alpha, 0.0, 1.0);

	if (alpha <= 0.0) {
		discard;
	}

	outputColor = vec4(theColor.rgb, theColor.a * alpha);
}
