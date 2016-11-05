#version 330
smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D light_texture;

const int levels = 4;
const int step = 255/levels;

void main() 
{
	vec2 texcoord = gl_FragCoord.xy;
	texcoord.x /= 1200; 
	texcoord.y /= 800;

	vec4 light = texture(light_texture, texcoord);
	//light.r = float(step * (int(light.r * 255.0) / step)) / 255.0;
	//light.g = float(step * (int(light.g * 255.0) / step)) / 255.0;
	//light.b = float(step * (int(light.b * 255.0) / step)) / 255.0;
	//light.a = float(step * (int(light.a * 255.0) / step)) / 255.0;

	float intensity = max(max(light.r, light.g), light.b);
	intensity = float(
		
		step * (int(intensity * 255.0) / step + levels)

		) / 255.0;
	light.rgb *= intensity;

	vec4 pixel = theColor * texture(basic_texture, theTexcoord) * light;
	outputColor = pixel;
}