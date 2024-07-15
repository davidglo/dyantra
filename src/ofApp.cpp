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
    string svgFile = "testTriangle.svg";
    svgSkeleton.loadSvg(svgFile);

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

    particleEnsemble.initialize(svgSkeleton.getEquidistantPoints());
    
    // GUI setup
    gui.setup();

    // Initialize new parameters
    gui.add(windowSize.set("Window Size", ""));
    gui.add(fpsDisplay.set("FPS", "")); // Add FPS display
    gui.add(numPointsDisplay.set("Number of Points", "")); // Add number of points display
    gui.add(showPotentialFieldGui.set("Show Potential Field", true));
    gui.add(showAttractorCircles.set("Show Attractor Circles", true)); // Add checkbox for attractor circles
    gui.add(showContourLines.set("Show Contour Lines", true)); // Add checkbox for contour lines
    gui.add(downscaleFactorGui.set("Downscale Factor", 3, 1, 10)); // Add slider for downscale factor
    gui.add(svgFileName.set("svgFile ", svgSkeleton.getFileName())); // Use getFileName method
    gui.add(svgCentroid.set("svgCentroid", ofVec2f(svgSkeleton.getSvgCentroid().x, svgSkeleton.getSvgCentroid().y)));
    gui.add(svgScale.set("svgScale", 1.0f)); // Initial scale is 1.0
    gui.add(potentialFieldColor.set("Potential Field Color", ofColor(255, 255, 255), ofColor(0, 0), ofColor(255, 255))); // Add color wheel
    gui.add(svgPointsColor.set("svg Points Color", ofColor(255, 255, 255), ofColor(0, 0), ofColor(255, 255))); // Add color wheel for SVG points
    
    // Setup and position the attractor information panel
    attractorGui.setup();
    attractorGui.setPosition(gui.getPosition().x + gui.getWidth() + 10, gui.getPosition().y); // Position to the right of the main panel
}

void ofApp::update() {
    
    // Synchronize GUI checkbox with showPotentialField variable
    showPotentialField = showPotentialFieldGui;
    
    // Update downscale factor
    if (downscaleFactor != downscaleFactorGui) {
        downscaleFactor = downscaleFactorGui;
        potentialField.allocate(ofGetWidth() / downscaleFactor, ofGetHeight() / downscaleFactor, OF_IMAGE_GRAYSCALE);
        potentialFieldUpdated = true;
        contourLinesUpdated = true;
    }
    
    if (potentialFieldUpdated) {
        attractorField.calculatePotentialField(potentialField, downscaleFactor, ofGetWidth() / downscaleFactor, ofGetHeight() / downscaleFactor);
        potentialFieldUpdated = false;
    }

    if (contourLinesUpdated) {
        updateContours();
        contourLinesUpdated = false;
    }

    // Update FPS display
    float fps = ofGetFrameRate();
    fpsDisplay = ofToString(fps, 2);
    
    // Update number of points display
    numPointsDisplay = ofToString(svgSkeleton.getEquidistantPoints().size());
    
    // Update window size display
    windowSize = ofToString(ofGetWidth()) + "x" + ofToString(ofGetHeight());

    // Update SVG centroid display
    svgCentroid = ofVec2f(svgSkeleton.getSvgCentroid().x, svgSkeleton.getSvgCentroid().y);
    
    // Update SVG scale display
    svgScale = svgSkeleton.getCumulativeScale();
    
    // Update GUI elements for attractors
    for (size_t i = 0; i < attractorField.getAttractors().size(); ++i) {
        updateAttractorGui(i, attractorField.getAttractors()[i]);
    }
    
    // In the future, we'll add the particle update code here.
}

