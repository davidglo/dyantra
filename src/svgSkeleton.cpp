#include "svgSkeleton.h"

void svgSkeleton::loadSvg(const std::string& filename) {
    svg.load(filename);
    generateEquidistantPoints(10000); // Default number of points
    calculateSvgCentroid();
}

void svgSkeleton::generateEquidistantPoints(int numDesiredPoints) {
    float totalPathLength = 0;

    std::vector<std::pair<ofPolyline, float>> polylinesWithLengths;

    // Calculate the total length of all paths
    int numPaths = svg.getNumPath();
    for (int i = 0; i < numPaths; i++) {
        ofPath path = svg.getPathAt(i);
        path.setPolyWindingMode(OF_POLY_WINDING_ODD);  // Ensure proper winding mode

        auto polylines = path.getOutline();
        for (auto& polyline : polylines) {
            float pathLength = polyline.getPerimeter();
            totalPathLength += pathLength;
            polylinesWithLengths.emplace_back(polyline, pathLength);
        }
    }

    // Determine the segment length
    float segmentLength = totalPathLength / numDesiredPoints;

    equidistantPoints.clear();  // Clear points vector before generating

    // Distribute points along the polylines
    for (auto& polylineWithLength : polylinesWithLengths) {
        ofPolyline& polyline = polylineWithLength.first;
        float pathLength = polylineWithLength.second;

        int numPointsForPath = round(pathLength / segmentLength);
        for (int j = 0; j < numPointsForPath; j++) {
            float lengthAlongPath = j * segmentLength;
            if (lengthAlongPath < pathLength) {
                glm::vec3 point = polyline.getPointAtLength(lengthAlongPath);
                equidistantPoints.push_back(point);
            }
        }
    }

    calculateSvgCentroid();
}

void svgSkeleton::calculateSvgCentroid() {
    if (equidistantPoints.empty()) return;

    ofPoint sum(0, 0);
    for (const auto& point : equidistantPoints) {
        sum += point;
    }
    svgCentroid = sum / equidistantPoints.size();
}

void svgSkeleton::translateSvg(const ofPoint& offset) {
    for (auto& point : equidistantPoints) {
        point += offset;
    }
    svgCentroid += offset;
}

void svgSkeleton::resizeSvg(float scale) {
    for (auto& point : equidistantPoints) {
        point = svgCentroid + (point - svgCentroid) * scale;
    }
}

void svgSkeleton::draw() const {
    ofSetColor(255); // White color for points
    for (const auto& point : equidistantPoints) {
        ofDrawCircle(point, 1); // Draw small circles at each point
    }

    ofFill();
    ofDrawCircle(svgCentroid, 5); // Draw a small circle at the centroid
}

const std::vector<glm::vec3>& svgSkeleton::getEquidistantPoints() const {
    return equidistantPoints;
}

const ofPoint& svgSkeleton::getSvgCentroid() const {
    return svgCentroid;
}
