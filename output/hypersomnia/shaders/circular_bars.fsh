#version 330
smooth in vec4 theColor;
in vec2 theTexcoord;
in vec2 startingAngleVec;
in vec2 endingAngleVec;
in vec2 endingAngleInsideVec;
in vec2 startingAngleInsideVec;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform vec2 texture_center;

float crossproduct2d(vec2 a, vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

void main() 
{
	vec4 pixel = texture(basic_texture, theTexcoord);
    
    vec2 v = theTexcoord - texture_center;
    normalize(v);

    vec2 lowerBound = startingAngleVec;
    vec2 upperBound = endingAngleVec;

    if(pixel.r > 0) {
    	lowerBound = startingAngleInsideVec;
    	upperBound = endingAngleInsideVec;
    }

    if(crossproduct2d(lowerBound, v) >= 0.0 && crossproduct2d(upperBound, v) <= 0.0) {
		outputColor = theColor * pixel.a;
    }
    else if(pixel.r > 0 && crossproduct2d(startingAngleVec, v) >= 0.0 && crossproduct2d(endingAngleVec, v) <= 0.0) {
		vec3 darkened_pixel = vec3(max(0, theColor.r - 0.5), max(0, theColor.g - 0.5), max(0, theColor.b - 0.5));

		outputColor = vec4(darkened_pixel.r, darkened_pixel.g, darkened_pixel.b, 0.3);
    }
    else discard;
}