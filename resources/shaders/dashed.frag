#version 410
// Fragment shader

out vec4 fColor;

flat in vec2 startPos;
in vec2 vertPos;

uniform bool dashEnabled;
uniform vec3 lineColor;

void main() {
    if(dashEnabled) {
        vec2  dir  = (vertPos -startPos);
        float dist = length(dir);
        // Discard fragment based on sine function to get a dashed line
        if (sin(dist * 80) > 0) {
            discard;
        }
    }
    fColor = vec4(lineColor, 1);
}
