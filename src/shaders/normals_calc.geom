#version 410 core
layout (lines_adjacency) in;
layout (line_strip, max_vertices = 10) out;

const float norm_length = 0.1f;

uniform bool visualize_normals;

/**
 * Emits a line from point a to point b
 *
 * @param a Point a
 * @param b Point a
 */
void emitLine(vec4 b, vec4 a) {
  gl_Position = a;
  EmitVertex();
  gl_Position = b;
  EmitVertex();
  EndPrimitive();
}

/**
 * Calculates the normal at b in line segment a-b-c
 *
 * @param a point a
 * @param b point b
 * @param c point c
 * @return the normal at point b
 */
vec2 calcNormal(vec4 a, vec4 b, vec4 c) {
  if(a == b) {
    vec2 norm = vec2(b - c);
    norm.x *= -1;
    return normalize(norm.yx);
  }
  if( b == c) {
    vec2 norm = vec2(a - b);
    norm.x *= -1;
    return normalize(norm.yx);
  }
  vec2 normal = normalize(b.xy - (b.xy + normalize(c.xy - b.xy) + normalize(a.xy - b.xy)));
  return normal;
}

void emitNormal(vec4 a, vec4 b, vec4 c) {
  // calculates the normal at b and then emits a line from b along its normal

  vec2 norm = calcNormal(a,b,c);
  vec4 norm4 = vec4(norm, 0, 0);

  gl_Position = b;
  EmitVertex();
  gl_Position = b + norm_length * norm4;
  EmitVertex();
  EndPrimitive();
}

void main() {

  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;
  vec4 p3 = gl_in[3].gl_Position;


  // Emit the curve itself
  // we are drawing most lines multiple times, but this ensures the lines
  // from/to the first/last vertex are drawn properly
  emitLine(p0, p1);
  emitLine(p1, p2);
  emitLine(p2, p3);

  if(visualize_normals) {
    emitNormal(p0, p1, p2);
    emitNormal(p1, p2, p3);
  }

}
