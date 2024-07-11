#include "attractorField.h"
#include <cmath>
#include <algorithm>

void AttractorField::addAttractor(const Attractor& attractor) {
    attractors.push_back(attractor);
}

void AttractorField::removeAttractorAt(int index) {
    if (index >= 0 && index < attractors.size()) {
        attractors.erase(attractors.begin() + index);
    }
}

void AttractorField::draw() const {
    ofNoFill();
    for (const auto& attractor : attractors) {
        attractor.draw();
    }
}

void AttractorField::drawContours() const {
    ofSetColor(255, 0, 0); // Red color for contour lines
    for (const auto& point : contourPoints) {
        ofDrawCircle(point.x, point.y, 0.5); // Smaller size for higher resolution
    }
}

void AttractorField::updateContours(float downscaleFactor, int width, int height, const std::vector<glm::vec3>& equidistantPoints) {
    contourPoints.clear();
    float contourThreshold = 0.5; // Threshold for contour lines

    for (int y = 1; y < height; ++y) {
        for (int x = 1; x < width; ++x) {
            float potential = computePotentialAtPoint(x * downscaleFactor, y * downscaleFactor);
            float potentialRight = computePotentialAtPoint((x + 1) * downscaleFactor, y * downscaleFactor);
            float potentialDown = computePotentialAtPoint(x * downscaleFactor, (y + 1) * downscaleFactor);

            if ((potential >= contourThreshold && potentialRight < contourThreshold) ||
                (potential < contourThreshold && potentialRight >= contourThreshold) ||
                (potential >= contourThreshold && potentialDown < contourThreshold) ||
                (potential < contourThreshold && potentialDown >= contourThreshold)) {
                contourPoints.emplace_back(x * downscaleFactor, y * downscaleFactor);
            }
        }
    }
}

void AttractorField::calculatePotentialField(ofImage& potentialField, float downscaleFactor, int width, int height) {
    float maxPotential = 0;
    float minPotential = FLT_MAX;

    std::vector<float> potentials(width * height);

    // Compute potential at each pixel
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float realX = x * downscaleFactor;
            float realY = y * downscaleFactor;
            float potential = computePotentialAtPoint(realX, realY);
            potentials[y * width + x] = potential;

            maxPotential = std::max(maxPotential, potential);
            minPotential = std::min(minPotential, potential);
        }
    }

    // Normalize and update the potential field image using logarithmic scale
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float potential = potentials[y * width + x];
            float logPotential = log(1 + potential - minPotential) / log(1 + maxPotential - minPotential);
            float normalizedPotential = ofMap(logPotential, 0, 1, 0, 255, true);
            potentialField.setColor(x, y, ofColor(normalizedPotential));
        }
    }

    potentialField.update();
}

float AttractorField::computePotentialAtPoint(float x, float y) const {
    float totalPotential = 0;
    for (const auto& attractor : attractors) {
        float dx = x - attractor.getCenter().x;
        float dy = y - attractor.getCenter().y;
        float r = sqrt(dx * dx + dy * dy);
        float amplitude = attractor.getAmplitude();
        float sigma = attractor.getRadius(); // Using radius as sigma
        float potential = amplitude * exp(-0.5 * (r / sigma) * (r / sigma));
        totalPotential += potential;
    }
    return totalPotential;
}

void AttractorField::computeForces(std::vector<ofPoint>& forces, const std::vector<glm::vec3>& positions, float amplitude, float sigma) {
    forces.resize(positions.size(), ofPoint(0, 0));

    for (size_t i = 0; i < positions.size(); ++i) {
        for (const auto& attractor : attractors) {
            float dx = positions[i].x - attractor.getCenter().x;
            float dy = positions[i].y - attractor.getCenter().y;
            float exp_numerator = dx * dx + dy * dy;
            float exp_denominator = 2.0 * sigma * sigma;
            float coefficient = amplitude / (sigma * sigma);
            float prefactor = -1.0 * coefficient * exp(-exp_numerator / exp_denominator);

            forces[i].x += prefactor * dx;
            forces[i].y += prefactor * dy;
        }
    }
}

void AttractorField::setContourPoints(const std::vector<ofPoint>& points) {
    contourPoints = points;
}

const std::vector<ofPoint>& AttractorField::getContourPoints() const {
    return contourPoints;
}

void AttractorField::setAttractorCenter(int index, const ofPoint& center) {
    if (index >= 0 && index < attractors.size()) {
        attractors[index].setCenter(center);
    }
}

void AttractorField::setAttractorRadius(int index, float radius) {
    if (index >= 0 && index < attractors.size()) {
        attractors[index].setRadius(radius);
    }
}

const std::vector<Attractor>& AttractorField::getAttractors() const {
    return attractors;
}

glm::vec3 AttractorField::calculateForceOnParticle(const glm::vec3& particlePosition) const {
    glm::vec3 forceVector(0.0f, 0.0f, 0.0f);

    for (const auto& attractor : attractors) {
        float dx = particlePosition.x - attractor.getCenter().x;
        float dy = particlePosition.y - attractor.getCenter().y;
        float exp_numerator = dx * dx + dy * dy;
        float exp_denominator = 2.0 * attractor.getRadius() * attractor.getRadius(); // Using radius as sigma
        float coefficient = attractor.getAmplitude() / (attractor.getRadius() * attractor.getRadius());
        float prefactor = -1.0 * coefficient * exp(-exp_numerator / exp_denominator);

        forceVector.x += prefactor * dx;
        forceVector.y += prefactor * dy;
    }

    return forceVector;
}
