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
  
//    string svgFile = "test-nonConnected.svg";
//    string svgFile = "taraYantra.svg";
//    string svgFile = "taraYantra2.svg";
//    string svgFile = "tara-crown-heart-lotus.svg";
//    string svgFile = "tara-crown-Chakra.svg";
//    string svgFile = "tara-CHL-head.svg";
    string svgFile = "tara-CHL-heart.svg";
//    string svgFile = "tara-CHL-lotus.svg";
//    string svgFile = "cir_seasonal-US.svg";
//    string svgFile = "tara-face.svg";
//    string svgFile = "circle.svg";
//    string svgFile = "line.svg";
//    string svgFile = "2lines.svg";
//    string svgFile = "triangle.svg";

    numPoints = 2000; // Set the desired number of points
    timestep = 0.003;
    gridSpacing = 50; // Set grid spacing
    numSpokes = 16;
    last_timeStep = timestep;
    
    timeReversalInProgress = false;
    nTimeReversalSteps = 120; // Adjust this value as needed
    timeReversalStepCounter = nTimeReversalSteps;
    nTimeReversalCalls = 0;
    timeReversalTimestep = 2000;      // Initialize the timestep for reversal
    timeReversalValueChanged = false;
    
    drawingAttractor = false;
    editingCenter = false;
    editingEdge = false;
    translatingSvg = false;
    resizingSvg = false;
    rotatingSvg = false;
    initialAngle = 0.0f; // To store the initial angle when starting the rotation
    selectedAttractorIndex = -1;

    int width = ofGetWidth() / downscaleFactor;
    int height = ofGetHeight() / downscaleFactor;
    potentialField.allocate(width, height, OF_IMAGE_GRAYSCALE);
    potentialFieldUpdated = true;
    showPotentialField = true; // Initialize the flag to show the potential field
    contourLinesUpdated = true; // Initialize the flag to update contour lines
    showGrid = true;
    drawMenus = true;
    drawFileMenu = true;

    // Initialize sequence control variables
    isSequenceRunning = false;
    currentSequenceIndex = 0;
    sequenceStartTime = 0;
    sequenceDuration = 0;
    sequenceFileLoaded = false;
    elapsedSequenceSteps = 0;
    
    svgSkeleton.loadSvg(svgFile);
    svgSkeleton.generateEquidistantPoints(numPoints); // Call with the data member
    svgSkeleton.autoFitToWindow(ofGetWidth(), ofGetHeight());
    particleEnsemble.initialize(svgSkeleton.getEquidistantPoints()); // initialize the particleEnsemble
    
    // GUI setup
    gui.setup();
    
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
    gui.add(timeReversalStatus.set("Time is reversing:", "FALSE"));  // Add this for displaying time reversal status
    
    
    // Add GUI elements for time reversal
    gui.add(timeReversalActive.set("Time Reversal Active", false));
    gui.add(timeReversalTimestepInput.setup("Time Reversal Step", timeReversalTimestep, -100000, 100000));  // Add the input field
    timeReversalTimestepInput.addListener(this, &ofApp::onTimeReversalTimestepInputUpdated);
    
    gui.add(showPotentialFieldGui.set("Show Potential Field", true));
    
    gui.add(flipPotentialFieldRender.set("Flip Potential Field", false)); // Add the checkbox to the GUI
    flipPotentialFieldRender.addListener(this, &ofApp::onFlipPotentialFieldRenderChanged);     // Add listener to the checkbox
    
    gui.add(showAttractorCircles.set("Show Attractor Circles", true)); // Add checkbox for attractor circles
    gui.add(showContourLines.set("Show Contour Lines", true)); // Add checkbox for contour lines
    gui.add(contourThresholdSlider.setup("Contour Threshold", 10000, 0.0, 50000.0));  // Initialize the contour threshold slider
    gui.add(downscaleFactorGui.set("Downscale Factor", 3, 1, 10)); // Add slider for downscale factor

    gui.add(showGrid.set("Show Grid", true));  // Add the checkbox for the grid
    regenerateGridIntersections();  // Generate initial grid intersections
    
    // Setup GUI for snapping
    gui.add(enableSnapping.set("Enable Snapping", true));
    
    // button for running the sequence
    gui.add(runSequenceToggle.setup("Run Sequence", false));

