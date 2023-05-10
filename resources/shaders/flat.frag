#version 410
// Fragment shader

out vec4 fColor;

uniform vec3 lineColor;

void main() {
  fColor = vec4(lineColor, 1);
}
