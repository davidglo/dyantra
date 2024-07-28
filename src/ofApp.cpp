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
    
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    
//    string svgFile = "taraYantra.svg";
    string svgFile = "taraYantra2.svg";
    //string svgFile = "circle.svg";
    //string svgFile = "line.svg";
    //string svgFile = "2lines.svg";
    //string svgFile = "triangle.svg";
    
    svgSkeleton.loadSvg(svgFile);

    numPoints = 1000; // Set the desired number of points
    timestep = 0.003;
    
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
    showGrid = true;
    
    // GUI setup
    gui.setup();

    svgSkeleton.generateEquidistantPoints(numPoints); // Call with the data member
    particleEnsemble.initialize(svgSkeleton.getEquidistantPoints()); // initialize the particleEnsemble
    
    elapsedTimesteps = 0;  // Initialize elapsed timesteps counter
    timeForward = true;  // Initialize time direction to forward
    
    // Initialize new parameters
    gui.add(windowSize.set("Window Size", ""));
    gui.add(fpsDisplay.set("FPS", "")); // Add FPS display
    gui.add(playPauseStatus.set("Play/Pause", "Pause"));  // Initialize as "Pause"
    gui.add(numPointsDisplay.set("Number of Points", "")); // Add number of points display
    
    // New input field for number of points
    gui.add(numPointsInput.setup("Edit points:", numPoints, 2, 50000));
    
    gui.add(timeDirectionDisplay.set("Time Direction", "FORWARD"));        // Add time direction to the GUI

    // Add the timestep input field
    gui.add(timestepLabel_1.set("timestep [0.0002-0.01]", ""));
    gui.add(timestepLabel_2.set("         [default=0.003]", ""));
    gui.add(timestepInput.setup("Edit timestep", timestep, 0.0002, 0.01));
    timestepInput.addListener(this, &ofApp::onTimestepChanged);
    
    // Add elapsed timesteps to the GUI
    gui.add(elapsedTimestepsDisplay.set("Elapsed Steps", ofToString(elapsedTimesteps)));
    
    gui.add(showPotentialFieldGui.set("Show Potential Field", true));
    gui.add(showAttractorCircles.set("Show Attractor Circles", true)); // Add checkbox for attractor circles
    gui.add(showContourLines.set("Show Contour Lines", true)); // Add checkbox for contour lines
    gui.add(contourThresholdSlider.setup("Contour Threshold", 10000, 0.0, 50000.0));  // Initialize the contour threshold slider
    gui.add(downscaleFactorGui.set("Downscale Factor", 3, 1, 10)); // Add slider for downscale factor

    gui.add(showGrid.set("Show Grid", true));  // Add the checkbox for the grid
    
    // svg related input
    gui.add(svgFileName.set("svgFile ", svgSkeleton.getFileName())); // Use getFileName method
    gui.add(showSvgPoints.setup("Show SVG Points", true));  // Initialize the new toggle
    gui.add(svgCentroid.set("svgCentroid", ofVec2f(svgSkeleton.getSvgCentroid().x, svgSkeleton.getSvgCentroid().y)));
    gui.add(svgScale.set("svgScale", 1.0f)); // Initial scale is 1.0
        
    ofColor initialPotentialFieldColor(128, 128, 128); // 50% intensity of white color
    gui.add(potentialFieldColor.set("Potential Field Color", initialPotentialFieldColor, ofColor(0, 0), ofColor(255, 255))); // Add color wheel
    ofColor initialSvgPointsColor(178, 178, 178); // 70% intensity of white color
    gui.add(svgPointsColor.set("SVG Points Color", initialSvgPointsColor, ofColor(0, 0), ofColor(255, 255))); // Add color wheel for SVG points
    
    contourThresholdSlider.addListener(this, &ofApp::onContourThresholdChanged);  // Add listener to the slider
    
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
    
    // Check if the number of points has changed
    if (!isPlaying && numPoints != numPointsInput) {
        numPoints = numPointsInput;
        svgSkeleton.generateEquidistantPoints(numPoints);
        svgSkeleton.updateSvgCentroid();
        particleEnsemble.initialize(svgSkeleton.getEquidistantPoints());
        potentialFieldUpdated = true;
        contourLinesUpdated = true;
    }
    
    if (potentialFieldUpdated) {
        attractorField.calculatePotentialField(potentialField, downscaleFactor, ofGetWidth() / downscaleFactor, ofGetHeight() / downscaleFactor, contourThresholdSlider);
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

    // Update play/pause status
    playPauseStatus = isPlaying ? "Play" : "Pause";
    
    // Update SVG centroid display
    svgCentroid = ofVec2f(svgSkeleton.getSvgCentroid().x, svgSkeleton.getSvgCentroid().y);
    
    // Update SVG scale display
    svgScale = svgSkeleton.getCumulativeScale();

    // Update GUI elements for attractors
    for (size_t i = 0; i < attractorField.getAttractors().size(); ++i) {
        updateAttractorGui(i, attractorField.getAttractors()[i]);
    }
        
    // Handle particle motion if playing
    if (isPlaying) {
        float dt = timestep * (timeForward ? 1 : -1);
//        particleEnsemble.radial_update(dt, angularVelocity, glm::vec3(svgSkeleton.getSvgCentroid().x, svgSkeleton.getSvgCentroid().y, 0));
        particleEnsemble.vv_propagatePositionsVelocities(attractorField.getAttractors(), dt);
        
        if (timeForward) {
            elapsedTimesteps++;  // Increment elapsed timesteps
        } else {
            elapsedTimesteps--;  // Decrement elapsed timesteps
        }
        elapsedTimestepsDisplay = ofToString(elapsedTimesteps);  // Update GUI display
    }
}

