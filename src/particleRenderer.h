#pragma once

#include "ofMain.h"

class particleRenderer {
public:
	particleRenderer();

	void load();
	void draw() const;
	void update(const std::vector<glm::vec3> &positions);

	ofShader shader;
	ofImage texture;
	ofVbo vbo;
};
