#version 410 core
layout (lines_adjacency) in;
layout (line_strip, max_vertices = 12) out;

const float norm_length = 0.1f;
uniform float curvatureScale;
uniform int curvatureType;

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
void emitNormal(vec4 p, dvec2 norm) {
    vec4 norm4 = vec4(norm, 0.0, 0.0);
    line_color = vec4(normalColor, 1);
    emitLine(p, p + norm_length * norm4);
}

// The curvature is based on the radius of the circle defined by the 3 given points
double calcCurvatureCircular(dvec2 a, dvec2 b, dvec2 c) {
    dvec2 ab = a - b;
    dvec2 cb = c - b;
    dvec2 ac = a - c;

    double denom = (dot(ab, ab) * dot(cb, cb) * dot(ac, ac));

    // Avoid division by zero
    if (denom == 0.0) return 0.0;

    dvec3 t = cross(dvec3(ab, 0.0), dvec3(cb, 0.0));
    return sqrt(dot(t, t) / denom);
}

// The following three methods are based on the formulas provided in:
// https://www.cs.utexas.edu/~evouga/uploads/4/5/6/8/45689883/notes1.pdf

// The curvature is based on the winding number theorem
double calcCurvatureWinding(dvec2 a, dvec2 b, dvec2 c) {
    dvec2 e1 = b - a;
    dvec2 e_1 = c - a;
    
    double cross = e_1.x * e1.y - e_1.y * e1.x;
    double dot = e_1.x * e1.x + e_1.y * e1.y;
    // Note that there is no float64 support for trigonometric operations
    float v = atan(float(cross / dot));

    return 2.0 * v / (length(e_1) + length(e1));
}

// The curvature is based on the gradient arc length
double calcCurvatureGradientArcLength(dvec2 a, dvec2 b, dvec2 c) {
    dvec2 e1 = b - a;
    dvec2 e_1 = c - a;
    
    double cross = e_1.x * e1.y - e_1.y * e1.x;
    double dot = e_1.x * e1.x + e_1.y * e1.y;
    float v = atan(float(cross / dot));
    
    return 4.0 * sin(v / 2.0) / (length(e_1) + length(e1));
}

// The curvature is based on the gradient arc length
double calcCurvatureAreaInflation(dvec2 a, dvec2 b, dvec2 c) {
    dvec2 e1 = b - a;
    dvec2 e_1 = c - a;
    
    double cross = e_1.x * e1.y - e_1.y * e1.x;
    double dot = e_1.x * e1.x + e_1.y * e1.y;
    float v = atan(float(cross / dot));
    
    return 4.0 * tan(v / 2.0) / (length(e_1) + length(e1));
}

// Calculates the curvature at point b in the line segment a-b-c with double precision based on the provided curvature calculation
double calcCurvatureDouble(dvec2 a, dvec2 b, dvec2 c, int curvType) {
    if(curvType == 0) {
        return calcCurvatureCircular(a, b, c);
    } else if(curvType == 1) {
        return calcCurvatureWinding(a, b, c);
    } else if(curvType == 2) {
        return calcCurvatureGradientArcLength(a, b, c);
    } else if(curvType == 3) {
        return calcCurvatureAreaInflation(a, b, c);
    }
}

// Calculates the (unnormalized) normal of point b in the line segment a-b-c with double precision
dvec2 calcNormal(dvec2 a, dvec2 b, dvec2 c) {
    dvec2 normal;

    if (a == b) {
        normal = c - b;
        normal.x *= -1.0;
        return normalize(dvec2(normal.y, normal.x));
    }

    if (b == c) {
        normal = b - a;
        normal.x *= -1.0;
        return normalize(dvec2(normal.y, normal.x));
    }

    dvec2 t1 = a - b;
    t1 = normalize(dvec2(-t1.y, t1.x));

    dvec2 t2 = b - c;
    t2 = normalize(dvec2(-t2.y, t2.x));
    normal = normalize(t1 + t2);

    // Ensure correct orientation; normal is always pointing outwards
    dvec2 ab = a - b;
    dvec2 cb = c - b;
    double crossP = ab.x * cb.y - ab.y * cb.x;
    return crossP > 0.0 ? -normal : normal;
}

// Calculates the position of a new point that sits along the normal based on the curvature
vec4 calcToothTip(vec4 p, dvec2 normal, double curvature) {
    double curvatureVisualisationSize = 0.1 * curvatureScale;
    vec4 toothTip = p;
    if (dot(normal, normal) > 0) {
        dvec2 offset = normal * curvature * curvatureVisualisationSize;
        toothTip.xy += vec2(offset);
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
    // Everything but the circular curvature calculation already takes care of ensuring the curvature points in the right direction
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
        double curv1 = abs(calcCurvatureDouble(coords_dvs[0], coords_dvs[1], coords_dvs[2], curvatureType));
        double curv2 = abs(calcCurvatureDouble(coords_dvs[1], coords_dvs[2], coords_dvs[3], curvatureType));

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
