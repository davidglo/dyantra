#pragma once

#include "ofMain.h"
#include "attractor.h"
#include "glm/vec3.hpp"

class attractorField {
public:
    void addAttractor(const attractor& attractor);
    void removeAttractorAt(int index);
    void draw() const;
    void drawContours() const;
    void updateContours(float downscaleFactor, int width, int height, const std::vector<glm::vec3>& equidistantPoints, float contourThreshold);
    void calculatePotentialField(ofImage& potentialField, float downscaleFactor, int width, int height, float contourThreshold);
    float computePotentialAtPoint(float x, float y) const;
    void computeForces(std::vector<ofPoint>& forces, const std::vector<glm::vec3>& positions, float amplitude, float sigma);

    void setContourPoints(const std::vector<ofPoint>& points);
    const std::vector<ofPoint>& getContourPoints() const;

    void setAttractorCenter(int index, const ofPoint& center);
    void setAttractorRadius(int index, float radius);

    const std::vector<attractor>& getAttractors() const;

    glm::vec3 calculateForceOnParticle(const glm::vec3& particlePosition) const; // New method

private:
    std::vector<attractor> attractors;
    std::vector<ofPoint> contourPoints;
};
