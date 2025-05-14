#include "attractorField.h"
#include <cmath>
#include <algorithm>

void attractorField::addAttractor(const attractor& attractor) {
    attractors.push_back(attractor);
}

void attractorField::removeAttractorAt(int index) {
    if (index >= 0 && index < attractors.size()) {
        attractors.erase(attractors.begin() + index);
    }
}

void attractorField::draw() const {
    ofNoFill();
    for (const auto& attractor : attractors) {
        attractor.draw();
    }
}

void attractorField::drawContours() const {
//    ofSetColor(0, 0, 255); // Red color for contour lines
    ofFill();
    for (const auto& point : contourPoints) {
        ofDrawCircle(point.x, point.y, 0.8); // Smaller size for higher resolution
    }
}

void attractorField::updateContours(float downscaleFactor, int width, int height, const std::vector<glm::vec3>& equidistantPoints, float contourThreshold) {
    contourPoints.clear();
    
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

void attractorField::calculatePotentialField(ofImage& potentialField, float downscaleFactor, int width, int height, float contourThreshold) {
    float maxPotential = 0;
    float minPotential = FLT_MAX;

    std::vector<float> potentials(width * height);

    // Determine the flip state based on the sign of contourThreshold
    bool flipState = contourThreshold < 0;
    
    // Compute potential at each pixel
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float realX = x * downscaleFactor;
            float realY = y * downscaleFactor;
            float potential = computePotentialAtPoint(realX, realY);
            
            if (flipState) {
                potential = -potential;
            }
            
            potentials[y * width + x] = potential;
            maxPotential = std::max(maxPotential, potential);
            minPotential = std::min(minPotential, potential);
        }
    }

    // Normalize and update the potential field image, visualizing it using a logarithmic scale
    /*
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float potential = potentials[y * width + x];
            float logPotential = log(1 + potential - minPotential) / log(1 + maxPotential - minPotential);
            float normalizedPotential = ofMap(logPotential, 0, 1, 0, 255, true);
            potentialField.setColor(x, y, ofColor(normalizedPotential));
        }
    }
    */
    

    // Adjust max potential for 20% brightness at contourThreshold
    float adjustedMaxPotential = contourThreshold * 5;
    
    /*
    // Apply logarithmic scaling to the potential values
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float potential = potentials[y * width + x];
            float normalizedPotential = (potential - minPotential) / (adjustedMaxPotential - minPotential);

            // Apply logarithmic scaling
            float logScaledPotential = log(1 + normalizedPotential * 9) / log(10); // Logarithmic scale base 10
            float brightness = ofMap(logScaledPotential, 0, 1, 0, 255, true);
            
            potentialField.setColor(x, y, ofColor(brightness));
        }
    }
    */

    // Apply linear scaling to the potential values
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float potential = potentials[y * width + x];
//            float normalizedPotential = (potential - minPotential) / (adjustedMaxPotential - minPotential);
            float normalizedPotential = (potential) / (maxPotential);
            // Linear scaling
            float brightness = ofMap(normalizedPotential, 0, 1, 0, 255, true);
            float transparency = ofMap(normalizedPotential, 0, 1, 0, 255, true);

			//if (potential < 10) {
			//	brightness = 0;
			//	transparency = 0;
			//}

            potentialField.setColor(x, y, ofColor(brightness, transparency));
        }
    }
    
    potentialField.update();
}

float attractorField::computePotentialAtPoint(float x, float y) const {
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

void attractorField::computeForces(std::vector<ofPoint>& forces, const std::vector<glm::vec3>& positions, float amplitude, float sigma) {
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

void attractorField::setContourPoints(const std::vector<ofPoint>& points) {
    contourPoints = points;
}

const std::vector<ofPoint>& attractorField::getContourPoints() const {
    return contourPoints;
}

void attractorField::setAttractorCenter(int index, const ofPoint& center) {
    if (index >= 0 && index < attractors.size()) {
        attractors[index].setCenter(center);
    }
}

void attractorField::setAttractorRadius(int index, float radius) {
    if (index >= 0 && index < attractors.size()) {
        attractors[index].setRadius(radius);
    }
}

const std::vector<attractor>& attractorField::getAttractors() const {
    return attractors;
}

attractor& attractorField::getAttractor(int index) {
    return attractors[index];  // Assuming attractors is the vector holding the attractor objects
}

glm::vec3 attractorField::calculateForceOnParticle(const glm::vec3& particlePosition) const {
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
