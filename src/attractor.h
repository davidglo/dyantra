#pragma once

#include "ofMain.h"

class attractor {
public:
    attractor(const ofPoint& center, float radius, float amplitude = 1.0f);

    void draw() const;
    bool isPointNear(const ofPoint& point, float tolerance) const;
    bool isPointNearEdge(const ofPoint& point, float tolerance) const;

    const ofPoint& getCenter() const;
    void setCenter(const ofPoint& center);

    float getRadius() const;
    void setRadius(float radius);

    float getAmplitude() const;
    void setAmplitude(float amplitude);

private:
    ofPoint center;
    float radius;
    float amplitude;
};
