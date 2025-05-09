#version 150

uniform sampler2D particleTex;
uniform vec4 color;
out vec4 out_Color;

void main() {
    out_Color = texture(particleTex, gl_PointCoord) * color;
}