//    ofColor initialPotentialFieldColor(128, 128, 128); // 50% intensity of white color
    ofColor initialPotentialFieldColor(80, 80, 80);
    gui.add(potentialFieldColor.set("Potential Field Color", initialPotentialFieldColor, ofColor(0, 0), ofColor(255, 255))); // Add color wheel

    // svg related input
    
    svgInfoGui.setup();
    svgInfoGui.setPosition(gui.getPosition().x + gui.getWidth() + 10, gui.getPosition().y);
    svgInfoGui.add(svgFileName.set("svgFile ", svgSkeleton.getFileName())); // Use getFileName method
    svgInfoGui.add(showSvgPoints.setup("Show SVG Points", true));  // Initialize the new toggle
    svgInfoGui.add(svgMidpoint.set("svgMidpoint", ofVec2f(svgSkeleton.getSvgCentroid().x, svgSkeleton.getSvgCentroid().y)));
    svgInfoGui.add(svgScale.set("svgScale", 1.0f)); // Initial scale is 1.0
    svgInfoGui.add(svgRotationAngle.set("SVG rot (deg)", ofRadToDeg(svgSkeleton.getCurrentRotationAngle())));
    ofColor initialSvgPointsColor(178, 178, 178); // 70% intensity of white color
    svgInfoGui.add(svgPointsColor.set("SVG Points Color", initialSvgPointsColor, ofColor(0, 0), ofColor(255, 255))); // Add color wheel for SVG points
    
    contourThresholdSlider.addListener(this, &ofApp::onContourThresholdChanged);  // Add listener to the slider
    
    // Setup and position the attractor information panel
    attractorGui.setup();
    attractorGui.setPosition(ofGetWidth() - 210, gui.getPosition().y); // Position to the right of the main panel
    
    // file handling GUI
    fileGui.setup();
    fileGui.setPosition(gui.getPosition().x, ofGetHeight() - 140);
    
    // Add save and load buttons to the main GUI
    fileGui.add(saveButton.setup("Click to Save Filename"));
    fileGui.add(buttonToPasteSaveFilenameFromClipboard.setup("  Paste from Clipboard"));
    fileGui.add(saveFileNameInput.setup("", "settings.xml"));
    saveButton.addListener(this, &ofApp::saveSettings);
    
    fileGui.add(loadButton.setup("Click to Load Filename"));
    fileGui.add(buttonToPasteLoadFilenameFromClipboard.setup("  Paste from Clipboard"));
    fileGui.add(loadFileNameInput.setup("", "settings.xml")); // Default to "settings.xml"
    loadButton.addListener(this, &ofApp::onLoadSettingsButtonPressed); // Attach the load listener

    // Add listeners for the buttons
    buttonToPasteLoadFilenameFromClipboard.addListener(this, &ofApp::onPasteLoadFilenameButtonPressed);
    buttonToPasteSaveFilenameFromClipboard.addListener(this, &ofApp::onPasteSaveFilenameButtonPressed);
    
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
        svgSkeleton.calculateSvgMidpoint();
        particleEnsemble.initialize(svgSkeleton.getEquidistantPoints());
    }
    
    if (potentialFieldUpdated) {
        calculatePotentialField();
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
    
    // Update SVG midpoint display
    svgMidpoint = ofVec2f(svgSkeleton.getSvgCentroid().x, svgSkeleton.getSvgCentroid().y);
    
    // Update SVG scale display
    svgScale = svgSkeleton.getCumulativeScale();
    
    // Update the SVG rotation angle
    svgRotationAngle = ofRadToDeg(svgSkeleton.getCurrentRotationAngle());
    
    if (timeReversalValueChanged){
        timeReversalTimestepInput = timeReversalTimestep;
        timeReversalValueChanged = false;
    }

    // Update GUI elements for attractors       
    for (size_t i = 0; i < attractorField.getAttractors().size(); ++i) {
        updateAttractorGui(i, attractorField.getAttractors()[i]);
    }
        
    // Handle particle motion if playing
    if (isPlaying) {
        float dt;
        if (timeReversalInProgress) {
            dt = gentlyReverseTimeWithCos();
            timeReversalStatus = "TRUE";
        }
        else {
/*
            if (timeForward) {dt = timestep;}
            else {dt = -timestep;}      // If timeForward is false, we want dt to be negative
            timeReversalStatus = "FALSE";
 */
            // Check if time reversal should start
            if (timeReversalActive && elapsedTimesteps == timeReversalTimestepInput) {
                timeReversalTimestepInput = -timeReversalTimestepInput;
                timeReversalInProgress = true;
                nTimeReversalCalls = 0;
                timeReversalStepCounter = nTimeReversalSteps;
                if (timeForward){originalTimeStep = timestep;}
                else{originalTimeStep = -1.0*timestep;}
            }
            
            dt = timeForward ? timestep : -timestep;  // Continue with normal time progression
            timeReversalStatus = "FALSE";
        }
        particleEnsemble.vv_propagatePositionsVelocities(attractorField.getAttractors(), dt);
        
        if (timeForward) {
            elapsedTimesteps++;  // Increment elapsed timesteps
        }
        else {
            elapsedTimesteps--;  // Decrement elapsed timesteps
        }
        elapsedTimestepsDisplay = ofToString(elapsedTimesteps);  // Update GUI display
    }
    
    // Check and run the sequence if the toggle is active
    if (runSequenceToggle) {
        runSequence();
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
    
    // unlike the particleEnsemble, the svgSkeleton points include the midpoint
    // we only draw the svgSkeleton points if explicitly indicated
    particleEnsemble.draw();
    if (showSvgPoints) {svgSkeleton.draw();}
    
    
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
    if(drawMenus){
        gui.draw();
        attractorGui.draw(); // Draw the attractor information panel
        svgInfoGui.draw(); // Draw the SVG information panel
    }
    if (drawFileMenu){
        fileGui.draw();
    }
}

void ofApp::mousePressed(int x, int y, int button) {
    ofPoint mousePos(x, y);
    
    if (button == OF_MOUSE_BUTTON_LEFT) {
        
        // Check if we are clicking near the midpoint to start translating
        if (svgSkeleton.isNearCentroid(mousePos, 10)) {
            initialMousePos = mousePos;
            translatingSvg = true;
            return;
        }
        
        // Check if we are clicking on a handle to start rotating
        if (svgSkeleton.isNearRotationalHandle(mousePos)) {
            initialMousePos = mousePos;
            rotatingSvg = true;
            initialAngle = atan2(mousePos.y - svgSkeleton.getSvgCentroid().y, mousePos.x - svgSkeleton.getSvgCentroid().x);
            return;
        }
        
        // Check if we are clicking on an SVG handle to start resizing
        if (svgSkeleton.isNearCrossEndPoints(mousePos)) {
            initialMousePos = mousePos;
            initialSvgScale = 1.0f;
            resizingSvg = true;
            return;
        }
        
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
        
        // Otherwise, we are drawing a new attractor
        tempAttractor = attractor(mousePos, 20);
        drawingAttractor = true;
    }
    else if (button == OF_MOUSE_BUTTON_RIGHT) {
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
    ofPoint mousePos(x, y);
    if(rotatingSvg){
        // Calculate the angle between the initial mouse position and the current mouse position relative to the midpoint
        float currentAngle = atan2(mousePos.y - svgSkeleton.getSvgCentroid().y, mousePos.x - svgSkeleton.getSvgCentroid().x);
        float angleDelta = currentAngle - initialAngle;

        // Apply the rotation to the SVG
        svgSkeleton.rotateSvg(angleDelta, false);

        if (enableSnapping) {
            // Get the position of the rotational handle after the rotation
            svgSkeleton.calculateAdjustedCrossSize();
            ofPoint rotationHandlePos = svgSkeleton.getRotationalHandlePosition();

            // Calculate the angle of the rotational handle relative to the midpoint
            float handleAngle = atan2(rotationHandlePos.y - svgSkeleton.getSvgCentroid().y, rotationHandlePos.x - svgSkeleton.getSvgCentroid().x);

            // Find the nearest grid intersection in terms of angle
            float minAngleDifference = FLT_MAX;
            float snappedAngle = handleAngle;  // Initialize with the current angle

            for (const auto& intersection : gridIntersections) {
                float intersectionAngle = atan2(intersection.y - svgSkeleton.getSvgCentroid().y, intersection.x - svgSkeleton.getSvgCentroid().x);
                float angleDifference = fabs(intersectionAngle - handleAngle);

                // Wrap angle difference to be within [0, PI]
                angleDifference = fmod(angleDifference + PI, TWO_PI) - PI;
                angleDifference = fabs(angleDifference);

                if (angleDifference < minAngleDifference) {
                    minAngleDifference = angleDifference;
                    snappedAngle = intersectionAngle;
                }
            }

            // If the angle difference is within the snapping threshold, snap the rotation
            if (minAngleDifference < 0.02) {  // Adjust the threshold as needed
                angleDelta += snappedAngle - handleAngle;

                // Apply the snapped rotation
                svgSkeleton.rotateSvg(snappedAngle - handleAngle, false);
            }
        }

        // Update the particle positions and SVG state
        particleEnsemble.update(svgSkeleton.getEquidistantPoints());

        // Update the initial angle for the next mouse move
        initialAngle = currentAngle;

    }
    if (drawingAttractor) {
         float radius = ofDist(tempAttractor.getCenter().x, tempAttractor.getCenter().y, x, y);
         radius = std::max(radius, 10.0f);  // Ensure minimum radius of 10
         tempAttractor.setRadius(radius);
    }
    else if (editingCenter && selectedAttractorIndex >= 0) {
         ofPoint newCenter(x, y);
         if (enableSnapping) {
             float minDistance;
             ofPoint nearestVertex = getNearestSvgVertex(newCenter, minDistance);
             if (minDistance < 10) {
                 newCenter = nearestVertex;
             }
             
             if (showGrid) {
                 ofPoint nearestIntersection = getNearestGridIntersection(newCenter, minDistance);
                 if (minDistance < 10) {
                     newCenter = nearestIntersection;
                 }
             }
         }
         attractorField.setAttractorCenter(selectedAttractorIndex, newCenter);
    }
    else if (editingEdge && selectedAttractorIndex >= 0) {
         ofPoint attractorCenter = attractorField.getAttractors()[selectedAttractorIndex].getCenter();
         float radius = ofDist(attractorCenter.x, attractorCenter.y, mousePos.x, mousePos.y);
         if (enableSnapping) {
             float minDistance;
             ofPoint nearestVertex = getNearestSvgVertex(mousePos, minDistance);
             if (minDistance < 10) { // Snap to the nearest vertex if within 10 pixels
                 radius = ofDist(attractorCenter.x, attractorCenter.y, nearestVertex.x, nearestVertex.y);
             }
             if (showGrid){
                 ofPoint nearestIntersection = getNearestGridIntersection(mousePos, minDistance);
                 if (minDistance < 10){
                     radius = ofDist(attractorCenter.x, attractorCenter.y, nearestIntersection.x, nearestIntersection.y);
                 }
             }
         }
         radius = std::max(radius, 5.0f);  // Ensure minimum radius of 5
         attractorField.setAttractorRadius(selectedAttractorIndex, radius);
         attractorRadiusInputs[selectedAttractorIndex]->getParameter().cast<float>().set(radius); // Synchronize the GUI radius input fields

    }
    else if (translatingSvg) {
         ofPoint offset(mousePos.x - initialMousePos.x, mousePos.y - initialMousePos.y);

         // Calculate the new potential midpoint position
         ofPoint newCenter = svgSkeleton.getSvgCentroid() + offset;

         // Enforce boundaries to keep the midpoint within the window
         if (mousePos.x > 10 && mousePos.x < (ofGetWidth()-10) && mousePos.y > 10 && mousePos.y < (ofGetHeight()-10)){
             if (enableSnapping) {
                 float minDistance;
                 ofPoint nearestIntersection = getNearestGridIntersection(newCenter, minDistance);
                 if (minDistance < 10) {
                     ofPoint snappedOffset = nearestIntersection - svgSkeleton.getSvgCentroid();
                     svgSkeleton.translateSvg(snappedOffset);
                     particleEnsemble.update(svgSkeleton.getEquidistantPoints());
                 } else {
                     svgSkeleton.translateSvg(offset);
                     particleEnsemble.update(svgSkeleton.getEquidistantPoints());
                 }
             } else {
                 svgSkeleton.translateSvg(offset);
                 particleEnsemble.update(svgSkeleton.getEquidistantPoints());
             }

             svgSkeleton.calculateSvgMidpoint();
             initialMousePos = mousePos; // Update initialMousePos with the current mouse position
         }
    }
    else if (resizingSvg) {
        float initialDistance = initialMousePos.distance(svgSkeleton.getSvgCentroid());
        float currentDistance = ofPoint(x, y).distance(svgSkeleton.getSvgCentroid());
        float scaleFactor = currentDistance / initialDistance;
        
        // Calculate the potential new handle positions after scaling
        svgSkeleton.calculateAdjustedCrossSize();
        auto newScalingHandles = svgSkeleton.getScalingHandlePositions();
        
        // Snapping logic
        if (enableSnapping) {
            for (auto& handlePos : newScalingHandles) {
                // Calculate the new position of each handle after scaling
                ofPoint newHandlePos = svgSkeleton.getSvgCentroid() + (handlePos - svgSkeleton.getSvgCentroid()) * scaleFactor;
                float minDistance;
                ofPoint nearestIntersection = getNearestGridIntersection(newHandlePos, minDistance);
                if (minDistance < 10) {
                    float handleDistanceBefore = svgSkeleton.getSvgCentroid().distance(handlePos);
                    float handleDistanceAfter = svgSkeleton.getSvgCentroid().distance(nearestIntersection);
                    scaleFactor = handleDistanceAfter / handleDistanceBefore;
                }
            }
        }
        
        // Enforce maximum scaling limit
        if (mousePos.x > 0 && mousePos.x < ofGetWidth() && mousePos.y > 0 && mousePos.y < ofGetHeight()){
            svgSkeleton.resizeSvg(scaleFactor,false);
            svgScale = svgSkeleton.getCumulativeScale(); // Update the cumulative scale in the GUI
            particleEnsemble.update(svgSkeleton.getEquidistantPoints());
            initialMousePos.set(x, y);
        }
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
    if (rotatingSvg) {
        rotatingSvg = false;
    }
}

void ofApp::updateContours() {
    int width = ofGetWidth() / downscaleFactor;
    int height = ofGetHeight() / downscaleFactor;
    float contourThreshold = contourThresholdSlider;  // Use the slider value
    attractorField.updateContours(downscaleFactor, width, height, svgSkeleton.getEquidistantPoints(), contourThreshold);
    calculatePotentialField();
}

void ofApp::calculatePotentialField(){
    int width = ofGetWidth() / downscaleFactor;
    int height = ofGetHeight() / downscaleFactor;
    float contourThreshold = contourThresholdSlider;  // Use the slider value
    if (flipPotentialFieldRender) {
        attractorField.calculatePotentialField(potentialField, downscaleFactor, width, height, -1 * contourThresholdSlider);  // Flip the sign
    }
    if (!flipPotentialFieldRender){
        attractorField.calculatePotentialField(potentialField, downscaleFactor, width, height, contourThresholdSlider);
    }
}

void ofApp::windowResized(int w, int h) {
    int width = w / downscaleFactor;
    int height = h / downscaleFactor;
    regenerateGridIntersections();  // Regenerate grid intersections when the window is resized
    potentialField.allocate(width, height, OF_IMAGE_GRAYSCALE); // Reallocate the potential field image
    potentialFieldUpdated = true; // Mark the potential field as needing an update
    contourLinesUpdated = true; // Mark contour lines for update
    attractorGui.setPosition(ofGetWidth() - 210, gui.getPosition().y); // Position to the right of the main panel
    fileGui.setPosition(gui.getPosition().x, ofGetHeight() - 140);
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
        if(!timeReversalInProgress){  // only take action if there's not already a time reversal in progress
            timeReversalInProgress = true;
            nTimeReversalCalls = 0;
            timeReversalStepCounter = nTimeReversalSteps;
            if (timeForward){originalTimeStep = timestep;}
            else{originalTimeStep = -1.0*timestep;}
        }
    }
    if (key == 'r' || key == 'R') {
        resetSimulation();
    }
    if (key == 'c' || key == 'C') {
        showContourLines = !showContourLines; // Toggle the flag
        showContourLines.set(showContourLines); // Sync the GUI checkbox
    }
    if(key == 'a' || key == 'A'){
        showAttractorCircles = !showAttractorCircles;
        showAttractorCircles.set(showAttractorCircles);
    }
    if (key == 'g' || key == 'G') {
        showGrid = !showGrid;     // Toggle the flag
        showGrid.set(showGrid);
        if(!showGrid){enableSnapping = false;}
    }
    if (key == 'w' || key == 'W') {
        svgSkeleton.writeSvg(particleEnsemble.getPositions());   // Assuming svgSkeleton is an instance of your svgSkeleton class
    }
    if (key == 'm' || key == 'M') {
        drawMenus = !drawMenus; // Toggle the flag
    }
    if(key == 'f' || key == 'F'){
        drawFileMenu = !drawFileMenu;
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
    amplitudeInput->setup("Edit Amplitude:", attractor.getAmplitude(), -100000.0f, 100000.0f);
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
    timeReversalInProgress = false;
    timeForward = true; // Reset to forward direction
}

void ofApp::drawGrid() {
    ofSetColor(64); // Set grid color to darker grey (64, 64, 64)
    ofSetLineWidth(1); // Set the line thickness to 1 pixel

    int width = ofGetWidth();
    int height = ofGetHeight();
    int centerX = width / 2;
    int centerY = height / 2;
    
    // Calculate the maximum radius for the concentric circles
    float maxRadius = std::max(width, height) / 2.0f;
    float radiusStep = gridSpacing; // Define the spacing between circles

    int numSegments = numSpokes*8; // Number of segments for smoother circles

    // Draw concentric circles with a smoother appearance
    for (float r = radiusStep; r <= maxRadius; r += radiusStep) {
        ofNoFill();
        ofBeginShape();
        for (int i = 0; i < numSegments; ++i) {
            float angle = ofMap(i, 0, numSegments, 0, TWO_PI);
            float x = centerX + r * cos(angle);
            float y = centerY + r * sin(angle);
            ofVertex(x, y);
        }
        ofEndShape(true);
    }

    // Draw radial lines (spokes)
    float angleStep = 360.0f / numSpokes;

    for (int i = 0; i < numSpokes; ++i) {
        float angle = ofDegToRad(i * angleStep);
        float x = centerX + maxRadius * cos(angle);
        float y = centerY + maxRadius * sin(angle);
        ofDrawLine(centerX, centerY, x, y);
    }

    // Draw the central lines (same as in the rectangular grid)
    ofDrawLine(centerX, 0, centerX, height); // Central vertical line
    ofDrawLine(0, centerY, width, centerY); // Central horizontal line
}

void ofApp::regenerateGridIntersections() {
    gridIntersections.clear();
    int width = ofGetWidth();
    int height = ofGetHeight();
    int centerX = width / 2;
    int centerY = height / 2;

    // Calculate the maximum radius for the grid
    float maxRadius = std::max(width, height) / 2.0f;
    float radiusStep = gridSpacing;
    float angleStep = 360.0f / numSpokes;

    // Generate intersections for concentric circles and spokes
    for (float r = radiusStep; r <= maxRadius; r += radiusStep) {
        for (int i = 0; i < numSpokes; ++i) {
            float angle = ofDegToRad(i * angleStep);
            float x = centerX + r * cos(angle);
            float y = centerY + r * sin(angle);
            gridIntersections.emplace_back(x, y);
        }
    }

    // Ensure the center point is included
    gridIntersections.emplace_back(centerX, centerY);
}

ofPoint ofApp::getNearestGridIntersection(const ofPoint& point, float& minDistance) {
    ofPoint nearestIntersection;
    minDistance = FLT_MAX;

    for (const auto& intersection : gridIntersections) {
        float distance = point.distance(intersection);
        if (distance < minDistance) {
            minDistance = distance;
            nearestIntersection = intersection;
        }
    }
    return nearestIntersection;
}

/*
void ofApp::drawRectangularGrid() {
    ofSetColor(64); // Set grid color to darker grey (64, 64, 64)
    ofSetLineWidth(1); // Set the line thickness to 1 pixel
    int width = ofGetWidth();
    int height = ofGetHeight();
    int centerX = width / 2;
    int centerY = height / 2;

    // Draw vertical lines
    for (int x = centerX; x < width; x += gridSpacing) {
        ofDrawLine(x, 0, x, height);
    }
    for (int x = centerX; x > 0; x -= gridSpacing) {
        ofDrawLine(x, 0, x, height);
    }

    // Draw horizontal lines
    for (int y = centerY; y < height; y += gridSpacing) {
        ofDrawLine(0, y, width, y);
    }
    for (int y = centerY; y > 0; y -= gridSpacing) {
        ofDrawLine(0, y, width, y);
    }

    // Draw the central lines with a thicker width
    ofSetColor(180); // Set grid color to darker grey (64, 64, 64)
    ofSetLineWidth(1); // Set the line thickness to 1 pixel
    ofDrawLine(centerX, 0, centerX, height); // Central vertical line
    ofDrawLine(0, centerY, width, centerY); // Central horizontal line
}

void ofApp::regenerateRectangularGridIntersections() {
    std::set<std::pair<int, int>> uniqueIntersections; // Use a set to store unique points
    gridIntersections.clear();
    int width = ofGetWidth();
    int height = ofGetHeight();
    int centerX = width / 2;
    int centerY = height / 2;

    // Generate grid intersections in all four quadrants

    // Bottom-right quadrant
    for (int x = centerX; x < width; x += gridSpacing) {
        for (int y = centerY; y < height; y += gridSpacing) {
            uniqueIntersections.emplace(x, y);
        }
    }

    // Bottom-left quadrant
    for (int x = centerX; x > 0; x -= gridSpacing) {
        for (int y = centerY; y < height; y += gridSpacing) {
            uniqueIntersections.emplace(x, y);
        }
    }

    // Top-right quadrant
    for (int x = centerX; x < width; x += gridSpacing) {
        for (int y = centerY; y > 0; y -= gridSpacing) {
            uniqueIntersections.emplace(x, y);
        }
    }

    // Top-left quadrant
    for (int x = centerX; x > 0; x -= gridSpacing) {
        for (int y = centerY; y > 0; y -= gridSpacing) {
            uniqueIntersections.emplace(x, y);
        }
    }

    // Ensure the center point is included
    uniqueIntersections.emplace(centerX, centerY);

    // Transfer unique points to the vector
    for (const auto& point : uniqueIntersections) {
        gridIntersections.emplace_back(point.first, point.second);
    }
}

ofPoint ofApp::getNearestRectangularGridIntersection(const ofPoint& point, float& minDistance) {
    ofPoint nearestIntersection;
    minDistance = FLT_MAX;

    for (const auto& intersection : gridIntersections) {
        float distance = point.distance(intersection);
        if (distance < minDistance) {
            minDistance = distance;
            nearestIntersection = intersection;
        }
    }

    return nearestIntersection;
}
*/

float ofApp::gentlyReverseTimeWithCos() {
    float new_timeStep, stepSize;
    
    ++nTimeReversalCalls;
    
    stepSize = 2 * PI / (2 * nTimeReversalSteps + 1);
    
    if (timeReversalStepCounter > 0) {       // slowly reduce the size of the timestep
        new_timeStep = originalTimeStep * 0.5 * (cos(nTimeReversalCalls * stepSize) + 1);
    } else if (timeReversalStepCounter == 0) {   // flip the sign
        new_timeStep = -1.0 * last_timeStep;
        originalTimeStep *= -1;
        timeForward = !timeForward;
        timeDirectionDisplay = timeForward ? "FORWARD" : "BACKWARD";
    } else if (timeReversalStepCounter < 0) {   // slowly increase the size of the timestep
        new_timeStep = originalTimeStep * 0.5 * (cos(nTimeReversalCalls * stepSize) + 1);
    }
    
    timeReversalStepCounter -= 1;

    if (timeReversalStepCounter < (-1 * nTimeReversalSteps)) {
        timeReversalInProgress = false;
        timeReversalStatus = "FALSE";  // Update the status
    }
    else {
        timeReversalStatus = "TRUE";  // Update the status
    }
    
    last_timeStep = new_timeStep;
    return new_timeStep;
}

// write particle positions as points along polyline
void ofApp::writeParticlePositionsToSvg() {
    if (!isPlaying) {  // Only write if the simulation is paused
        // Get the current time for timestamp
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
        std::string filename = "particle_positions_" + oss.str() + ".svg";

        std::ofstream svgFile;
        svgFile.open(ofToDataPath(filename));

        if (svgFile.is_open()) {
            // Write the SVG header
            svgFile << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n";

            // Write particle positions as a polyline
            svgFile << "<polyline points=\"";
            for (const auto& position : particleEnsemble.getPositions()) {
                svgFile << position.x << "," << position.y << " ";
                ofLogNotice() << "Writing position: " << position.x << ", " << position.y;
            }
            svgFile << "\" fill=\"none\" stroke=\"black\" stroke-width=\"1\" />\n";

            // Write the SVG footer
            svgFile << "</svg>\n";
            svgFile.close();
            ofLogNotice() << "Particle positions written to " << filename;
        } else {
            ofLogError() << "Unable to open file for writing: " << filename;
        }
    }
}

void ofApp::onFlipPotentialFieldRenderChanged(bool & state) {
    // Update the potential field render when the flip state changes
    potentialFieldUpdated = true;
}

void ofApp::onTimeReversalTimestepInputUpdated(int & value){
    if (timeReversalTimestepInput >= 0 && timeReversalTimestepInput <= nTimeReversalSteps){
        timeReversalTimestep = nTimeReversalSteps + 1;
        timeReversalTimestepInput = timeReversalTimestep;
    }
    else if (timeReversalTimestepInput < 0 && timeReversalTimestepInput >= (-nTimeReversalSteps)){
        timeReversalTimestep = -nTimeReversalSteps - 1;
        timeReversalTimestepInput = timeReversalTimestep;
    }
    else {
        timeReversalTimestep = value;
        timeReversalTimestepInput = timeReversalTimestep;
    }
    timeReversalValueChanged = true;
}

void ofApp::saveSettings() {
    
    std::string baseFilename = saveFileNameInput;
    if (baseFilename.empty()) {
        baseFilename = "settings.xml";
    }

    // Separate the base filename and the extension
    std::string extension = ".xml";
    std::string filenameWithoutExtension = baseFilename;

    // If the base filename already ends with ".xml", remove the extension part
    if (baseFilename.size() >= extension.size() && baseFilename.compare(baseFilename.size() - extension.size(), extension.size(), extension) == 0) {
        filenameWithoutExtension = baseFilename.substr(0, baseFilename.size() - extension.size());
    }

    std::string filename = filenameWithoutExtension + extension;
    int counter = 1;

    // Check if the file already exists and append _1, _2, etc., if necessary
    while (ofFile::doesFileExist(filename)) {
        filename = filenameWithoutExtension + "_" + ofToString(counter++) + extension;
    }
    
    // Create an XML object to store the settings
    ofXml settings;
    settings.appendChild("Settings");
    settings.set("Settings");

    // Save the main GUI
    ofXml guiXml = settings.appendChild("gui");
    gui.saveTo(guiXml);

    // Manually save numPointsInput
    ofXml numPointsNode = guiXml.appendChild("numPointsInput");
    numPointsNode.set(numPointsInput.getParameter().toString());

    // Manually save the SVG info GUI parameters (without the <group> tag)
    ofXml svgInfoXml = settings.appendChild("svgInfoGui");
    svgInfoXml.appendChild("svgFile_").set(svgFileName.get());
    svgInfoXml.appendChild("Show_SVG_Points").set((bool)showSvgPoints);
    svgInfoXml.appendChild("svgMidpoint").set(svgMidpoint.get());
    svgInfoXml.appendChild("svgScale").set(svgScale.get());
    svgInfoXml.appendChild("SVG_rot__deg_").set(svgRotationAngle.get());
    svgInfoXml.appendChild("SVG_Points_Color").set(svgPointsColor.get());

    // Save the attractor GUI
    ofXml attractorXml = settings.appendChild("attractorGui");
    
    // Save attractor centers, radius, and amplitude
    for (size_t i = 0; i < attractorCenters.size(); ++i) {
        ofXml attractorGroup = attractorXml.appendChild("Attractor_" + ofToString(i));
        attractorGroup.appendChild("Center").set(attractorCenters[i].get());
        attractorGroup.appendChild("Radius").set(attractorRadiusInputs[i]->getParameter().cast<float>().get());
        attractorGroup.appendChild("Amplitude").set(attractorAmplitudeInputs[i]->getParameter().cast<float>().get());
    }
    
    // Save all settings to an XML file
    settings.save(filename);

    ofLogNotice() << "Settings saved to " << filename;
}

void ofApp::loadSettings(const std::string& filename) {
    // Load the XML file
    
    float scaleForCurrentGraphicsWindow;
    ofPoint oldSvgCentroid;
    ofPoint newSvgCentroid;
    
    ofXml settings;
    if (settings.load(filename)) {
        ofLogNotice() << "Settings loaded from " << filename;
        
        // Load the main GUI
        ofXml guiXml = settings.getChild("gui");
        ofPoint originalWindowSize;
        if (guiXml) {            
            // Load the GUI from the XML
            gui.loadFrom(guiXml);

            // Retrieve the original window size from the loaded GUI
            std::string windowSizeStr = windowSize.get();
            std::vector<std::string> tokens = ofSplitString(windowSizeStr, "x");
            if (tokens.size() == 2) {
                originalWindowSize.x = ofToFloat(tokens[0]);
                originalWindowSize.y = ofToFloat(tokens[1]);
            }
            
            // Manually load numPointsInput
            ofXml numPointsNode = guiXml.findFirst("numPointsInput");
            if (numPointsNode) {
                numPointsInput = ofToInt(numPointsNode.getValue());
            }
        }
        
        scaleForCurrentGraphicsWindow = min(ofGetWidth()/originalWindowSize.x,ofGetHeight()/originalWindowSize.y);
        
        // Load the SVG info GUI
        ofXml svgInfoXml = settings.getChild("svgInfoGui");
        if (svgInfoXml && originalWindowSize.x > 0 && originalWindowSize.y > 0) {
            svgInfoGui.loadFrom(svgInfoXml);

            // Load svgFile_
            ofXml fileNode = svgInfoXml.findFirst("svgFile_");
            if (fileNode) {
                std::string svgFile = fileNode.getValue();
                svgFileName = svgFile;
                svgSkeleton.loadSvg(svgFile);  // Load the SVG file
                svgSkeleton.generateEquidistantPoints(numPoints); // Call with the data member
//                svgSkeleton.autoFitToWindow(ofGetWidth(), ofGetHeight());
                particleEnsemble.initialize(svgSkeleton.getEquidistantPoints()); // initialize the particleEnsemble
            }
            
            // Update the SVG position to match the loaded midpoint
            ofXml midpointNode = svgInfoXml.findFirst("svgMidpoint");
            if (midpointNode) {
                std::string midpointStr = midpointNode.getValue();
                std::vector<std::string> tokens = ofSplitString(midpointStr, ",");
                if (tokens.size() == 2) {

                    ofPoint relativeCentroid;
                    oldSvgCentroid.x = ofToFloat(tokens[0]);
                    oldSvgCentroid.y = ofToFloat(tokens[1]);
                    relativeCentroid.x = oldSvgCentroid.x / originalWindowSize.x;
                    relativeCentroid.y = oldSvgCentroid.y / originalWindowSize.y;

                    // Calculate the new absolute position based on the current window size
//                    ofPoint newCentroid;
                    newSvgCentroid.x = relativeCentroid.x * ofGetWidth();
                    newSvgCentroid.y = relativeCentroid.y * ofGetHeight();

                    // Calculate the translation needed to move the SVG to the new midpoint
                    ofPoint currentCentroid = svgSkeleton.getSvgCentroid();
                    ofPoint translation = newSvgCentroid - currentCentroid;
                    
                    // Apply the translation
                    svgSkeleton.translateSvg(translation);
                    svgSkeleton.calculateSvgMidpoint();
                    particleEnsemble.update(svgSkeleton.getEquidistantPoints());
                    particleEnsemble.reinitialize(svgSkeleton.getEquidistantPoints());
                }
            }

            // Apply the scale if it's specified
            ofXml scaleNode = svgInfoXml.findFirst("svgScale");
            if (scaleNode) {
                float scale = scaleNode.getFloatValue();
                scale = scale * scaleForCurrentGraphicsWindow; //apply scale for current graphics window size
                svgSkeleton.resizeSvg(scale, true);
                particleEnsemble.update(svgSkeleton.getEquidistantPoints());
            }

            // Apply the rotation if it's specified
            ofXml rotationNode = svgInfoXml.findFirst("SVG_rot__deg_");
            if (rotationNode) {
                float rotation = rotationNode.getFloatValue();
                svgSkeleton.rotateSvg(ofDegToRad(rotation), true);
                particleEnsemble.update(svgSkeleton.getEquidistantPoints());
            }
            
            // Load SVG_Points_Color
            ofXml colorNode = svgInfoXml.findFirst("SVG_Points_Color");
            if (colorNode) {
                std::string colorStr = colorNode.getValue();
                std::vector<std::string> colorTokens = ofSplitString(colorStr, ",");
                if (colorTokens.size() == 4) {
                    int r = ofToInt(colorTokens[0]);
                    int g = ofToInt(colorTokens[1]);
                    int b = ofToInt(colorTokens[2]);
                    int a = ofToInt(colorTokens[3]);
                    ofColor color(r, g, b, a);
                    svgPointsColor = color;
                }
            }
            
        }
        
        // Load the attractor GUI..
        // (... but first delete previous attractors & associated GUI elements)
        for (int i = attractorField.getAttractors().size() - 1; i >= 0; --i) {
            attractorField.removeAttractorAt(i);
            removeAttractorGui(i);
        }
        
        ofXml attractorXml = settings.getChild("attractorGui");
        
        ofPoint oldAttCentroid;
        ofPoint newAttCentroid;

        if (attractorXml && originalWindowSize.x > 0 && originalWindowSize.y > 0) {
            attractorCenters.clear();
            attractorRadiusInputs.clear();
            attractorAmplitudeInputs.clear();
            attractorGui.clear();
            
            auto attractorNodes = attractorXml.getChildren();
            for (auto& attractorNode : attractorNodes) {
                if (attractorNode.getName().find("Attractor_") != std::string::npos) {
                    // Load center
                    ofPoint center;
                    ofPoint relativeCenter;
                    if (attractorNode.getChild("Center")) {
                        std::string centerStr = attractorNode.getChild("Center").getValue();
                        std::vector<std::string> tokens = ofSplitString(centerStr, ",");
                        if (tokens.size() == 2) {
                            
                            oldAttCentroid.x = ofToFloat(tokens[0]);
                            oldAttCentroid.y = ofToFloat(tokens[1]);

                            // place the attractor centers so they are correctly scaled relative to the SVG centroid
                            // note: this assumes the attractor positions are correct relative to the svg centroid
                            center = newSvgCentroid + scaleForCurrentGraphicsWindow * (oldAttCentroid - oldSvgCentroid);
                        }
                    }

                    // Load radius
                    float radius = 0;
                    if (attractorNode.getChild("Radius")) {
                        radius = attractorNode.getChild("Radius").getFloatValue();
                        radius = radius * scaleForCurrentGraphicsWindow; // scaling for current graphics window
                    }

                    // Load amplitude
                    float amplitude = 0;
                    if (attractorNode.getChild("Amplitude")) {
                        amplitude = attractorNode.getChild("Amplitude").getFloatValue();
                    }

                    // Add attractor data
                    attractor tempAttractor(center, radius);
                    tempAttractor.setAmplitude(amplitude);
                    attractorField.addAttractor(tempAttractor);
                    addAttractorGui(tempAttractor);
                }
            }
        }
    } else {
        ofLogError() << "Failed to load settings from " << filename;
    }
}

void ofApp::onLoadSettingsButtonPressed() {
    std::string filename = loadFileNameInput; // Alternative way to get the filename
    loadSettings(filename); // Load settings from the specified file
}

void ofApp::onPasteLoadFilenameButtonPressed() {
    // Get the text from the clipboard
    std::string clipboardText = ofGetClipboardString();
    // Set the clipboard text to the loadFileNameInput field
    loadFileNameInput = clipboardText;
}

void ofApp::onPasteSaveFilenameButtonPressed() {
    // Get the text from the clipboard
    std::string clipboardText = ofGetClipboardString();
    // Set the clipboard text to the saveFileNameInput field
    saveFileNameInput = clipboardText;
}

void ofApp::loadSequenceFiles() {
    ofDirectory dir("sequence");
    dir.allowExt("xml");
    dir.listDir();
    sequenceFiles.clear();
    
    for (auto& file : dir) {
        sequenceFiles.push_back(file.getAbsolutePath());
    }
    
    // Sort sequenceFiles alphabetically by filename
    std::sort(sequenceFiles.begin(), sequenceFiles.end());
    
    currentSequenceIndex = 0;
}

void ofApp::runSequence() {
    
    // Check if "Run Sequence" toggle is enabled
    if (!runSequenceToggle) {
        isSequenceRunning = false;
        return;
    }

    // Start sequence if not already running
    if (!isSequenceRunning) {
        loadSequenceFiles();  // Load sequence files
        isSequenceRunning = true;
        currentSequenceIndex = 0;
    }

    // Check if we've finished all files
    if (currentSequenceIndex >= sequenceFiles.size()) {
        isSequenceRunning = false;
        runSequenceToggle = false;  // Turn off the toggle
        return;
    }

    // Check the time elapsed for the current file
    float elapsedTime = elapsedTimesteps;
    if (elapsedTime == 0 && !sequenceFileLoaded) {
        // Load the next file if duration for the current file is complete
        const std::string& filename = sequenceFiles[currentSequenceIndex];
        loadSettings(filename);
        resetSimulation();
        sequenceFileLoaded = true;
        
        sequenceDuration = 2 * (timeReversalTimestep + nTimeReversalSteps);

        sequenceStartTime = ofGetElapsedTimef();  // Reset start time for the new file
        currentSequenceIndex++;                   // Move to the next file
    }
    else if (elapsedTime != 0 && sequenceFileLoaded){
        sequenceFileLoaded = false;
    }
}
