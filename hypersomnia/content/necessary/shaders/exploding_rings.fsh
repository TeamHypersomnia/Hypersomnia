#version 330
smooth in vec4 theColor;
in float inner_radius_sq;
in float outer_radius_sq;
in vec2 ring_center;
out vec4 outputColor;
layout(origin_upper_left) in vec4 gl_FragCoord;

void main() {
    vec2 dir = gl_FragCoord.xy - ring_center;
    float distance_sq = dot(dir, dir);

    if (distance_sq <= outer_radius_sq && distance_sq >= inner_radius_sq) {
        outputColor = theColor;
    }
    else {
        discard;
    }
}