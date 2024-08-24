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
    calculateSvgCentroid();
    translation.set(0, 0);
    cumulativeScale = 1.0f;
    initialCentroid = svgCentroid;
    crossSize = 1.05f;
    currentRotationAngle = PI / 4.0f; // 45 degrees counterclockwise from vertical
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

void svgSkeleton::resizeSvg(float scaleFactor, bool loadingSvg) {

    if(loadingSvg){cumulativeScale = scaleFactor;}
    else{cumulativeScale *= scaleFactor;}

    for (auto& point : equidistantPoints) {
        point = svgCentroid + (point - svgCentroid) * scaleFactor;
    }
    // After resizing, recalculate the centroid
    calculateSvgCentroid();
    // Adjust the cross size dynamically after resizing
    calculateCrossSize();
}

void svgSkeleton::rotateSvg(float angleDelta, bool loadingSvg) {
    ofPoint centroid = getSvgCentroid();

    // Rotate each point around the centroid by angleDelta
    for (auto& point : equidistantPoints) {
        float s = sin(angleDelta);
        float c = cos(angleDelta);

        // Translate point to origin (centroid)
        point.x -= centroid.x;
        point.y -= centroid.y;

        // Rotate point
        float newX = point.x * c - point.y * s;
        float newY = point.x * s + point.y * c;

        // Translate point back
        point.x = newX + centroid.x;
        point.y = newY + centroid.y;
    }
    
    if(loadingSvg){currentRotationAngle = angleDelta;}
    else{currentRotationAngle += angleDelta;}
    
    // Wrap the current rotation angle to ensure it stays within [0, 2*PI]
    currentRotationAngle = fmod(currentRotationAngle, TWO_PI);
    if (currentRotationAngle < 0) {
        currentRotationAngle += TWO_PI;  // Ensure the angle is positive
    }
}

