#version 410
// Vertex shader

layout(location = 0) in vec2 vertcoords_clip_vs;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

flat out vec2 startPos;
out vec2 vertPos;

void main() {
    vec4 pos = projectionMatrix * viewMatrix * vec4(vertcoords_clip_vs, 0.0, 1.0);
    gl_Position = pos;
    vertPos     = vertcoords_clip_vs;
    startPos    = vertcoords_clip_vs;
}
