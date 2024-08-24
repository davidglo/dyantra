#pragma once

#include "ofMain.h"
#include "ofxSVG.h"
#include "attractor.h"
#include "ofxGui.h"
#include "attractorField.h"
#include "particleEnsemble.h"
#include "svgSkeleton.h"
#include <fstream>
#include <ctime>
#include <iomanip>

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();

    void mousePressed(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void keyPressed(int key); // Declare the keyPressed method
    
private:
    svgSkeleton svgSkeleton; // Use the new svgSkeleton class
    particleEnsemble particleEnsemble; // Use the new particleEnsemble class

    void updateContours();  // Declare the updateContours method
    void calculatePotentialField();
    
    // New helper function to find the nearest vertex on the SVG paths
    ofPoint getNearestSvgVertex(const ofPoint& point, float& minDistance);
    
    // Variables to handle attractor drawing and editing
    attractorField attractorField; // Use the new attractorField class
    attractor tempAttractor = attractor(ofPoint(0, 0), 0);
    bool drawingAttractor;

    // Editing state for attractors
    bool editingCenter;
    bool editingEdge;
    int selectedAttractorIndex;

    // Editing state for SVG paths
    bool translatingSvg;
    bool resizingSvg;
    bool rotatingSvg;
    ofPoint initialMousePos;
    ofPoint svgOffset;
    float initialSvgScale;
    float initialAngle;

    // Potential field
    ofImage potentialField;
    bool potentialFieldUpdated;
    bool showPotentialField; // Flag to control potential field visualization
    bool contourLinesUpdated; // Flag to control contour line update

    // Downscale factor
    int downscaleFactor = 3; // Downscale factor for lower resolution calculations
    
    // New data member for number of points
    int numPoints;
    
    ofParameter<string> elapsedTimestepsDisplay;  // New parameter for elapsed timesteps
    long long elapsedTimesteps;  // Counter for elapsed timesteps
    
    ofParameter<string> timeDirectionDisplay;  // New parameter for time direction
    bool timeForward;  // Flag to track the direction of time
    
    // GUI
    ofxPanel gui;
    ofParameter<bool> showPotentialFieldGui;
    ofParameter<ofColor> potentialFieldColor; // Add color wheel
    ofParameter<ofColor> svgPointsColor; // New parameter for SVG points color
    ofParameter<string> fpsDisplay; // Add FPS display
    ofParameter<bool> showAttractorCircles; // Add checkbox for attractor circles
    ofParameter<bool> showContourLines; // Add checkbox for contour lines
    ofxFloatSlider contourThresholdSlider;  // New slider for contour threshold
    ofParameter<int> downscaleFactorGui; // Add slider for downscale factor
    
    // New parameters for additional information
    ofParameter<string> windowSize;
    ofParameter<string> svgFileName;
    ofParameter<ofVec2f> svgCentroid;
    ofParameter<float> svgScale;
    ofParameter<float> svgRotationAngle;
    
    ofParameter<string> numPointsDisplay;   // parameter for displaying the number of points
    ofxIntField numPointsInput;             // New input field for number of points
    ofxToggle showSvgPoints;                // New toggle for showing/hiding SVG points
    
    // Separate panel for attractor information
    ofxPanel attractorGui;

    // Dynamic GUI elements for attractors
    std::vector<ofParameterGroup> attractorGroups;
    std::vector<ofParameter<ofVec2f>> attractorCenters;

    std::vector<std::shared_ptr<ofxFloatField>> attractorRadiusInputs;
    std::map<ofxFloatField*, int> radiusInputToAttractorIndex;
    void attractorRadiusChanged(float & radius);
    
    std::vector<std::shared_ptr<ofxFloatField>> attractorAmplitudeInputs;
    std::map<ofxFloatField*, int> amplitudeInputToAttractorIndex;
    void attractorAmplitudeChanged(float & amplitude);
            
    void addAttractorGui(const attractor& attractor);
    void removeAttractorGui(int index);
    void updateAttractorGui(int index, const attractor& attractor);
    
    ofxPanel svgInfoGui;
    
    // Play/pause state
    bool isPlaying = false;

    // Angular velocity for circular motion
    float angularVelocity = 0.05;
    float timestep;
    
    ofxFloatField timestepInput;
    ofParameter<string> timestepLabel_1;
    ofParameter<string> timestepLabel_2;
    
    void onTimestepChanged(float & value) {timestep = value;}
    
    ofParameter<string> playPauseStatus;  // New parameter for play/pause status
    
    void onContourThresholdChanged(float & value); // Add this function
    
    void resetSimulation();
    
    std::vector<ofPoint> gridIntersections;  // Store the grid intersection points
    ofParameter<bool> showGrid; // Declare showGrid as private
    void drawGrid(); // Function to draw the grid
    int gridSpacing;
    
    int numSpokes;  // number of spokes in radial grid
    
    void regenerateGridIntersections();
    // New helper function to find the nearest vertex on the grid
    ofPoint getNearestGridIntersection(const ofPoint& point, float& minDistance);
    
    // Time reversal variables
    bool timeReversalInProgress;
    int nTimeReversalSteps; // Adjust this value as needed
    int timeReversalStepCounter;
    int nTimeReversalCalls;
    float last_timeStep;
    float originalTimeStep;
    
    // Function declaration
    float gentlyReverseTimeWithCos();
    ofParameter<string> timeReversalStatus;  // Add this for displaying time reversal status
    
    ofParameter<bool> enableSnapping;
    
    void writeParticlePositionsToSvg();
    
    // Add the declaration for the flipPotentialFieldRender checkbox
    ofParameter<bool> flipPotentialFieldRender;
    void onFlipPotentialFieldRenderChanged(bool & state);
    
    // GUI elements for time reversal
    ofParameter<bool> timeReversalActive;  // Checkbox to activate/deactivate time reversal
    ofxIntField timeReversalTimestepInput;  // Input field to set the timestep at which reversal occurs

    void onTimeReversalTimestepInputUpdated(int & value);
    
    // Other variables
    int timeReversalTimestep;  // Variable to store the user-defined timestep for reversal
    bool timeReversalValueChanged;
    
    ofxPanel fileGui;
    
    ofxButton saveButton;
    ofxInputField<std::string> saveFileNameInput; // Text input field for loading settings

    ofxButton loadButton;
    ofxInputField<std::string> loadFileNameInput; // Text input field for loading settings
    
    void saveSettings();
    void loadSettings(const std::string& filename);

    void onLoadSettingsButtonPressed();
    
};
