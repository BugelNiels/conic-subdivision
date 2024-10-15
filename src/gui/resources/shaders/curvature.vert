#version 410
// Vertex shader

layout(location = 0) in vec2 vertcoords_clip_vs;
layout(location = 1) in vec2 vertnorms_clip_vs;

out vec2 norm_vs;

void main() {
    gl_Position = vec4(vertcoords_clip_vs, 0.0, 1.0);
    norm_vs = vertnorms_clip_vs;
}
