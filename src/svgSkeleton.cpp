#include "svgSkeleton.h"
#include <cmath>
#include <glm/vec3.hpp>


void svgSkeleton::loadSvg(const std::string& filename) {
    
    ofFile file(filename);
    if (!file.exists()) {
        ofSystemAlertDialog("Error: SVG file not found.\nPlease make sure the file exists and the path is correct.");
        return;
    }
    
    fileName = filename; // Store the file name
    svg.load(filename);
//    generateEquidistantPoints(2000); // Default number of points
    calculateSvgCentroid();
    translation.set(0, 0);
    cumulativeScale = 1.0f;
    initialCentroid = svgCentroid;
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
    int remainingPoints = numDesiredPoints - static_cast<int>(numVertices) + 1;  // add 1 for the centroid
    
    equidistantPoints.clear();  // Clear points vector before generating
    equidistantPoints.reserve(numDesiredPoints + 1); // Reserve space for the total number of points + centroid
    
    // Step 2: Add vertices to equidistantPoints
    equidistantPoints.insert(equidistantPoints.end(), vertices.begin(), vertices.end());
    
    if (remainingPoints <= 0) {
        // Step 4: Calculate the centroid
        calculateSvgCentroid();
        glm::vec3 centroid(svgCentroid.x, svgCentroid.y, svgCentroid.z);
        
        // Step 5: Add the centroid to equidistantPoints
        equidistantPoints.insert(equidistantPoints.begin(), centroid); // Ensure centroid is the first point
        return;
    }
    
    float segmentLength = totalPathLength / remainingPoints;

    // Step 3: Distribute remaining points evenly along the polylines
    for (auto& polylineWithLength : polylinesWithLengths) {
        ofPolyline& polyline = polylineWithLength.first;
        float pathLength = polylineWithLength.second;

        // Determine the number of points for this path segment
        int numPointsForPath = std::round(pathLength / segmentLength);

        // Ensure points are distributed approximately equidistantly
        float lengthStep = pathLength / numPointsForPath;
        for (int j = 0; j <= numPointsForPath; j++) {
            float lengthAlongPath = j * lengthStep;
            glm::vec3 point = polyline.getPointAtLength(lengthAlongPath);
            
            // Check if the point is already in equidistantPoints
            if (std::find(equidistantPoints.begin(), equidistantPoints.end(), point) == equidistantPoints.end()) {
                equidistantPoints.push_back(point);
            }
        }
    }
        
    // Step 4: Calculate the centroid
    calculateSvgCentroid();
    glm::vec3 centroid(svgCentroid.x, svgCentroid.y, svgCentroid.z);
    
    // Step 5: Add the centroid to equidistantPoints
    equidistantPoints.insert(equidistantPoints.begin(), centroid); // Ensure centroid is the first point
    
    // Apply the stored translation and scale to the newly generated points
    for (auto& point : equidistantPoints) {
        point = svgCentroid + (point - svgCentroid) * cumulativeScale + translation;
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
    translation += offset;
    for (auto& point : equidistantPoints) {
        point += offset;
    }
    svgCentroid += offset;
    initialCentroid += offset; // Ensure the initialCentroid is also updated
}

void svgSkeleton::resizeSvg(float scaleFactor) {
    cumulativeScale *= scaleFactor;
    for (auto& point : equidistantPoints) {
        point = svgCentroid + (point - svgCentroid) * scaleFactor;
    }
    // After resizing, recalculate the centroid
    calculateSvgCentroid();
}

void svgSkeleton::draw() const {
    for (const auto& point : equidistantPoints) {
        ofDrawCircle(point, 1); // Draw small circles at each point
    }
}

const std::vector<glm::vec3>& svgSkeleton::getEquidistantPoints() const {
    return equidistantPoints;
}

const ofPoint& svgSkeleton::getSvgCentroid() const {
    return svgCentroid;
}

void svgSkeleton::updateSvgCentroid() {
    calculateSvgCentroid();
    initialCentroid = svgCentroid; // Ensure the initialCentroid is also updated
}

const ofPoint& svgSkeleton::getInitialCentroid() const {
    return initialCentroid;
}
