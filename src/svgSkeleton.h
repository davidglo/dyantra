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

    const std::vector<glm::vec3>& getEquidistantPoints() const;
    const ofPoint& getSvgCentroid() const;

private:
    ofxSVG svg;
    std::vector<glm::vec3> equidistantPoints;
    ofPoint svgCentroid;
};
