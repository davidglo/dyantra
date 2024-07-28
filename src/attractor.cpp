#include "attractor.h"

attractor::attractor(const ofPoint& center, float radius, float amplitude)
    : center(center), radius(radius), amplitude(amplitude) {
}

void attractor::draw() const {
    ofNoFill();
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
    ofDrawCircle(center, 1);
}

bool attractor::isPointNear(const ofPoint& point, float tolerance) const {
    return center.distance(point) <= tolerance;
}

bool attractor::isPointNearEdge(const ofPoint& point, float tolerance) const {
    return fabs(center.distance(point) - radius) <= tolerance;
}

const ofPoint& attractor::getCenter() const {
    return center;
}

void attractor::setCenter(const ofPoint& center) {
    this->center = center;
}

float attractor::getRadius() const {
    return radius;
}

void attractor::setRadius(float radius) {
    this->radius = radius;
}

float attractor::getAmplitude() const {
    return amplitude;
}

void attractor::setAmplitude(float amplitude) {
    this->amplitude = amplitude;
}

float attractor::get_coefficient() const {
    return amplitude / (radius * radius);
}

float attractor::get_exp_denominator() const {
    return -2.0f * radius * radius;
}
