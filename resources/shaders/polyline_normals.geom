#version 410 core
layout (lines_adjacency) in;
layout (line_strip, max_vertices = 10) out;

const float norm_length = 0.1f;

uniform bool visualize_normals;

uniform vec3 normalColor;
uniform vec3 lineColor;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
in vec2 norm_vs[];
out vec4 line_color;

/**
 * Emits a line from point a to point b
 *
 * @param a Point a
 * @param b Point a
 */
void emitLine(vec4 b, vec4 a) {
  gl_Position = projectionMatrix * viewMatrix * a;
  EmitVertex();
  gl_Position = projectionMatrix * viewMatrix * b;
  EmitVertex();
  EndPrimitive();
}

void emitNormal(vec4 a, vec4 b, vec4 c, vec2 norm) {
  // calculates the normal at b and then emits a line from b along its normal

  vec4 norm4 = vec4(norm, 0, 0);

  line_color = vec4(normalColor, 1);

  emitLine(b, b + norm_length * norm4);
}

void main() {

  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;
  vec4 p3 = gl_in[3].gl_Position;


  // Emit the curve itself
  // we are drawing most lines multiple times, but this ensures the lines
  // from/to the first/last vertex are drawn properly
  line_color = vec4(lineColor, 1);
  emitLine(p0, p1);
  emitLine(p1, p2);
  emitLine(p2, p3);

  if(visualize_normals) {
    emitNormal(p0, p1, p2, norm_vs[1]);
    emitNormal(p1, p2, p3, norm_vs[2]);
  }

}
