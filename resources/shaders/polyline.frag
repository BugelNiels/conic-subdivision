#version 410
// Fragment shader

in vec4 line_color;
out vec4 fColor;

void main() {
  fColor = line_color;
}
