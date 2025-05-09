#version 150

uniform sampler2D particleTex;
uniform vec4 color;

out vec4 outputColor;

void main() {
    vec4 texColor = texture(particleTex, gl_PointCoord);
    outputColor = texColor * color;
}
