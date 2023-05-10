#version 410
// Vertex shader

layout(location = 0) in vec2 vertcoords_clip_vs;

uniform mat4 projectionMatrix;

flat out vec3 startPos;
out vec3 vertPos;

void main() {
    vec4 pos = projectionMatrix * vec4(vertcoords_clip_vs, 0.0, 1.0);
    gl_Position = pos;
    vertPos     = pos.xyz / pos.w;
    startPos    = vertPos;
}
