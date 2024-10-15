#version 410
// Vertex shader

layout(location = 0) in vec2 vertcoords_world_vs;
layout(location = 0) out vec2 vertcoords_vs;


uniform mat4 toWorldMatrix;

/**
 * Vertex shader. Coordinates are only those of a quad directly in front of the
 * camera.
 */
void main() {
    gl_Position = vec4(vertcoords_world_vs, 0.0, 1.0);
    vec4 pos =  toWorldMatrix * vec4(vertcoords_world_vs, 0.0, 1.0);
    vertcoords_vs = pos.xy;
}