void svgSkeleton::draw() const {
    if (!equidistantPoints.empty()) {
        // Draw the SVG points as circles with the default SVG points color
        for (size_t i = 1; i < equidistantPoints.size(); ++i) {
            ofDrawCircle(equidistantPoints[i], 1); // Draw small circles at each point
        }
        
        // Calculate cross size based on the farthest points in x and y directions
        float crossSizeX, crossSizeY;
        calculateMaxDistances(crossSizeX, crossSizeY);

        // Increase the cross size by 10%
        crossSizeX *= crossSize;
        crossSizeY *= crossSize;


        // Draw the cross as dashed lines
        float dashLength = 5.0f; // Length of each dash
        float gapLength = 3.0f;  // Length of the gap between dashes
        
        const auto& centroid = equidistantPoints[0];
        
        // Draw horizontal dashed line
        for (float x = centroid.x - crossSizeX; x < centroid.x + crossSizeX; x += dashLength + gapLength) {
            ofDrawLine(x, centroid.y, std::min(x + dashLength, centroid.x + crossSizeX), centroid.y);
        }

        // Draw vertical dashed line
        for (float y = centroid.y - crossSizeY; y < centroid.y + crossSizeY; y += dashLength + gapLength) {
            ofDrawLine(centroid.x, y, centroid.x, std::min(y + dashLength, centroid.y + crossSizeY));
        }
        
        // Draw the scaling handles as circles
        for (const auto& handle : getScalingHandlePositions()) {
            ofNoFill();
            ofDrawCircle(handle, 5); // Draw circles at each handle
        }

        // Draw the rotational handle as a dashed line
        ofPoint rotationHandle = getRotationalHandlePosition();
        ofPoint rotationLineStart = svgCentroid;

        float rotationLineLength = rotationHandle.distance(rotationLineStart);

        for (float dist = 0; dist < rotationLineLength; dist += dashLength + gapLength) {
            ofPoint start = rotationLineStart + dist * (rotationHandle - rotationLineStart) / rotationLineLength;
            ofPoint end = start + std::min(dashLength, rotationLineLength - dist) * (rotationHandle - rotationLineStart) / rotationLineLength;
            ofDrawLine(start, end);
        }
        
        // Draw the rotational handle circle
        ofDrawCircle(rotationHandle, 10); // Draw the rotational handle as a circle
    }
    /*
    if (!equidistantPoints.empty()) {
        
        // Draw the SVG points as circles with the default SVG points color
//        ofSetColor(255);  // Reset to default color (white or other)
        for (size_t i = 1; i < equidistantPoints.size(); ++i) {
            ofDrawCircle(equidistantPoints[i], 1); // Draw small circles at each point
        }

        // Calculate cross size based on the farthest points in x and y directions
        float crossSizeX, crossSizeY;
        calculateMaxDistances(crossSizeX, crossSizeY);

        // Increase the cross size by 10%
        crossSizeX *= crossSize;
        crossSizeY *= crossSize;

        // now draw the cross
//        ofSetColor(200, 100, 255);  // Light purple color for the cross
        float dashLength = 5.0f; // Length of each dash
        float gapLength = 3.0f;  // Length of the gap between dashes
        
        const auto& centroid = equidistantPoints[0];
        
        // Draw horizontal dashed line
        for (float x = centroid.x - crossSizeX; x < centroid.x + crossSizeX; x += dashLength + gapLength) {
            ofDrawLine(x, centroid.y, std::min(x + dashLength, centroid.x + crossSizeX), centroid.y);
        }

        // Draw vertical dashed line
        for (float y = centroid.y - crossSizeY; y < centroid.y + crossSizeY; y += dashLength + gapLength) {
            ofDrawLine(centroid.x, y, centroid.x, std::min(y + dashLength, centroid.y + crossSizeY));
        }

        // Draw circles at the endpoints of the cross
        ofNoFill();
        ofDrawCircle(centroid.x - crossSizeX, centroid.y, 5.0f);
        ofDrawCircle(centroid.x + crossSizeX, centroid.y, 5.0f);
        ofDrawCircle(centroid.x, centroid.y - crossSizeY, 5.0f);
        ofDrawCircle(centroid.x, centroid.y + crossSizeY, 5.0f);
        ofFill();
    }
    */
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

void svgSkeleton::autoFitToWindow(int windowWidth, int windowHeight) {
    float svgWidth = svg.getWidth();
    float svgHeight = svg.getHeight();
    float scaleX = static_cast<float>(windowWidth) / svgWidth;
    float scaleY = static_cast<float>(windowHeight) / svgHeight;
    float scale = std::min(scaleX, scaleY) * 0.9f; // Scale down slightly to fit within window

    // Center the SVG
    ofPoint newCentroid = ofPoint(windowWidth / 2, windowHeight / 2);
    ofPoint offset = newCentroid - svgCentroid;
    translateSvg(offset);
    resizeSvg(scale,false);
    updateSvgCentroid();
}

void svgSkeleton::calculateMaxDistances(float& maxDistanceX, float& maxDistanceY) const {
    maxDistanceX = 0.0f;
    maxDistanceY = 0.0f;

    if (equidistantPoints.empty()) return;

    const auto& centroid = equidistantPoints[0];

    for (size_t i = 1; i < equidistantPoints.size(); ++i) {
        float distanceX = std::abs(equidistantPoints[i].x - centroid.x);
        float distanceY = std::abs(equidistantPoints[i].y - centroid.y);

        if (distanceX > maxDistanceX) {
            maxDistanceX = distanceX;
        }
        if (distanceY > maxDistanceY) {
            maxDistanceY = distanceY;
        }
    }
}

void svgSkeleton::calculateCrossSize() {
    float crossSizeX, crossSizeY;
    calculateMaxDistances(crossSizeX, crossSizeY);

    // Increase the cross size by 10%
    crossSizeX *= crossSize;
    crossSizeY *= crossSize;
}

bool svgSkeleton::isNearCentroid(const ofPoint& point, float threshold) const {
    return point.distance(svgCentroid) <= threshold;
}

bool svgSkeleton::isNearCrossEndPoints(const ofPoint& point) const {
    float crossSizeX, crossSizeY;
    calculateMaxDistances(crossSizeX, crossSizeY);

    // Increase the cross size
    crossSizeX *= crossSize;
    crossSizeY *= crossSize;

    // Define the four endpoints of the cross
    ofPoint endPoint1(svgCentroid.x - crossSizeX, svgCentroid.y);
    ofPoint endPoint2(svgCentroid.x + crossSizeX, svgCentroid.y);
    ofPoint endPoint3(svgCentroid.x, svgCentroid.y - crossSizeY);
    ofPoint endPoint4(svgCentroid.x, svgCentroid.y + crossSizeY);

    float threshold = 10.0f;  // Define the distance threshold for snapping

    // Check if the point is near any of the cross endpoints
    return (point.distance(endPoint1) <= threshold ||
            point.distance(endPoint2) <= threshold ||
            point.distance(endPoint3) <= threshold ||
            point.distance(endPoint4) <= threshold);
}

std::vector<ofPoint> svgSkeleton::getScalingHandlePositions() const {
    // Calculate cross size based on the farthest points in x and y directions
    float crossSizeX, crossSizeY;
    calculateMaxDistances(crossSizeX, crossSizeY);

    // Increase the cross size by 10%
    crossSizeX *= crossSize;
    crossSizeY *= crossSize;

    std::vector<ofPoint> handles = {
        ofPoint(svgCentroid.x + crossSizeX, svgCentroid.y),  // Right
        ofPoint(svgCentroid.x - crossSizeX, svgCentroid.y),  // Left
        ofPoint(svgCentroid.x, svgCentroid.y + crossSizeY),  // Bottom
        ofPoint(svgCentroid.x, svgCentroid.y - crossSizeY)   // Top
    };

    return handles;
}

ofPoint svgSkeleton::getRotationalHandlePosition() const {
    
    // Calculate cross size based on the farthest points in x and y directions
    float crossSizeX, crossSizeY;
    calculateMaxDistances(crossSizeX, crossSizeY);

    // Increase the cross size by 10%
    crossSizeX *= crossSize*1.1;
    crossSizeY *= crossSize*1.1;
    
    float crossDims = std::max(crossSizeX, crossSizeY);
    float angle = currentRotationAngle;

    ofPoint rotationHandle(
        svgCentroid.x + crossDims * cos(angle),
        svgCentroid.y + crossDims * sin(angle)
    );

    return rotationHandle;
}

bool svgSkeleton::isNearRotationalHandle(const ofPoint& mousePos) const {
    ofPoint rotationHandle = getRotationalHandlePosition();
    return mousePos.distance(rotationHandle) <= 10.0f; // Adjust the threshold as needed
}
