#include "ofApp.h"
#include <cmath>
#include <algorithm>

// Standalone helper function
bool isPointNear(const ofPoint& p1, const ofPoint& p2, float tolerance) {
    return p1.distance(p2) <= tolerance;
}

// New helper function to find the nearest vertex on the SVG paths
ofPoint ofApp::getNearestSvgVertex(const ofPoint& point, float& minDistance) {
    ofPoint nearestVertex;
    minDistance = FLT_MAX;

    for (const auto& vertex : svgSkeleton.getEquidistantPoints()) {
        float distance = point.distance(vertex);
        if (distance < minDistance) {
            minDistance = distance;
            nearestVertex = vertex;
        }
    }

    return nearestVertex;
}

void ofApp::setup() {
    svgSkeleton.loadSvg("testTriangle.svg");

    drawingAttractor = false;
    editingCenter = false;
    editingEdge = false;
    translatingSvg = false;
    resizingSvg = false;
    selectedAttractorIndex = -1;

    int width = ofGetWidth() / downscaleFactor;
    int height = ofGetHeight() / downscaleFactor;
    potentialField.allocate(width, height, OF_IMAGE_GRAYSCALE);
    potentialFieldUpdated = true;
    showPotentialField = true; // Initialize the flag to show the potential field
    contourLinesUpdated = true; // Initialize the flag to update contour lines
}

void ofApp::update() {
    if (potentialFieldUpdated) {
        attractorField.calculatePotentialField(potentialField, downscaleFactor, ofGetWidth() / downscaleFactor, ofGetHeight() / downscaleFactor);
        potentialFieldUpdated = false;
    }

    if (contourLinesUpdated) {
        updateContours();
        contourLinesUpdated = false;
    }
}

void ofApp::draw() {
    ofBackground(0);  // Set background to black

    // Draw the potential field if the flag is set
    if (showPotentialField) {
        potentialField.draw(0, 0, ofGetWidth(), ofGetHeight()); // Upscale when drawing
    }

    // Draw contour lines
    attractorField.drawContours();

    ofSetColor(255);  // Set color to white for drawing

    // Draw the SVG skeleton
    svgSkeleton.draw();

    // Draw established attractors
    attractorField.draw();

    // Draw the temporary attractor being adjusted
    if (drawingAttractor) {
        tempAttractor.draw();
    }

    // Draw the centroid of the SVG paths
    ofFill();
    ofDrawCircle(svgSkeleton.getSvgCentroid(), 5); // Draw a small circle at the centroid

    ofFill();  // Restore fill state
}

void ofApp::mousePressed(int x, int y, int button) {
    ofPoint mousePos(x, y);

    if (button == OF_MOUSE_BUTTON_LEFT) {
        // Check if we are clicking near an attractor's center
        for (size_t i = 0; i < attractorField.getAttractors().size(); ++i) {
            if (attractorField.getAttractors()[i].isPointNear(mousePos, 10)) {
                selectedAttractorIndex = i;
                editingCenter = true;
                return;
            }
        }

        // Check if we are clicking near an attractor's edge
        for (size_t i = 0; i < attractorField.getAttractors().size(); ++i) {
            if (attractorField.getAttractors()[i].isPointNearEdge(mousePos, 10)) {
                selectedAttractorIndex = i;
                editingEdge = true;
                return;
            }
        }

        // Check if we are clicking near the SVG centroid
        if (isPointNear(svgSkeleton.getSvgCentroid(), mousePos, 10)) {
            initialMousePos = mousePos;
            translatingSvg = true;
            return;
        }

        // Check if we are clicking on an SVG point to start resizing
        for (auto& point : svgSkeleton.getEquidistantPoints()) {
            if (isPointNear(point, mousePos, 10)) {
                initialMousePos = mousePos;
                initialSvgScale = 1.0f;
                resizingSvg = true;
                return;
            }
        }

        // Otherwise, we are drawing a new attractor
        tempAttractor = Attractor(mousePos, 0);
        drawingAttractor = true;
    } else if (button == OF_MOUSE_BUTTON_RIGHT) {
        // Check if we are right-clicking near an attractor's center to delete it
        for (size_t i = 0; i < attractorField.getAttractors().size(); ++i) {
            if (attractorField.getAttractors()[i].isPointNear(mousePos, 10)) {
                attractorField.removeAttractorAt(i);
                potentialFieldUpdated = true;
                contourLinesUpdated = true; // Mark contour lines for update
                return;
            }
        }
    }
}

void ofApp::mouseDragged(int x, int y, int button) {
    if (drawingAttractor) {
        float radius = ofDist(tempAttractor.getCenter().x, tempAttractor.getCenter().y, x, y);
        tempAttractor.setRadius(radius);
    } else if (editingCenter && selectedAttractorIndex >= 0) {
        ofPoint newCenter(x, y);
        float minDistance;
        ofPoint nearestVertex = getNearestSvgVertex(newCenter, minDistance);
        if (minDistance < 10) { // Snap if within 10 pixels
            newCenter = nearestVertex;
        }
        attractorField.setAttractorCenter(selectedAttractorIndex, newCenter);
    } else if (editingEdge && selectedAttractorIndex >= 0) {
        float radius = ofDist(attractorField.getAttractors()[selectedAttractorIndex].getCenter().x,
                              attractorField.getAttractors()[selectedAttractorIndex].getCenter().y, x, y);
        attractorField.setAttractorRadius(selectedAttractorIndex, radius);
    } else if (translatingSvg) {
        ofPoint offset(x - initialMousePos.x, y - initialMousePos.y);
        svgSkeleton.translateSvg(offset);
        initialMousePos.set(x, y);  // Update for continuous translation
    } else if (resizingSvg) {
        float initialDistance = initialMousePos.distance(svgSkeleton.getSvgCentroid());
        float currentDistance = ofPoint(x, y).distance(svgSkeleton.getSvgCentroid());
        float scaleFactor = currentDistance / initialDistance;
        svgSkeleton.resizeSvg(scaleFactor);
        initialMousePos.set(x, y);  // Update initialMousePos for continuous scaling
    }
    potentialFieldUpdated = true;
    contourLinesUpdated = true; // Mark contour lines for update
}

void ofApp::mouseReleased(int x, int y, int button) {
    if (drawingAttractor) {
        attractorField.addAttractor(tempAttractor);
        drawingAttractor = false;
    }
    editingCenter = false;
    editingEdge = false;
    translatingSvg = false;
    resizingSvg = false;
    selectedAttractorIndex = -1;
    potentialFieldUpdated = true;
    contourLinesUpdated = true; // Mark contour lines for update
}

void ofApp::updateContours() {
    int width = ofGetWidth() / downscaleFactor;
    int height = ofGetHeight() / downscaleFactor;
    attractorField.updateContours(downscaleFactor, width, height, svgSkeleton.getEquidistantPoints());
}

void ofApp::windowResized(int w, int h) {
    int width = w / downscaleFactor;
    int height = h / downscaleFactor;
    potentialField.allocate(width, height, OF_IMAGE_GRAYSCALE); // Reallocate the potential field image
    potentialFieldUpdated = true; // Mark the potential field as needing an update
    contourLinesUpdated = true; // Mark contour lines for update
}

void ofApp::keyPressed(int key) {
    if (key == 'p' || key == 'P') {
        showPotentialField = !showPotentialField; // Toggle the flag
    }
}

