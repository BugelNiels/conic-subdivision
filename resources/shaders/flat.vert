#version 410
// Vertex shader

layout(location = 0) in vec2 vertcoords_clip_vs;

uniform mat4 projectionMatrix;

void main() {
  gl_Position = projectionMatrix * vec4(vertcoords_clip_vs, 0.0, 1.0);
}
