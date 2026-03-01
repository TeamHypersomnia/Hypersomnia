precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D light_texture;

uniform int global_time;

const int light_levels = 3;
const int light_step = 255/light_levels;

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}



// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

void main() 
{
	vec2 texcoord = gl_FragCoord.xy;
	texcoord.x /= float(textureSize(light_texture, 0).x);
	texcoord.y /= float(textureSize(light_texture, 0).y);

	vec4 light = texture(light_texture, texcoord);

	float origint = max(max(light.r, light.g), light.b);
	float intensity = origint;
	intensity = float(
		
		light_step * (int(intensity * 255.0) / light_step + light_levels)

		) / 255.0;
	light.rgb *= intensity;

	vec4 texComponent = texture(basic_texture, theTexcoord);
	vec4 pixel = theColor * texComponent * light;
	pixel.r = min(texComponent.r, pixel.r);
	pixel.g = min(texComponent.g, pixel.g);
	pixel.b = min(texComponent.b, pixel.b);

	// ---- FILM GRAIN (based on light intensity) ----
	vec3 inputs = vec3( gl_FragCoord.xy, float(global_time) );
	float grain = random( inputs );
	grain = grain * 2.0 - 1.0;

	float shadowWeight = 0.0 - intensity;
	shadowWeight = pow(shadowWeight, 1.5);

	pixel.rgb *= (1.0 - grain * 0.8 * max(0.0, 1.0 - origint));
	pixel.rgb = clamp(pixel.rgb, 0.0, 1.0);

	outputColor = pixel;
}