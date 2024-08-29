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
    translation.set(0, 0);
    cumulativeScale = 1.0f;
    referenceOrigin.set(0, 0);
    svgMidpoint.set(0, 0);
    crossSizeScaleFactor = 1.05f;
    currentRotationAngle = 0.0f; // 45 degrees counterclockwise from vertical
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
    int remainingPoints = numDesiredPoints - static_cast<int>(numVertices) + 1;  // add 1 for the midpoint
    
    equidistantPoints.clear();  // Clear points vector before generating
    equidistantPoints.reserve(numDesiredPoints + 1); // Reserve space for the total number of points + midpoint
    
    // Step 2: Add vertices to equidistantPoints
    equidistantPoints.insert(equidistantPoints.end(), vertices.begin(), vertices.end());
    
    if (remainingPoints <= 0) {
        // Step 4: Calculate the midpoint
        calculateSvgMidpoint();
        glm::vec3 midpoint(svgMidpoint.x, svgMidpoint.y, svgMidpoint.z);
        
        // Step 5: Add the midpoint to equidistantPoints
        equidistantPoints.insert(equidistantPoints.begin(), midpoint); // Ensure midpoint is the first point
        
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
        
    // Step 4: Calculate the midpoint
    calculateSvgMidpoint();
    glm::vec3 midpoint(svgMidpoint.x, svgMidpoint.y, svgMidpoint.z);
    
    // Step 5: Add the midpoint to equidistantPoints
    equidistantPoints.insert(equidistantPoints.begin(), midpoint); // Ensure midpoint is the first point
    

    // Apply the stored translation and scale to the newly generated points
    for (auto& point : equidistantPoints) {
        point = svgMidpoint + (point - svgMidpoint) * cumulativeScale + translation;
    }
}

void svgSkeleton::autoFitToWindow(int windowWidth, int windowHeight) {
    float svgWidth = svg.getWidth();
    float svgHeight = svg.getHeight();
    referenceOrigin = ofPoint(svg.getWidth()/2, svg.getHeight()/2);
    float scaleX = static_cast<float>(windowWidth) / svgWidth;
    float scaleY = static_cast<float>(windowHeight) / svgHeight;
    float scale = std::min(scaleX, scaleY) * 0.9f; // Scale down slightly to fit within window

    // Center the SVG
    ofPoint newCentroid = ofPoint(windowWidth / 2, windowHeight / 2);
    ofPoint offset = newCentroid - svgMidpoint;
    translateSvg(offset);
    resizeSvg(scale,false);

    calculateSvgMidpoint();
}

void svgSkeleton::calculateSvgMidpoint() {
    
    if (equidistantPoints.empty()) return;

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (const auto& point : equidistantPoints) {
        if (point.x < minX) minX = point.x;
        if (point.x > maxX) maxX = point.x;
        if (point.y < minY) minY = point.y;
        if (point.y > maxY) maxY = point.y;
    }

    // Calculate the midpoint
    svgMidpoint.x = (minX + maxX) / 2.0f;
    svgMidpoint.y = (minY + maxY) / 2.0f;
}

void svgSkeleton::translateSvg(const ofPoint& offset) {
    translation += offset;
    for (auto& point : equidistantPoints) {
        point += offset;
    }
    svgMidpoint += offset;
//    referenceOrigin += offset; // Ensure the referenceOrigin is also updated
}

void svgSkeleton::resizeSvg(float scaleFactor, bool loadingSvg) {

    if(loadingSvg){cumulativeScale = scaleFactor;}
    else{cumulativeScale *= scaleFactor;}

    for (auto& point : equidistantPoints) {
        point = svgMidpoint + (point - svgMidpoint) * scaleFactor;
    }
    // After resizing, recalculate the midpoint
    calculateSvgMidpoint();
    // Adjust the cross size dynamically after resizing
    calculateAdjustedCrossSize();
}

