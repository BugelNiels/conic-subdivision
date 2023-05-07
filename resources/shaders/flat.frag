#version 410
// Fragment shader

out vec4 fColor;

uniform vec3 line_color;

void main() {
  fColor = vec4(line_color, 1);
}
