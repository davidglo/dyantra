#include "particleRenderer.h"

particleRenderer::particleRenderer() {
}

void particleRenderer::load() {
	ofDisableArbTex();
	bool textureLoaded = texture.load("textures/particle.png");
	ofEnableArbTex();

	if (!textureLoaded) {
		ofLogError("particleRenderer") << "Failed to load particle texture!";
	}

	bool shaderLoaded = shader.load("shaders/particle");
	if (!shaderLoaded) {
		ofLogError("particleRenderer") << "Failed to load particle shaders!";
	}
}

void particleRenderer::update(const std::vector<glm::vec3> &positions) {
	vbo.setVertexData(&positions[0], positions.size(), GL_DYNAMIC_DRAW);
}

void particleRenderer::draw() const {
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();

	texture.bind();
	shader.begin();

	shader.setUniform4f("color", ofGetStyle().color);
	shader.setUniform1f("size", ofGetStyle().pointSize);

	vbo.draw(GL_POINTS, 0, vbo.getNumVertices());

	shader.end();
	texture.unbind();

	ofDisablePointSprites();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}
