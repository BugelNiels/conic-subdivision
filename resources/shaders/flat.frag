#version 410
// Fragment shader

out vec4 fColor;

flat in vec3 startPos;
in vec3 vertPos;

uniform vec3 lineColor;

void main() {
  vec2  dir  = (vertPos.xy-startPos.xy);
  float dist = length(dir);
  if (sin(dist * 500) > 0) {
    discard;
  }
  fColor = vec4(lineColor, 1);
}
