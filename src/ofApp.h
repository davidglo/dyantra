#pragma once

#include "ofMain.h"
#include "ofxSVG.h"
#include "attractor.h"
#include "AttractorField.h"
#include "ParticleEnsemble.h"
#include "SvgSkeleton.h"

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
    SvgSkeleton svgSkeleton; // Use the new SvgSkeleton class
    ParticleEnsemble particleEnsemble; // Use the new ParticleEnsemble class

    void updateContours();  // Declare the updateContours method

    // New helper function to find the nearest vertex on the SVG paths
    ofPoint getNearestSvgVertex(const ofPoint& point, float& minDistance);

    // Variables to handle attractor drawing and editing
    AttractorField attractorField; // Use the new AttractorField class
    Attractor tempAttractor = Attractor(ofPoint(0, 0), 0);
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
};
