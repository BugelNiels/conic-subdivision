#version 410
// Fragment shader

out vec4 fColor;

flat in vec2 startPos;
in vec2 vertPos;

uniform vec3 lineColor;

void main() {
    vec2  dir  = (vertPos -startPos);
    float dist = length(dir);
    if (sin(dist * 80) > 0) {
        discard;
    }
    fColor = vec4(lineColor, 1);
}
