#version 300 es
precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in float inner_radius_sq;
in float outer_radius_sq;
in vec2 ring_center;
out vec4 outputColor;

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