#version 410 core
layout (lines_adjacency) in;
layout (line_strip, max_vertices = 12) out;

const float norm_length = 0.1f;
uniform float curvatureScale;

uniform bool visualize_normals;
uniform bool visualize_curvature;
uniform bool stability_colors;
uniform sampler1D colorMap;

uniform vec3 normalColor;
uniform vec3 lineColor;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

in dvec2 coords_dvs[];
in vec2 norm_vs[];
in float stability_vs[];

out vec4 line_color;

bool calcNormals = false;

const vec3 curvOutlineCol = vec3(0.66, 0.44, 0.81);
const vec3 curvLineCol = vec3(0, 1, 0);
const vec3 curvLineCol2 = vec3(0.95);

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

void emitLine(vec4 b, vec4 a, vec3 col1, vec3 col2) {
    line_color = vec4(col1, 1);
    gl_Position = projectionMatrix * viewMatrix * a;
    EmitVertex();

    line_color = vec4(col2, 1);
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

void emitNormal(vec4 a, vec4 b, vec4 c, dvec2 norm) {

    vec4 norm4 = vec4(norm, 0, 0);

    line_color = vec4(normalColor, 1);

    emitLine(b, b + norm_length * norm4);
}

float calcCurvature(vec4 a, vec4 b, vec4 c) {
    vec2 ab = a.xy - b.xy;
    vec2 cb = c.xy - b.xy;

    float normAB = length(ab);
    float normCB = length(cb);

    float curvature = 2.0 * length(cross(vec3(ab, 0.0), vec3(cb, 0.0))) / (normAB * normCB * (normAB + normCB));
    return curvature;
}

double calcCurvatureDouble(dvec2 a, dvec2 b, dvec2 c) {
    dvec2 ab = a - b;
    dvec2 cb = c - b;

    double normAB = length(ab);
    double normCB = length(cb);

    double curvature = 2.0 * length(cross(dvec3(ab, 0.0), dvec3(cb, 0.0))) / (normAB * normCB * (normAB + normCB));
    return curvature;
}

vec2 calcNormal(vec4 a, vec4 b, vec4 c) {
    vec2 normal = normalize(b.xy - (b.xy + normalize(c.xy - b.xy) + normalize(a.xy - b.xy)));
    return normal;
}

dvec2 calcNormal(dvec2 a, dvec2 b, dvec2 c) {
    dvec2 normal = normalize(b - (b + normalize(c - b) + normalize(a - b)));
    return normal;
}

vec4 calcToothTip(vec4 a, vec4 b, vec4 c, dvec2 normal, double curvature) {
    double curvatureVisualisationSize = 0.05 * curvatureScale;
    vec4 toothTip = b;
    if (length(normal) > 0) {
        dvec2 offset = normal * curvature * curvatureVisualisationSize;
        toothTip.xy += vec2(offset);
    }
    return toothTip;
}

vec4 calcToothTip(vec4 a, vec4 b, vec4 c, vec2 normal) {
    float curvature = calcCurvature(a, b, c);
    return calcToothTip(a, b, c, normal, curvature);
}

void main() {

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;
    vec4 p3 = gl_in[3].gl_Position;


    line_color = vec4(lineColor, 1);
    dvec2 nc1 = calcNormal(coords_dvs[0], coords_dvs[1], coords_dvs[2]);
    dvec2 n1 = calcNormals ? nc1 : dvec2(-norm_vs[1]);
    if(dot(n1, nc1) < 0) {
        n1 *= -1;
    }

    n1 = normalize(n1);
    dvec2 nc2 = calcNormal(coords_dvs[1], coords_dvs[2], coords_dvs[3]);
    dvec2 n2 = calcNormals ? nc2 : dvec2(-norm_vs[2]);
    if(dot(n2, nc2) < 0) {
        n2 *= -1;
    }
    n2 = normalize(n2);
    if (visualize_normals) {
        emitNormal(p0, p1, p2, n1);
        emitNormal(p1, p2, p3, n2);
    }

    if (visualize_curvature) {
        line_color = vec4(curvLineCol, 1.0);
        double curv1 = calcCurvatureDouble(coords_dvs[0], coords_dvs[1], coords_dvs[2]);
        double curv2 = calcCurvatureDouble(coords_dvs[1], coords_dvs[2], coords_dvs[3]);

        vec4 firstToothTip = calcToothTip(p0, p1, p2, n1, curv1);
        vec4 secondToothTip = calcToothTip(p1, p2, p3, n2, curv2);
        emitLine(p2, secondToothTip, curvLineCol2, curvLineCol);
        line_color = vec4(curvOutlineCol, 1.0);
        emitLine(firstToothTip, secondToothTip);
    }

    // Emit the curve itself
    if (stability_colors) {
        emitLine(p1, p2, stability_vs[1]);
        line_color = vec4(lineColor, 1);
    } else {
        line_color = vec4(lineColor, 1);
        emitLine(p1, p2);
    }

}
