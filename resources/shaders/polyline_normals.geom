#version 410 core
layout (lines_adjacency) in;
layout (line_strip, max_vertices = 12) out;

const float norm_length = 0.1f;

uniform bool visualize_normals;
uniform bool visualize_curvature;
uniform bool stability_colors;
uniform sampler1D colorMap;

uniform vec3 normalColor;
uniform vec3 lineColor;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

in vec2 norm_vs[];
in float stability_vs[];

out vec4 line_color;

bool calcNormals = false;

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

void emitLine(vec4 b, vec4 a, float u) {
    line_color = vec4(texture(colorMap, u).xyz, 1);
    gl_Position = projectionMatrix * viewMatrix * a;
    EmitVertex();
    gl_Position = projectionMatrix * viewMatrix * b;
    EmitVertex();
    EndPrimitive();
}

float calcCurvature(vec4 a, vec4 b, vec4 c) {
    float dotProduct = dot(c - b, a - b);
    float angle = acos(dotProduct / (length(c - b) * length(a - b)));
    float curvature = (2 * sin(angle) / length(c - a));
    return curvature;
}

vec2 calcNormal(vec4 a, vec4 b, vec4 c) {
    vec2 normal = normalize(b.xy - (b.xy + normalize(c.xy - b.xy) + normalize(a.xy - b.xy)));
    return normal;
}

vec4 calcToothTip(vec4 a, vec4 b, vec4 c, vec2 normal) {
    float curvatureVisualisationSize = 0.05;
    float curvature = calcCurvature(a, b, c);
    vec4 toothTip = b;
    if (length(normal) > 0) {
        vec2 offset = normal * curvature * curvatureVisualisationSize;
        toothTip.xy += offset;
    }
    return toothTip;
}

vec4 calcToothTip(vec4 a, vec4 b, vec4 c) {
    return calcToothTip(a, b, c, calcNormal(a, b, c));
}

void emitNormal(vec4 a, vec4 b, vec4 c, vec2 norm) {

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
    if (stability_colors) {
        emitLine(p1, p2, stability_vs[1]);
        line_color = vec4(lineColor, 1);
    } else {
        line_color = vec4(lineColor, 1);
        emitLine(p1, p2);
    }

    line_color = vec4(lineColor, 1);
    vec2 n1 = calcNormals ? calcNormal(p0, p1, p2) : -norm_vs[1];
    vec2 n2 = calcNormals ? calcNormal(p1, p2, p3) : -norm_vs[2];
    if (visualize_normals) {
        emitNormal(p0, p1, p2, n1);
        emitNormal(p1, p2, p3, n2);
    }

    if (visualize_curvature) {
        line_color = vec4(0.3, 0.3, 0.3, 1.0);
        vec4 firstToothTip = calcToothTip(p0, p1, p2, n1);
        emitLine(p1, firstToothTip);
        vec4 secondToothTip = calcToothTip(p1, p2, p3, n2);
        emitLine(p2, secondToothTip);
        emitLine(firstToothTip, secondToothTip);
    }

}
