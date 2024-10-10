#version 410
// Fragment shader

layout(location = 0) in vec2 vertcoords_vs;

out vec4 fColor;

// 3D matrix representing the conic
uniform mat3 conic;

const double width = 0.01;

// Gradient of the matrix at point p (length of the normal)
float gradient(vec3 p, mat3 Q) {
    float xn = dot(Q[0], p);
    float yn = dot(Q[1], p);
    return length(vec2(xn, yn));
}

void main() {
    vec3 p = vec3(vertcoords_vs, 1.0);

    // Evaluate the implicit equation  p^T*Q*T=0
    float conicValue = dot(p, conic * p);
    // Normalize
    conicValue /= gradient(p, conic);
    // According to the implicit equation, if conicValue is exactly 0, it lies on the conic
    // In this case, we only keep the points that lie sufficiently close to the conic line
    if (abs(conicValue) <= width) {
        fColor = vec4(1, 0, 0, 1);
    } else {
        discard;
    }
}