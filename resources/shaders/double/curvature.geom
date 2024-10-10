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

in dvec2 coords_dvs[];
in vec2 norm_vs[];

out vec4 line_color;

bool calcNormals = false;

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
void emitNormal(vec4 p, dvec2 norm) {
    vec4 norm4 = vec4(norm, 0.0, 0.0);
    line_color = vec4(normalColor, 1);
    emitLine(p, p + norm_length * norm4);
}

// Calculates the curvature at point b in the line segment a-b-c with double precision
// The curvature is based on the curvature of the circle defined by the 3 given points
double calcCurvatureDouble(dvec2 a, dvec2 b, dvec2 c) {
    dvec2 ab = a - b;
    dvec2 cb = c - b;
    dvec2 ac = a - c;

    double denom = (dot(ab, ab) * dot(cb, cb) * dot(ac, ac));

    // Avoid division by zero
    if (denom == 0.0) return 0.0;

    // return 2.0 * length(cross(dvec3(ab, 0.0), dvec3(cb, 0.0))) / (normAB * normCB * (normAB + normCB));

    dvec3 t = cross(dvec3(ab, 0.0), dvec3(cb, 0.0));
    return 2 * sqrt(dot(t, t) / denom);
}

// Calculates the (unnormalized) normal of point b in the line segment a-b-c with double precision
dvec2 calcNormal(dvec2 a, dvec2 b, dvec2 c) {
    dvec2 ab = a - b;
    dvec2 cb = c - b;
    return ab + cb;
}

// Calculates the position of a new point that sits along the normal based on the curvature
vec4 calcToothTip(vec4 p, dvec2 normal, double curvature) {
    double curvatureVisualisationSize = 0.05 * curvatureScale;
    vec4 toothTip = p;
    if (dot(normal, normal) > 0) {
        dvec2 offset = normal * curvature * curvatureVisualisationSize;
        toothTip.xy -= vec2(offset);
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
    dvec2 nc1 = calcNormal(coords_dvs[0], coords_dvs[1], coords_dvs[2]);
    dvec2 n1 = calcNormals ? nc1 : dvec2(-norm_vs[1]);
    if (dot(n1, nc1) < 0) {
        n1 *= -1;
    }
    n1 = normalize(n1);

    // Normal at p2
    dvec2 nc2 = calcNormal(coords_dvs[1], coords_dvs[2], coords_dvs[3]);
    dvec2 n2 = calcNormals ? nc2 : dvec2(-norm_vs[2]);
    if (dot(n2, nc2) < 0) {
        n2 *= -1;
    }
    n2 = normalize(n2);

    // Normal visualization
    if (visualize_normals) {
        emitNormal(p1, n1);
        emitNormal(p2, n2);
    }

    // Curvature visualization
    if (visualize_curvature) {
        line_color = vec4(curvLineCol, 1.0);
        double curv1 = calcCurvatureDouble(coords_dvs[0], coords_dvs[1], coords_dvs[2]);
        double curv2 = calcCurvatureDouble(coords_dvs[1], coords_dvs[2], coords_dvs[3]);

        vec4 firstToothTip = calcToothTip(p1, n1, curv1);
        vec4 secondToothTip = calcToothTip(p2, n2, curv2);
        emitLine(p2, secondToothTip, curvLineCol2, curvLineCol);
        line_color = vec4(curvOutlineCol, 1.0);
        emitLine(firstToothTip, secondToothTip);
    }

    // Visualize the curve itself
    line_color = vec4(lineColor, 1);
    emitLine(p1, p2);
}
