#pragma once

#include "ofMain.h"
#include "ofxSVG.h"
#include "attractor.h"
#include "ofxGui.h"
#include "attractorField.h"
#include "particleEnsemble.h"
#include "svgSkeleton.h"

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
    ofPoint initialMousePos;
    ofPoint svgOffset;
    float initialSvgScale;

    // Potential field
    ofImage potentialField;
    bool potentialFieldUpdated;
    bool showPotentialField; // Flag to control potential field visualization
    bool contourLinesUpdated; // Flag to control contour line update

    // Downscale factor
    int downscaleFactor = 3; // Downscale factor for lower resolution calculations
    
    // GUI
    ofxPanel gui;
    ofParameter<bool> showPotentialFieldGui;
    ofParameter<ofColor> potentialFieldColor; // Add color wheel
    ofParameter<ofColor> svgPointsColor; // New parameter for SVG points color
    ofParameter<string> fpsDisplay; // Add FPS display
    ofParameter<bool> showAttractorCircles; // Add checkbox for attractor circles
    ofParameter<bool> showContourLines; // Add checkbox for contour lines
    ofParameter<int> downscaleFactorGui; // Add slider for downscale factor
    
    // New parameters for additional information
    ofParameter<string> windowSize;
    ofParameter<string> svgFileName;
    ofParameter<ofVec2f> svgCentroid;
    ofParameter<float> svgScale;
    
    // Separate panel for attractor information
    ofxPanel attractorGui;
    
    // Dynamic GUI elements for attractors
    std::vector<ofParameterGroup> attractorGroups;
    std::vector<ofParameter<ofVec2f>> attractorCenters;
    std::vector<ofParameter<float>> attractorRadii;
    std::vector<ofParameter<float>> attractorAmplitudes;

    // New parameter for displaying the number of points
    ofParameter<string> numPointsDisplay;
    
    void addAttractorGui(const attractor& attractor);
    void removeAttractorGui(int index);
    void updateAttractorGui(int index, const attractor& attractor);
};
