#version 410
// Vertex shader

layout(location = 0) in vec2 vertcoords_clip_vs;
layout(location = 1) in vec2 vertnorms_clip_vs;
layout(location = 2) in float stabilities;
layout(location = 3) in dvec2 coords_double;


out dvec2 coords_dvs;
out vec2 norm_vs;
out float stability_vs;

void main() {
  gl_Position = vec4(vertcoords_clip_vs, 0.0, 1.0);
  coords_dvs = coords_double;
  norm_vs = vertnorms_clip_vs;
  stability_vs = stabilities;
}
