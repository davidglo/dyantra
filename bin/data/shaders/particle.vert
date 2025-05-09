#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;
uniform float size;

void main() {
	gl_Position = modelViewProjectionMatrix * position;
    gl_PointSize = size;
}