void ofApp::draw() {
    ofBackground(0);  // Set background to black
    
    // Draw the potential field if the flag is set
    if (showPotentialField) {
        ofSetColor(potentialFieldColor->r, potentialFieldColor->g, potentialFieldColor->b); // Apply color
        potentialField.draw(0, 0, ofGetWidth(), ofGetHeight()); // Upscale when drawing
    }

    if (showGrid) {
        drawGrid();  // Draw grid
    }
    
    // Draw contour lines if the flag is set
    if (showContourLines) {
        ofSetColor(potentialFieldColor->r, potentialFieldColor->g, potentialFieldColor->b); // Apply color
        attractorField.drawContours();
    }

    ofSetColor(svgPointsColor);  // Set color to white for drawing

    // Draw the SVG skeleton if the toggle is on
    if (showSvgPoints) {
        svgSkeleton.draw();
    }

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
        tempAttractor = attractor(mousePos, 20);
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
         radius = std::max(radius, 5.0f);  // Ensure minimum radius of 2
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
         ofPoint attractorCenter = attractorField.getAttractors()[selectedAttractorIndex].getCenter();
         ofPoint mousePos(x, y);
         float radius = ofDist(attractorCenter.x, attractorCenter.y, mousePos.x, mousePos.y);

         float minDistance;
         ofPoint nearestVertex = getNearestSvgVertex(mousePos, minDistance);
         if (minDistance < 10) { // Snap to the nearest vertex if within 10 pixels
             radius = ofDist(attractorCenter.x, attractorCenter.y, nearestVertex.x, nearestVertex.y);
         }

         radius = std::max(radius, 5.0f);  // Ensure minimum radius of 5
         attractorField.setAttractorRadius(selectedAttractorIndex, radius);
         attractorRadiusInputs[selectedAttractorIndex]->getParameter().cast<float>().set(radius); // Synchronize the GUI radius input fields

     } else if (translatingSvg) {
         ofPoint offset(x - initialMousePos.x, y - initialMousePos.y);
         svgSkeleton.translateSvg(offset);
         particleEnsemble.translate(offset);
         initialMousePos.set(x, y);
         svgSkeleton.updateSvgCentroid();
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
    float contourThreshold = contourThresholdSlider;  // Use the slider value
    attractorField.updateContours(downscaleFactor, width, height, svgSkeleton.getEquidistantPoints(), contourThreshold);
    attractorField.calculatePotentialField(potentialField, downscaleFactor, width, height, contourThreshold);  // Update potential field with contourThreshold
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
    if (key == ' ') {
        isPlaying = !isPlaying;
        playPauseStatus = isPlaying ? "Play" : "Pause";  // Update play/pause status
        if (isPlaying) {
            showSvgPoints = false;  // Hide SVG points when starting
        }
    }
    if (key == 'b' || key == 'B') {
        timeForward = !timeForward;
        timeDirectionDisplay = timeForward ? "FORWARD" : "BACKWARD";  // Update time direction display
    }
    if (key == 'r' || key == 'R') {
        resetSimulation();
    }
    if (key == 'c' || key == 'C') {
        showContourLines = !showContourLines; // Toggle the flag
        showContourLines.set(showContourLines); // Sync the GUI checkbox
    }
    if (key == 'g' || key == 'G') {
        showGrid = !showGrid;  // Toggle the flag
        showGrid.set(showGrid);
    }
}

