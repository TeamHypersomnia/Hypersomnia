#version 130

precision mediump int;
precision mediump float;

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

bool is_between(
    vec2 first, 
    vec2 second, 
    vec2 candidate
) {
	if (first == second) {
		return true;
	}

    bool boundary_winding_cw = crossproduct2d(first, second) > 0.f;

    if (boundary_winding_cw) {
        return crossproduct2d(candidate, second) >= 0.f && crossproduct2d(first, candidate) >= 0.f;
    }
    else {
        return !(crossproduct2d(candidate, first) > 0.f && crossproduct2d(second, candidate) > 0.f);
    }
}

void main() 
{
	vec4 pixel = texture(basic_texture, theTexcoord);
    
    vec2 v = theTexcoord - texture_center;
    float aspect = float(textureSize(basic_texture, 0).x) / float(textureSize(basic_texture, 0).y);
    v.y /= aspect;

    normalize(v);

    if(pixel.r > 0.0f) {
		bool b1 = is_between(startingAngleInsideVec, endingAngleInsideVec, v);

        if (b1) {
            outputColor = theColor * pixel.a;
        }
        else if (is_between(startingAngleVec, endingAngleVec, v)) {
            vec3 darkened_pixel = vec3(max(0.0f, theColor.r - 0.5), max(0.0f, theColor.g - 0.5), max(0.0f, theColor.b - 0.5));

            outputColor = vec4(darkened_pixel.r, darkened_pixel.g, darkened_pixel.b, 0.3);
        }
        else {
            discard;
        }
    }
    else {
        if(
            is_between(startingAngleVec, endingAngleVec, v) 
        ) {
            outputColor = theColor * pixel.a;
        }
        else {
            discard;
        }
    }
}