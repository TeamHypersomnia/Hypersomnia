precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;

/*
	Environment shadows.

	The shadow texture holds, per pixel:
	R - shadow height of the tallest caster whose shadow covers the pixel,
	G - strength of that shadow,
	B - shadow height of the physical body lying under the pixel (0 on the bare ground),
	A - coverage by NO_SHADOW areas, under which the sun never reaches.
*/

uniform sampler2D shadow_texture;

/*
	Displacement of the shadow per one level of shadow height, in fragment pixels.
*/

uniform vec2 shadow_step;

/*
	Zero disables receiving shadows in the current draw call.
*/

uniform float shadow_strength;

/*
	Maximum number of samples that guard against false shadows
	on the sunlit side of taller objects. Zero disables the guard.
*/

uniform int shadow_fix;

/*
	Negative means the receiver's height is read from the shadow texture.
*/

uniform float receiver_height;

vec4 fetch_shadow(highp vec2 frag_pos) {
	ivec2 size = textureSize(shadow_texture, 0);
	ivec2 texel = clamp(ivec2(floor(frag_pos)), ivec2(0), size - ivec2(1));
	return texelFetch(shadow_texture, texel, 0);
}

float to_shadow_level(float v) {
	return floor(v * 255.0 + 0.5);
}

float calc_shadow_amount() {
	if (shadow_strength <= 0.0) {
		return 0.0;
	}

	highp vec2 frag_pos = gl_FragCoord.xy;
	vec4 here = fetch_shadow(frag_pos);

	float sun_reaches = 1.0 - here.a;

	if (sun_reaches <= 0.0) {
		return 0.0;
	}

	float r = receiver_height >= 0.0 ? receiver_height : to_shadow_level(here.b);

	/*
		A receiver raised above the ground samples the ground shadow further from the sun
		so that the shadow of a taller caster lands on it where it would physically.
		That sample would also catch the base of a taller object standing right behind the receiver,
		so walk towards the sample point and bail out when a taller body is found there.
	*/

	if (r > 0.0 && shadow_fix > 0) {
		float path_len = r * length(shadow_step);
		int samples = min(shadow_fix, int(ceil(path_len / 2.0)));

		for (int i = 1; i <= 16; ++i) {
			if (i > samples) {
				break;
			}

			float t = r * float(i) / float(samples);

			if (to_shadow_level(fetch_shadow(frag_pos + t * shadow_step).b) > r) {
				return 0.0;
			}
		}
	}

	vec4 s = fetch_shadow(frag_pos + r * shadow_step);
	return to_shadow_level(s.r) > r ? s.g * shadow_strength * sun_reaches : 0.0;
}

void main() 
{
	vec4 pixel = theColor * texture(basic_texture, theTexcoord);

	/*
		Ground decals are drawn fully illuminated, so shadows darken them directly.
	*/

	pixel.rgb *= 1.0 - calc_shadow_amount();
	outputColor = pixel;
}