void svgSkeleton::rotateSvg(float angleDelta, bool loadingSvg) {
    ofPoint midpoint = getSvgCentroid();

    // Rotate each point around the midpoint by angleDelta
    for (auto& point : equidistantPoints) {
        float s = sin(angleDelta);
        float c = cos(angleDelta);

        // Translate point to origin (midpoint)
        point.x -= midpoint.x;
        point.y -= midpoint.y;

        // Rotate point
        float newX = point.x * c - point.y * s;
        float newY = point.x * s + point.y * c;

        // Translate point back
        point.x = newX + midpoint.x;
        point.y = newY + midpoint.y;
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
        
        float dashLength = 5.0f; // Length of each dash
        float gapLength = 3.0f;  // Length of the gap between dashes
        
        const auto& midpoint = equidistantPoints[0];
        
        // Draw horizontal dashed line
        for (float x = midpoint.x - crossSizeX; x < midpoint.x + crossSizeX; x += dashLength + gapLength) {
            ofDrawLine(x, midpoint.y, std::min(x + dashLength, midpoint.x + crossSizeX), midpoint.y);
        }

        // Draw vertical dashed line
        for (float y = midpoint.y - crossSizeY; y < midpoint.y + crossSizeY; y += dashLength + gapLength) {
            ofDrawLine(midpoint.x, y, midpoint.x, std::min(y + dashLength, midpoint.y + crossSizeY));
        }
        
        // Draw the scaling handles as circles
        for (const auto& handle : getScalingHandlePositions()) {
            ofNoFill();
            ofDrawCircle(handle, 5); // Draw circles at each handle
        }

        // Draw the rotational handle as a dashed line
        ofPoint rotationHandle = getRotationalHandlePosition();
        ofPoint rotationLineStart = svgMidpoint;

        float rotationLineLength = rotationHandle.distance(rotationLineStart);

        for (float dist = 0; dist < rotationLineLength; dist += dashLength + gapLength) {
            ofPoint start = rotationLineStart + dist * (rotationHandle - rotationLineStart) / rotationLineLength;
            ofPoint end = start + std::min(dashLength, rotationLineLength - dist) * (rotationHandle - rotationLineStart) / rotationLineLength;
            ofDrawLine(start, end);
        }
        
        // Draw the rotational handle circles
//        ofDrawCircle(rotationHandle, 10); // Draw the rotational handle as a circle
//        ofDrawCircle(rotationHandle, 5); // Draw the rotational handle as a circle
        drawDottedCircle(rotationHandle, 12, 5, 3); // Adjust dotLength and gapLength as needed
        drawDottedCircle(rotationHandle, 5, 3, 2);
    }
}

const std::vector<glm::vec3>& svgSkeleton::getEquidistantPoints() const {
    return equidistantPoints;
}

const ofPoint& svgSkeleton::getSvgCentroid() const {
    return svgMidpoint;
}

const ofPoint& svgSkeleton::getInitialCentroid() const {
    return referenceOrigin;
}

bool svgSkeleton::isNearCentroid(const ofPoint& point, float threshold) const {
    return point.distance(svgMidpoint) <= threshold;
}

bool svgSkeleton::isNearCrossEndPoints(const ofPoint& point) const {

    ofPoint endPoint1(svgMidpoint.x - crossSizeX, svgMidpoint.y);
    ofPoint endPoint2(svgMidpoint.x + crossSizeX, svgMidpoint.y);
    ofPoint endPoint3(svgMidpoint.x, svgMidpoint.y - crossSizeY);
    ofPoint endPoint4(svgMidpoint.x, svgMidpoint.y + crossSizeY);

    float threshold = 10.0f;  // Define the distance threshold for snapping

    // Check if the point is near any of the cross endpoints
    return (point.distance(endPoint1) <= threshold ||
            point.distance(endPoint2) <= threshold ||
            point.distance(endPoint3) <= threshold ||
            point.distance(endPoint4) <= threshold);
}

std::vector<ofPoint> svgSkeleton::getScalingHandlePositions() const {

    std::vector<ofPoint> handles = {
        ofPoint(svgMidpoint.x + crossSizeX, svgMidpoint.y),  // Right
        ofPoint(svgMidpoint.x - crossSizeX, svgMidpoint.y),  // Left
        ofPoint(svgMidpoint.x, svgMidpoint.y + crossSizeY),  // Bottom
        ofPoint(svgMidpoint.x, svgMidpoint.y - crossSizeY)   // Top
    };

    return handles;
}

ofPoint svgSkeleton::getRotationalHandlePosition() const {
    
    float crossDims = std::max(crossSizeX, crossSizeY);
    float angle = currentRotationAngle;

    ofPoint rotationHandle(
        svgMidpoint.x + crossDims * cos(angle),
        svgMidpoint.y + crossDims * sin(angle)
    );

    return rotationHandle;
}

bool svgSkeleton::isNearRotationalHandle(const ofPoint& mousePos) const {
    ofPoint rotationHandle = getRotationalHandlePosition();
    return mousePos.distance(rotationHandle) <= 10.0f; // Adjust the threshold as needed
}

void svgSkeleton::drawDottedCircle(const ofPoint& center, float radius, float dotLength, float gapLength) const {
    int numSegments = 36; // Number of segments to approximate the circle
    float angleStep = TWO_PI / numSegments; // Step size for each segment

    for (int i = 0; i < numSegments; i++) {
        float angle1 = i * angleStep;
        float angle2 = angle1 + angleStep;

        // Calculate the start and end points of each segment
        ofPoint p1(center.x + radius * cos(angle1), center.y + radius * sin(angle1));
        ofPoint p2(center.x + radius * cos(angle2), center.y + radius * sin(angle2));

        // Draw a short line segment and then a gap to create a dotted effect
        if (i % 2 == 0) {
            ofDrawLine(p1, p2);
        }
    }
}

void svgSkeleton::calculateAdjustedCrossSize() {

    if (equidistantPoints.empty()) return;
    
    float maxDistance = 0.0f;
    const auto& midpoint = getSvgCentroid();
    
    for (const auto& point : equidistantPoints) {
        float distance = midpoint.distance(point);
        if (distance > maxDistance) {
            maxDistance = distance;
        }
    }
    
    crossSizeX = maxDistance * crossSizeScaleFactor;
    crossSizeY = maxDistance * crossSizeScaleFactor;
    
}

