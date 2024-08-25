#pragma once

#include "ofMain.h"
#include "ofxSVG.h"

class svgSkeleton {
public:
    void loadSvg(const std::string& filename);
    void generateEquidistantPoints(int numDesiredPoints);
    void calculateSvgCentroid();
    void translateSvg(const ofPoint& offset);
    void resizeSvg(float scale, bool loadingSvg);
    void rotateSvg(float angleDelta, bool loadingSvg); // Declaration
    void draw() const;
    
    void updateSvgCentroid();
    const ofPoint& getInitialCentroid() const;
    
    void autoFitToWindow(int windowWidth, int windowHeight);
    
    const std::vector<glm::vec3>& getEquidistantPoints() const;
    const ofPoint& getSvgCentroid() const;

    std::string getFileName() const {return fileName;}
    float getCumulativeScale() const {return cumulativeScale;}
    
    float getCurrentRotationAngle() const{return currentRotationAngle;}
    
    // Check if a point is near the SVG centroid
    bool isNearCentroid(const ofPoint& point, float threshold) const;

    // Check if a point is near the endpoints of the cross
    bool isNearCrossEndPoints(const ofPoint& point) const;
    
    bool canScale(float scaleFactor, int windowWidth, int windowHeight) const;
    bool canTranslate(const ofPoint& newCenter, int windowWidth, int windowHeight) const;
    
    // Function to check if a specific circle can be dragged within the window bounds
    bool canDragCircle(const ofPoint& circlePosition, const ofPoint& offset, int windowWidth, int windowHeight) const;
    
    // Optionally, if you need a method to get the specific circle position:
    ofPoint getCrossCirclePosition() const;
    
    std::vector<ofPoint> getScalingHandlePositions() const; // Get the positions of the scaling handles
    ofPoint getRotationalHandlePosition() const; // Get the position of the rotational handle
    bool isNearRotationalHandle(const ofPoint& mousePos) const; // Check if near the rotational handle
    
    void drawDottedCircle(const ofPoint& center, float radius, float dotLength, float gapLength) const;
    
    void calculateAdjustedCrossSize();
    
private:
    ofxSVG svg;
    std::vector<glm::vec3> equidistantPoints;
    ofPoint svgCentroid;
    std::string fileName; // Add this member to store the file name
    ofPoint translation;
    float cumulativeScale;
    ofPoint initialCentroid;
    float crossSizeScaleFactor;
    float maxDistanceX, maxDistanceY;
    float currentRotationAngle;
    float crossSizeX,crossSizeY;
    
    void calculateMaxDistances(float& maxDistanceX, float& maxDistanceY) const;
    
};
