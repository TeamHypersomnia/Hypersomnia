#version 330
smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D light_texture;

const int light_levels = 3;
const int light_step = 255/light_levels;

void main() 
{
	vec2 texcoord = gl_FragCoord.xy;
	texcoord.x /= textureSize(light_texture, 0).x; 
	texcoord.y /= textureSize(light_texture, 0).y;

	vec4 light = texture(light_texture, texcoord);
	//light.r = float(light_step * (int(light.r * 255.0) / light_step)) / 255.0;
	//light.g = float(light_step * (int(light.g * 255.0) / light_step)) / 255.0;
	//light.b = float(light_step * (int(light.b * 255.0) / light_step)) / 255.0;
	//light.a = float(light_step * (int(light.a * 255.0) / light_step)) / 255.0;

	float intensity = max(max(light.r, light.g), light.b);
	
	int level = (int(intensity * 255.0) / light_step + light_levels);

	if(level < 4)
		discard;

	intensity = float(
		
		light_step * (level + 2)

		) / 255.0;
	light.rgb *= intensity;

	vec4 pixel = theColor * texture(basic_texture, theTexcoord) * light;
	outputColor = pixel;
}