void ofApp::draw() {
    ofBackground(0);  // Set background to black

    // Draw the potential field if the flag is set
    if (showPotentialField) {
        ofSetColor(potentialFieldColor->r, potentialFieldColor->g, potentialFieldColor->b); // Apply color
        potentialField.draw(0, 0, ofGetWidth(), ofGetHeight()); // Upscale when drawing
    }

    // Draw contour lines if the flag is set
    if (showContourLines) {
        ofSetColor(potentialFieldColor->r, potentialFieldColor->g, potentialFieldColor->b); // Apply color
        attractorField.drawContours();
    }

    ofSetColor(svgPointsColor);  // Set color to white for drawing

    // Draw the SVG skeleton
    svgSkeleton.draw();

    // Draw the particles
    particleEnsemble.draw();

    // Draw established attractors if the flag is set
    if (showAttractorCircles) {
        ofSetColor(potentialFieldColor->r, potentialFieldColor->g, potentialFieldColor->b); // Apply color
        attractorField.draw();
    }

    // Draw the temporary attractor being adjusted
    if (drawingAttractor) {
        ofSetColor(potentialFieldColor->r, potentialFieldColor->g, potentialFieldColor->b); // Apply color
        tempAttractor.draw();
    }
    
    // Draw GUI
    gui.draw();
    attractorGui.draw(); // Draw the attractor information panel
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
        tempAttractor = attractor(mousePos, 0);
        drawingAttractor = true;
    } else if (button == OF_MOUSE_BUTTON_RIGHT) {
        // Check if we are right-clicking near an attractor's center to delete it
        for (size_t i = 0; i < attractorField.getAttractors().size(); ++i) {
            if (attractorField.getAttractors()[i].isPointNear(mousePos, 10)) {
                attractorField.removeAttractorAt(i);
                removeAttractorGui(i);
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
        if (minDistance < 10) {
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
        particleEnsemble.translate(offset);
        initialMousePos.set(x, y);
    } else if (resizingSvg) {
        float initialDistance = initialMousePos.distance(svgSkeleton.getSvgCentroid());
        float currentDistance = ofPoint(x, y).distance(svgSkeleton.getSvgCentroid());
        float scaleFactor = currentDistance / initialDistance;
        svgSkeleton.resizeSvg(scaleFactor);
        svgScale = svgSkeleton.getCumulativeScale(); // Update the cumulative scale in the GUI
        particleEnsemble.resize(scaleFactor, svgSkeleton.getSvgCentroid());
        initialMousePos.set(x, y);
    }
    potentialFieldUpdated = true;
    contourLinesUpdated = true;
}

void ofApp::mouseReleased(int x, int y, int button) {
    if (drawingAttractor) {
        attractorField.addAttractor(tempAttractor);
        addAttractorGui(tempAttractor);
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
        showPotentialFieldGui = showPotentialField; // Sync the GUI checkbox
    }
}

void ofApp::addAttractorGui(const attractor& attractor) {
    ofParameterGroup group;
    group.setName("Attractor " + ofToString(attractorGroups.size()));

    ofParameter<ofVec2f> center;
    center.set("Center", ofVec2f(attractor.getCenter().x, attractor.getCenter().y));
    group.add(center);
    attractorCenters.push_back(center);

    ofParameter<float> radius;
    radius.set("Radius", attractor.getRadius(), 0.0f, 200.0f);
    group.add(radius);
    attractorRadii.push_back(radius);

    ofParameter<float> amplitude;
    amplitude.set("Amplitude", attractor.getAmplitude(), 0.0f, 10.0f);
    group.add(amplitude);
    attractorAmplitudes.push_back(amplitude);

    attractorGroups.push_back(group);
    attractorGui.add(group);
}

void ofApp::removeAttractorGui(int index) {
    if (index >= 0 && index < attractorGroups.size()) {
        attractorGroups.erase(attractorGroups.begin() + index);
        attractorCenters.erase(attractorCenters.begin() + index);
        attractorRadii.erase(attractorRadii.begin() + index);
        attractorAmplitudes.erase(attractorAmplitudes.begin() + index);

        // Clear the current GUI and re-add all elements
        attractorGui.clear();
        
        // Re-add all attractor groups
        for (size_t i = 0; i < attractorGroups.size(); ++i) {
            attractorGroups[i].setName("Attractor " + ofToString(i));
            attractorGui.add(attractorGroups[i]);
        }
    }
}

void ofApp::updateAttractorGui(int index, const attractor& attractor) {
    if (index >= 0 && index < attractorGroups.size()) {
        attractorCenters[index].set(ofVec2f(attractor.getCenter().x, attractor.getCenter().y));
        attractorRadii[index].set(attractor.getRadius());
        attractorAmplitudes[index].set(attractor.getAmplitude());
    }
}
