#include "attractor.h"

Attractor::Attractor(const ofPoint& center, float radius, float amplitude)
    : center(center), radius(radius), amplitude(amplitude) {
}

void Attractor::draw() const {
    ofNoFill();
    ofSetColor(255); // White color for attractor edges
    int numSegments = 100; // Increase the number of segments for smoother circles
    ofBeginShape();
    for (int i = 0; i < numSegments; ++i) {
        float angle = ofMap(i, 0, numSegments, 0, TWO_PI);
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        ofVertex(x, y);
    }
    ofEndShape(true);

    ofFill();
    ofSetColor(255, 0, 0); // Red color for attractor centers
    ofDrawCircle(center, 2);
}

bool Attractor::isPointNear(const ofPoint& point, float tolerance) const {
    return center.distance(point) <= tolerance;
}

bool Attractor::isPointNearEdge(const ofPoint& point, float tolerance) const {
    return fabs(center.distance(point) - radius) <= tolerance;
}

const ofPoint& Attractor::getCenter() const {
    return center;
}

void Attractor::setCenter(const ofPoint& center) {
    this->center = center;
}

float Attractor::getRadius() const {
    return radius;
}

void Attractor::setRadius(float radius) {
    this->radius = radius;
}

float Attractor::getAmplitude() const {
    return amplitude;
}

void Attractor::setAmplitude(float amplitude) {
    this->amplitude = amplitude;
}
