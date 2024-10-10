#version 410 core
layout (lines_adjacency) in;
layout (line_strip, max_vertices = 12) out;

const float norm_length = 0.1f;
uniform float curvatureScale;

uniform bool visualize_normals;
uniform bool visualize_curvature;
uniform sampler1D colorMap;

uniform vec3 normalColor;
uniform vec3 lineColor;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

in vec2 norm_vs[];

out vec4 line_color;

bool calcNormals = true;

// Just some color definitions
const vec3 curvOutlineCol = vec3(0.66, 0.44, 0.81);
const vec3 curvLineCol = vec3(0, 1, 0);
const vec3 curvLineCol2 = vec3(0.95);

// Emit line from point a to b
void emitLine(vec4 b, vec4 a) {
    gl_Position = projectionMatrix * viewMatrix * a;
    EmitVertex();

    gl_Position = projectionMatrix * viewMatrix * b;
    EmitVertex();
    EndPrimitive();
}

// Emit line from point a to b with a color gradient between col1 and col2
void emitLine(vec4 b, vec4 a, vec3 col1, vec3 col2) {
    line_color = vec4(col1, 1);
    gl_Position = projectionMatrix * viewMatrix * a;
    EmitVertex();

    line_color = vec4(col2, 1);
    gl_Position = projectionMatrix * viewMatrix * b;
    EmitVertex();
    EndPrimitive();
}

// Emit line from point a to b with the colormap sampled at position u (between 0 and 1)
void emitLine(vec4 b, vec4 a, float u) {
    line_color = vec4(texture(colorMap, u).xyz, 1);
    gl_Position = projectionMatrix * viewMatrix * a;
    EmitVertex();

    gl_Position = projectionMatrix * viewMatrix * b;
    EmitVertex();
    EndPrimitive();
}

// Emit a line for representing the normal from point p
void emitNormal(vec4 p, vec2 norm) {

    vec4 norm4 = vec4(norm, 0, 0);

    line_color = vec4(normalColor, 1);

    emitLine(p, p + norm_length * norm4);
}

// Calculates the curvature at point b in the line segment a-b-c
float calcCurvature(vec4 a, vec4 b, vec4 c) {
    vec2 ab = a.xy - b.xy;
    vec2 cb = c.xy - b.xy;

    float normAB = length(ab);
    float normCB = length(cb);

    float curvature = 2.0 * length(cross(vec3(ab, 0.0), vec3(cb, 0.0))) / (normAB * normCB * (normAB + normCB));
    return curvature;
}

// Calculates the normal of point b in the line segment a-b-c
vec2 calcNormal(vec4 a, vec4 b, vec4 c) {
    vec2 normal = normalize(b.xy - (b.xy + normalize(c.xy - b.xy) + normalize(a.xy - b.xy)));
    return normal;
}

// Calculates the position of a new point that sits along the normal based on the curvature at point b in the line segment a-b-c
vec4 calcToothTip(vec4 a, vec4 b, vec4 c, vec2 normal) {
    float curvatureVisualisationSize = 0.05 * curvatureScale;
    float curvature = calcCurvature(a, b, c);
    vec4 toothTip = b;
    if (length(normal) > 0) {
        vec2 offset = normal * curvature * curvatureVisualisationSize;
        toothTip.xy += offset;
    }
    return toothTip;
}

void main() {

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;
    vec4 p3 = gl_in[3].gl_Position;

    line_color = vec4(lineColor, 1);

    // Normal at p1
    vec2 n1 = calcNormals ? calcNormal(p0, p1, p2) : -norm_vs[1];
    n1 = normalize(n1);
    // Normal at p2
    vec2 n2 = calcNormals ? calcNormal(p1, p2, p3) : -norm_vs[2];
    n2 = normalize(n2);

    // Normal visualization
    if (visualize_normals) {
        emitNormal(p1, n1);
        emitNormal(p2, n2);
    }

    // Curvature visualization
    if (visualize_curvature) {
        line_color = vec4(curvLineCol, 1.0);
        vec4 firstToothTip = calcToothTip(p0, p1, p2, n1);
        vec4 secondToothTip = calcToothTip(p1, p2, p3, n2);
        emitLine(p2, secondToothTip, curvLineCol2, curvLineCol);
        line_color = vec4(curvOutlineCol, 1.0);
        emitLine(firstToothTip, secondToothTip);
    }

    // Visualize the curve itself
    line_color = vec4(lineColor, 1);
    emitLine(p1, p2);
}
