precision mediump int;
precision mediump float;

uniform vec2 startingAngleVec;
uniform vec2 endingAngleVec;
uniform vec2 eye_frag_pos;

smooth in vec4 theColor;

out vec4 outputColor;

float crossproduct2d(vec2 a, vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

bool is_between(
    vec2 first, 
    vec2 second, 
    vec2 candidate
) {
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
    vec2 v = gl_FragCoord.xy - eye_frag_pos;
    normalize(v);

    if (is_between(startingAngleVec, endingAngleVec, v)) {
		discard;
	}
	else {
		outputColor = theColor;
	}
}