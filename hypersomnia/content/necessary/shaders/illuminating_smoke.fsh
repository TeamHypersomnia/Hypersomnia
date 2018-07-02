#version 300 es
precision mediump int;
precision mediump float;

in vec2 theTexcoord;
out vec4 outputColor;

uniform sampler2D smoke_texture;

const int smoke_levels = 3;
const int smoke_step = 255/smoke_levels;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() 
{	
	vec4 smoke = texture(smoke_texture, theTexcoord);

	float smoke_intensity = smoke.a;//max(max(smoke.r, smoke.g), smoke.b);
	int smoke_step_number = (int(smoke_intensity * 255.0) / smoke_step);

	if(smoke_step_number <= 0)
		discard;
		//else
		//smoke_step_number = 2;

	smoke_intensity = float(
		
		smoke_step * (smoke_step_number)

		) / 255.0;

	{
		vec3 smoke_hsv = rgb2hsv(smoke.rgb);
		vec3 colorful_smoke = hsv2rgb(vec3(smoke_hsv.x, smoke_hsv.y, 1.0));
	
		smoke.a = smoke_intensity;
		smoke.rgb = colorful_smoke;
	}

	outputColor = smoke;
}