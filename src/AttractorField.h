#pragma once

#include "ofMain.h"
#include "attractor.h"

class AttractorField {
public:
    void addAttractor(const Attractor& attractor);
    void removeAttractorAt(int index);
    void draw() const;
    void drawContours() const;
    void updateContours(float downscaleFactor, int width, int height, const std::vector<glm::vec3>& equidistantPoints);
    void calculatePotentialField(ofImage& potentialField, float downscaleFactor, int width, int height);
    float computePotentialAtPoint(float x, float y) const;
    void computeForces(std::vector<ofPoint>& forces, const std::vector<glm::vec3>& positions, float amplitude, float sigma);

    void setContourPoints(const std::vector<ofPoint>& points);
    const std::vector<ofPoint>& getContourPoints() const;

    void setAttractorCenter(int index, const ofPoint& center);
    void setAttractorRadius(int index, float radius);

    const std::vector<Attractor>& getAttractors() const;

private:
    std::vector<Attractor> attractors;
    std::vector<ofPoint> contourPoints;
};
