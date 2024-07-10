#include "attractor.h"

Attractor::Attractor(const ofPoint& center, float radius)
    : center(center), radius(radius) {}

void Attractor::draw() const {
    // Draw center point in red
    ofFill();
    ofSetColor(255, 0, 0);
    ofDrawCircle(center, 3);
    ofNoFill();
    ofSetColor(255); // Set color back to white for the circle edge

    // Draw circle edge as polyline with small line segments
    ofPolyline polyline;
    int numSegments = 100;  // Number of segments to approximate the circle
    for (int i = 0; i <= numSegments; ++i) {
        float angle = ofMap(i, 0, numSegments, 0, TWO_PI);
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        polyline.addVertex(ofPoint(x, y));
    }
    polyline.close();
    polyline.draw();
}

bool Attractor::isPointNear(const ofPoint& point, float tolerance) const {
    return center.distance(point) <= tolerance;
}

bool Attractor::isPointNearEdge(const ofPoint& point, float tolerance) const {
    float distanceToEdge = abs(center.distance(point) - radius);
    return distanceToEdge <= tolerance;
}

void Attractor::setCenter(const ofPoint& center) {
    this->center = center;
}

const ofPoint& Attractor::getCenter() const {
    return center;
}

void Attractor::setRadius(float radius) {
    this->radius = radius;
}

float Attractor::getRadius() const {
    return radius;
}

