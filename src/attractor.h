#pragma once

#include "ofMain.h"

class Attractor {
public:
    Attractor(const ofPoint& center, float radius);

    void draw() const;
    bool isPointNear(const ofPoint& point, float tolerance) const;
    bool isPointNearEdge(const ofPoint& point, float tolerance) const;

    void setCenter(const ofPoint& center);
    const ofPoint& getCenter() const;

    void setRadius(float radius);
    float getRadius() const;

private:
    ofPoint center;
    float radius;
};
