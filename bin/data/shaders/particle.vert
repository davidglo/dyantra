#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;
uniform float size;
out vec4 point;

void main() {
	mat4 mat = mat4(1.0);
	mat[0].x = 0.001;
	mat[1].y = 0.001;
	mat[2].z = 0.001;

	gl_Position = mat * position;
    gl_PointSize = size;
}
