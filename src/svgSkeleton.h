#pragma once

#include "ofMain.h"
#include "ofxSVG.h"

class svgSkeleton {
public:
    void loadSvg(const std::string& filename);
    void generateEquidistantPoints(int numDesiredPoints);
    void calculateSvgCentroid();
    void translateSvg(const ofPoint& offset);
    void resizeSvg(float scale);
    void draw() const;
    
    void updateSvgCentroid();
    const ofPoint& getInitialCentroid() const;
    
    const std::vector<glm::vec3>& getEquidistantPoints() const;
    const ofPoint& getSvgCentroid() const;
//    std::string getFileName() const; // Add this method to get the file name
//    float getCumulativeScale() const; // Add this method to get the cumulative scale

    std::string getFileName() const {return fileName;}
    float getCumulativeScale() const {return cumulativeScale;}
    
private:
    ofxSVG svg;
    std::vector<glm::vec3> equidistantPoints;
    ofPoint svgCentroid;
    std::string fileName; // Add this member to store the file name
    ofPoint translation;
    float cumulativeScale;
    ofPoint initialCentroid;
};
