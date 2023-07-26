#version 410
// Fragment shader

layout(location = 0) in vec2 vertcoords_vs;

out vec4 fColor;

uniform mat3 conic;

const double width = 0.01;

float grad(vec3 p, mat3 Q) {
  float xn = dot(Q[0], p);
  float yn = dot(Q[1], p);
  return length(vec2(xn, yn));
}

void main() {
  vec3 p = vec3(vertcoords_vs, 1.0);

  float conicValue = dot(p, conic * p);
  conicValue /= grad(p, conic);

  if (abs(conicValue) <= width) {
    fColor = vec4(1, 0, 0, 1);
  } else {
    discard;
  }
}