void ofApp::addAttractorGui(const attractor& attractor) {
    ofParameterGroup group;
    group.setName("Attractor " + ofToString(attractorGroups.size()));

    ofParameter<ofVec2f> center;
    center.set("Center", ofVec2f(attractor.getCenter().x, attractor.getCenter().y));
    group.add(center);
    attractorCenters.push_back(center);

    attractorGroups.push_back(group);
    attractorGui.add(group);

    auto radiusInput = std::make_shared<ofxFloatField>();
    radiusInput->setup("Edit Radius:", attractor.getRadius(), 5.0f, 4000.0f);
    radiusInput->addListener(this, &ofApp::attractorRadiusChanged);
    attractorRadiusInputs.push_back(radiusInput);
    radiusInputToAttractorIndex[radiusInput.get()] = attractorGroups.size() - 1;
    attractorGui.add(radiusInput.get());
    
    auto amplitudeInput = std::make_shared<ofxFloatField>();
    amplitudeInput->setup("Edit Amplitude:", attractor.getAmplitude(), 0.0f, 100000.0f);
    amplitudeInput->addListener(this, &ofApp::attractorAmplitudeChanged);
    attractorAmplitudeInputs.push_back(amplitudeInput);
    amplitudeInputToAttractorIndex[amplitudeInput.get()] = attractorGroups.size() - 1;
    attractorGui.add(amplitudeInput.get());
    
}

void ofApp::removeAttractorGui(int index) {
    if (index >= 0 && index < attractorGroups.size()) {
        attractorGroups.erase(attractorGroups.begin() + index);
        attractorCenters.erase(attractorCenters.begin() + index);

        radiusInputToAttractorIndex.erase(attractorRadiusInputs[index].get());
        attractorRadiusInputs.erase(attractorRadiusInputs.begin() + index);

        amplitudeInputToAttractorIndex.erase(attractorAmplitudeInputs[index].get());
        attractorAmplitudeInputs.erase(attractorAmplitudeInputs.begin() + index);
        
        // Clear the current GUI and re-add all elements
        attractorGui.clear();
        
        // Re-add all attractor groups
        for (size_t i = 0; i < attractorGroups.size(); ++i) {
            attractorGroups[i].setName("Attractor " + ofToString(i));
            attractorGui.add(attractorGroups[i]);
            attractorGui.add(attractorRadiusInputs[i].get());
            attractorGui.add(attractorAmplitudeInputs[i].get());
            radiusInputToAttractorIndex[attractorRadiusInputs[i].get()] = i;
            amplitudeInputToAttractorIndex[attractorAmplitudeInputs[i].get()] = i;
        }
    }
}

void ofApp::updateAttractorGui(int index, const attractor& attractor) {
    if (index >= 0 && index < attractorGroups.size()) {
        attractorCenters[index].set(ofVec2f(attractor.getCenter().x, attractor.getCenter().y));
    }
}

void ofApp::onContourThresholdChanged(float & value) {
    contourLinesUpdated = true;
}

void ofApp::attractorRadiusChanged(float & radius) {
    radius = std::max(radius, 5.0f);  // Ensure minimum radius of 5
    for (size_t i = 0; i < attractorRadiusInputs.size(); ++i) {
        float currentValueFromInputs = attractorRadiusInputs[i]->getParameter().cast<float>();
            attractorField.getAttractor(i).setRadius(currentValueFromInputs);  // Update the attractor's radius
            potentialFieldUpdated = true;   // Synchronize the GUI field
            contourLinesUpdated = true;
    }
}

void ofApp::attractorAmplitudeChanged(float & amplitude) {
    for (size_t i = 0; i < attractorAmplitudeInputs.size(); ++i) {
        float currentValueFromInputs = attractorAmplitudeInputs[i]->getParameter().cast<float>();
        attractorField.getAttractor(i).setAmplitude(currentValueFromInputs);  // Update the attractor's amplitude
        potentialFieldUpdated = true;
        contourLinesUpdated = true;
    }
}

void ofApp::resetSimulation() {
    // Pause the simulation
    isPlaying = false;
    playPauseStatus = "Pause";  // Update play/pause status
    showSvgPoints = true;  // Show SVG points when resetting

    // Reset particle positions to original positions along the SVG skeleton
    particleEnsemble.reinitialize(svgSkeleton.getEquidistantPoints());

    // Reset elapsed timesteps
    elapsedTimesteps = 0;
    elapsedTimestepsDisplay = ofToString(elapsedTimesteps);  // Update GUI display
}

void ofApp::drawGrid() {
    ofSetColor(64); // Set grid color to white
    ofSetLineWidth(1); // Set the line thickness to 1 pixel (change this value to adjust the thickness)
    int gridSpacing = 100; // Set grid spacing
    int width = ofGetWidth();
    int height = ofGetHeight();
    int centerX = width / 2;
    int centerY = height / 2;

    for (int x = centerX; x < width; x += gridSpacing) {
        ofDrawLine(x, 0, x, height);
    }
    for (int x = centerX; x > 0; x -= gridSpacing) {
        ofDrawLine(x, 0, x, height);
    }
    for (int y = centerY; y < height; y += gridSpacing) {
        ofDrawLine(0, y, width, y);
    }
    for (int y = centerY; y > 0; y -= gridSpacing) {
        ofDrawLine(0, y, width, y);
    }
}
