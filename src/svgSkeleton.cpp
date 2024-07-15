#include "svgSkeleton.h"
#include <cmath>


void svgSkeleton::loadSvg(const std::string& filename) {
    svg.load(filename);
    fileName = filename; // Store the file name
    generateEquidistantPoints(2000); // Default number of points
    calculateSvgCentroid();
    cumulativeScale = 1.0f; // Reset cumulative scale when a new SVG is loaded
}

std::string svgSkeleton::getFileName() const {
    return fileName;
}

float svgSkeleton::getCumulativeScale() const {
    return cumulativeScale;
}

void svgSkeleton::generateEquidistantPoints(int numDesiredPoints) {
    float totalPathLength = 0;
    std::vector<std::pair<ofPolyline, float>> polylinesWithLengths;

    // Step 1: Calculate the total length of all paths and identify vertices
    std::vector<glm::vec3> vertices;
    int numPaths = svg.getNumPath();
    for (int i = 0; i < numPaths; i++) {
        ofPath path = svg.getPathAt(i);
        path.setPolyWindingMode(OF_POLY_WINDING_ODD);  // Ensure proper winding mode

        auto polylines = path.getOutline();
        for (auto& polyline : polylines) {
            float pathLength = polyline.getPerimeter();
            totalPathLength += pathLength;
            polylinesWithLengths.emplace_back(polyline, pathLength);

            // Identify vertices
            auto points = polyline.getVertices();
            for (size_t j = 0; j < points.size(); ++j) {
                // First and last points are always vertices
                if (j == 0 || j == points.size() - 1) {
                    vertices.push_back(glm::vec3(points[j].x, points[j].y, points[j].z));
                } else {
                    // Calculate the angle between adjacent segments
                    glm::vec3 prev(points[j - 1].x - points[j].x, points[j - 1].y - points[j].y, points[j - 1].z - points[j].z);
                    glm::vec3 next(points[j + 1].x - points[j].x, points[j + 1].y - points[j].y, points[j + 1].z - points[j].z);
                    float angle = std::acos(glm::dot(glm::normalize(prev), glm::normalize(next)));

                    // If angle is sharp, consider it a vertex
                    if (angle < glm::radians(170.0f)) {  // Threshold angle can be adjusted
                        vertices.push_back(glm::vec3(points[j].x, points[j].y, points[j].z));
                    }
                }
            }
        }
    }

    // Ensure unique vertices
    std::sort(vertices.begin(), vertices.end(), [](const glm::vec3& a, const glm::vec3& b) {
        return a.x == b.x ? a.y < b.y : a.x < b.x;
    });
    vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());

    size_t numVertices = vertices.size(); // Use size_t for numVertices

    // If numDesiredPoints is less than numVertices + 1 (for centroid), set it to numVertices + 1
    if (numDesiredPoints < numVertices + 1) {
        numDesiredPoints = numVertices + 1;
    }

    int remainingPoints = numDesiredPoints - static_cast<int>(numVertices) - 1;  // Subtract 1 for the centroid

    equidistantPoints.clear();  // Clear points vector before generating
    equidistantPoints.reserve(numDesiredPoints); // Reserve space for the total number of points

    // Step 2: Add vertices to equidistantPoints
    equidistantPoints.insert(equidistantPoints.end(), vertices.begin(), vertices.end());

    float segmentLength = totalPathLength / remainingPoints;

    // Step 3: Distribute remaining points evenly along the polylines
    for (auto& polylineWithLength : polylinesWithLengths) {
        ofPolyline& polyline = polylineWithLength.first;
        float pathLength = polylineWithLength.second;

        // Determine the number of points for this path segment
        int numPointsForPath = std::round(pathLength / segmentLength);

        // Ensure points are distributed approximately equidistantly
        float lengthStep = pathLength / numPointsForPath;
        for (int j = 0; j < numPointsForPath; j++) {
            float lengthAlongPath = j * lengthStep;
            glm::vec3 point = polyline.getPointAtLength(lengthAlongPath);
            equidistantPoints.push_back(point);
        }
    }

    // Step 4: Calculate the centroid
    calculateSvgCentroid();
    glm::vec3 centroid(svgCentroid.x, svgCentroid.y, svgCentroid.z);

    // Step 5: Add the centroid to equidistantPoints
    equidistantPoints.push_back(centroid);

    // If we have more points than needed, trim the vector
    if (equidistantPoints.size() > numDesiredPoints) {
        equidistantPoints.resize(numDesiredPoints);
    }
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
    cumulativeScale *= scale; // Update cumulative scale
}

void svgSkeleton::draw() const {
//    ofSetColor(255); // White color for points
    for (const auto& point : equidistantPoints) {
        ofDrawCircle(point, 1); // Draw small circles at each point
    }
/*
    //    code for drawing the centroid; not clear if we need it now that we have a point rendered at the centroid
    ofFill();
    ofDrawCircle(svgCentroid, 5); // Draw a small circle at the centroid
*/
}

const std::vector<glm::vec3>& svgSkeleton::getEquidistantPoints() const {
    return equidistantPoints;
}

const ofPoint& svgSkeleton::getSvgCentroid() const {
    return svgCentroid;
}
