#version 410
// geometry shader

layout(lines_adjacency) in;
layout(line_strip, max_vertices = 10) out;

// Outputs separate colors for the line and the curvature comb
out vec4 line_color;

// calculates the curvature at b in line segment a-b-c
float calcCurvature(vec4 a, vec4 b, vec4 c) {
  float dotProduct = dot(c - b, a - b);
  float angle = acos(dotProduct / (length(c - b) * length(a - b)));
  float curvature = (2 * sin(angle) / length(c - a));
  return curvature;
}

// Calculates the normal at b in line segment a-b-c
vec2 calcNormal(vec4 a, vec4 b, vec4 c) {
  vec2 normal = normalize(
      b.xy - (b.xy + normalize(c.xy - b.xy) + normalize(a.xy - b.xy)));
  return normal;
}

// Emits a line from pointA to pointB
void emitLine(vec4 pointA, vec4 pointB) {
  gl_Position = pointA;
  EmitVertex();
  gl_Position = pointB;
  EmitVertex();
  EndPrimitive();
}

// Calculates the position of the tooth tip at point b in the line segment a-b-c
vec4 calcToothTip(vec4 a, vec4 b, vec4 c) {
  // Determines the magnitude of the visualisation
  float curvatureVisualisationSize = 0.05;
  // Calculate curvature for the tooth
  float curvature = calcCurvature(a, b, c);
  // Derive normal for the tooth
  vec2 normal = calcNormal(a, b, c);
  vec4 toothTip = b;
  // fix combs not drawing for first and last vertices
  if (length(normal) > 0) {
    vec2 offset = normal * curvature * curvatureVisualisationSize;
    toothTip.xy += offset;
  }
  return toothTip;
}

// Constructs the comb teeth and connecting lines from the curve points. While
// drawing two teeth for each line means that each tooth will be drawn twice, it
// does make sure that we always have both the first and the last tooth drawn,
// and that all connecting lines go between two teeth instead of either the
// first or the last one going between a tooth and the place where a tooth would
// have been.
void main() {
  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;
  vec4 p3 = gl_in[3].gl_Position;

  // Emit the curve itself
  line_color = vec4(0.8, 0.8, 0.8, 1.0);
  emitLine(p1, p2);

  // Calculate and draw tooth tips
  vec4 firstToothTip = calcToothTip(p0, p1, p2);
  emitLine(p1, firstToothTip);

  vec4 secondToothTip = calcToothTip(p1, p2, p3);
  emitLine(p2, secondToothTip);

  // emit the connecting line
  emitLine(firstToothTip, secondToothTip);
}
