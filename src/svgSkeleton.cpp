#include "svgSkeleton.h"
#include <cmath>
#include <glm/vec3.hpp>
#include "ofxXmlSettings.h"


void svgSkeleton::loadSvg(const std::string& filename) {
    
    ofFile file(filename);
    if (!file.exists()) {
        ofSystemAlertDialog("Error: SVG file not found.\nPlease make sure the file exists and the path is correct.");
        return;
    }
    
    fileName = filename; // Store the file name
    
    ofXml xml;
    if (xml.load(filename)) {
        polyLineLabels.clear();

        // Ensure we navigate to the <svg> element
        auto svgElement = xml.findFirst("//svg");
        if (svgElement) {
            // Find all elements within the SVG that have an 'id' attribute
            auto allElementsWithId = svgElement.find("//*[@id]");  // Select any element with an 'id' attribute
            for (auto& element : allElementsWithId) {
                // Check if this element is part of a commented block
                std::string elementContent = element.toString();
                if (elementContent.find("<!--") == std::string::npos && elementContent.find("-->") == std::string::npos) {
                    std::string id = element.getAttribute("id").getValue();

                    // Only store IDs that contain the substring 'path'
                    if (id.find("path") != std::string::npos) {
                        polyLineLabels.push_back(id);  // Store the ID in the polyLineLabels vector
                    }
                }
            }
        } else {
            ofLogError() << "No <svg> element found in the file: " << filename;
        }
    } else {
        ofLogError() << "Failed to load SVG file: " << filename;
    }
    
    svg.load(filename);
    translation.set(0, 0);
    cumulativeScale = 1.0f;
    svgMidpoint.set(0, 0);
    crossSizeScaleFactor = 1.05f;
    currentRotationAngle = 0.0f; // 45 degrees counterclockwise from vertical
}

void svgSkeleton::generateEquidistantPoints(int numDesiredPoints) {
    float totalPathLength = 0;
    std::vector<std::pair<ofPolyline, float>> polylinesWithLengths;
    std::vector<glm::vec3> vertices;

    equidistantPoints.clear();  // Clear previous points
    equidistantPointsPathIDs.clear();  // Clear previous path IDs
    pathVertices.clear();  // Clear previous path vertices data
//    pointsByPath.clear();  // Clear any existing pointsByPath data

    // Step 1: Calculate the total length of all paths and identify vertices
    int numPaths = svg.getNumPath();
    for (int i = 0; i < numPaths; i++) {
        ofPath path = svg.getPathAt(i);
        path.setPolyWindingMode(OF_POLY_WINDING_ODD);  // Ensure proper winding mode

        auto polylines = path.getOutline();
        for (auto& polyline : polylines) {
            float pathLength = polyline.getPerimeter();
            totalPathLength += pathLength;
            polylinesWithLengths.emplace_back(polyline, pathLength);
            
            // Prepare a vector to store vertices for the current polyline
            std::vector<glm::vec3> currentPathVertices;
            std::vector<int> currentPathVerticesIndices;

            // Identify vertices and order points
            auto points = polyline.getVertices();
            for (size_t j = 0; j < points.size(); ++j) {
                glm::vec3 vertex(points[j].x, points[j].y, points[j].z);
//                cout << "points[j].x : "  << points[j].x << " points[j].y "  << points[j].y << endl;

                // First and last points are always vertices
                if (j == 0 || j == points.size() - 1) {
                    vertices.push_back(vertex);
                    currentPathVertices.push_back(vertex);
                    currentPathVerticesIndices.push_back(j);
                } else {
                    // Calculate the angle between adjacent segments
                    glm::vec3 prev(points[j - 1].x - points[j].x, points[j - 1].y - points[j].y, points[j - 1].z - points[j].z);
                    glm::vec3 next(points[j + 1].x - points[j].x, points[j + 1].y - points[j].y, points[j + 1].z - points[j].z);
                    float angle = std::acos(glm::dot(glm::normalize(prev), glm::normalize(next)));

                    // If angle is sharp, consider it a vertex
                    if (angle < glm::radians(170.0f)) {  // Threshold angle can be adjusted
                        vertices.push_back(vertex);
                        currentPathVertices.push_back(vertex);
                        currentPathVerticesIndices.push_back(j);
                    }
                }
            }
            // if the polyline is a closed path, ensure that the last vertex is identical to the first one
            size_t lastIdx = currentPathVertices.size() - 1;
            if(polyline.isClosed() && (currentPathVertices[lastIdx] != currentPathVertices[0])){
                currentPathVertices.push_back(currentPathVertices[0]);
            }
            pathVertices.push_back(currentPathVertices); // Store vertices of the current polyline in pathVertices
            pathVerticesIndices.push_back(currentPathVerticesIndices);
        }
    }

    // Calculate the remaining points needed after vertices are accounted for
    int remainingPoints = numDesiredPoints - static_cast<int>(vertices.size()) - 1; // -1 to avoid counting centroid
    if (remainingPoints <= 0) {
        // If remaining points are zero or less, just return the vertices
        equidistantPoints.insert(equidistantPoints.end(), vertices.begin(), vertices.end());
        for (size_t i = 0; i < vertices.size(); ++i) {
            equidistantPointsPathIDs.push_back(polyLineLabels[0]);  // Assuming all vertices belong to the first path, adjust as needed
        }
    }
    else{
        
        float segmentLength = totalPathLength / remainingPoints;
        
        for (size_t i = 0; i < polylinesWithLengths.size(); ++i) {
            ofPolyline& polyline = polylinesWithLengths[i].first;
            auto& polylineVertices = pathVertices[i]; // get vertices from the pathVertices
            
            float lengthAlongPath(0.0);
            float lengthStep;
            std::vector<int> currentPathVerticesIndices = pathVerticesIndices[i];
            
            for(size_t ii=0; ii< (polylineVertices.size() - 1); ++ii){
                
                glm::vec3 startVertex = polylineVertices[ii];
                
                if (ii > 0){
                    size_t maxIdx = equidistantPoints.size() - 1;
                    lengthAlongPath = lengthAlongPath + glm::distance(startVertex,equidistantPoints[maxIdx]);
                }
                
                equidistantPoints.push_back(startVertex);
                equidistantPointsPathIDs.push_back(polyLineLabels[i]);
//                size_t idx = equidistantPoints.size()-1;
//                cout << "vertex-x: "  << equidistantPoints[idx].x << " vertex-y: "  << equidistantPoints[idx].y << endl;
                float pathLength;
                
                if(polylineVertices[ii] != polylineVertices[ii+1]){
//                    pathLength = glm::distance(polylineVertices[ii], polylineVertices[ii+1]);
                    int idx1 = currentPathVerticesIndices[ii];
                    int idx2 = currentPathVerticesIndices[ii+1];
                    float lengthAtIndex1 = polyline.getLengthAtIndex(idx1);
                    float lengthAtIndex2 = polyline.getLengthAtIndex(idx2);
                    pathLength = abs(lengthAtIndex2 - lengthAtIndex1);
                }else{
                    pathLength = polyline.getPerimeter();
                }
                
                // Determine the number of points for this path segment
                int numPointsForPath = std::round(pathLength / segmentLength);
                
                // Ensure points are distributed approximately equidistantly
                lengthStep = pathLength / (numPointsForPath + 1); // add 1 cuz number of segments is numPointsForPath+1
                
                // now loop over the points between vertices
                for (int j = 1; j <= numPointsForPath; j++) {
                    lengthAlongPath = lengthAlongPath + lengthStep;
                    glm::vec3 point = polyline.getPointAtLength(lengthAlongPath);
                    equidistantPoints.push_back(point);
                    equidistantPointsPathIDs.push_back(polyLineLabels[i]);
//                    idx = equidistantPoints.size()-1;
//                    cout << " j " << j << " point-x: "  << equidistantPoints[idx].x << " point-y: "  << equidistantPoints[idx].y << endl;
                }
            }
            
            glm::vec3 endVertex = polylineVertices[polylineVertices.size() - 1];
            equidistantPoints.push_back(endVertex);
            equidistantPointsPathIDs.push_back(polyLineLabels[i]);
//            size_t idx = equidistantPoints.size()-1;
//            cout << "vertex-x: "  << equidistantPoints[idx].x << " vertex-y: "  << equidistantPoints[idx].y << endl;
            size_t lastIdx = equidistantPoints.size()-1;
            float distFirstToLast = glm::distance(equidistantPoints[lastIdx], equidistantPoints[0]);
                        
        }
    }
    
    // Apply the stored translation and scale to the newly generated points
    calculateSvgMidpoint();
    glm::vec3 midpoint(svgMidpoint.x, svgMidpoint.y, svgMidpoint.z);
    
    // add the midpoint as the final element in equidistantPoints
    equidistantPoints.insert(equidistantPoints.begin(), midpoint);
    equidistantPointsPathIDs.insert(equidistantPointsPathIDs.begin(), "midpoint");

    for (auto& point : equidistantPoints) {
        point = svgMidpoint + (point - svgMidpoint) * cumulativeScale + translation;
    }
}

void svgSkeleton::autoFitToWindow(int windowWidth, int windowHeight) {
    float svgWidth = svg.getWidth();
    float svgHeight = svg.getHeight();
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

void svgSkeleton::writeSvg(const std::vector<glm::vec3>& particlePositions) {
    // Ensure the vectors are valid and particlePositions has one less element than equidistantPoints
    if (particlePositions.empty() || equidistantPoints.size() <= 1 || equidistantPointsPathIDs.size() <= 1) return;

    // Get the current timestamp for the filename
    std::string timestamp = ofGetTimestampString("%Y-%m-%d_%H-%M-%S");
    std::string filename = "output_" + timestamp + ".svg";

    std::ofstream svgFile;
    svgFile.open(ofToDataPath(filename));

    if (svgFile.is_open()) {
        // Write the SVG header
        svgFile << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
        svgFile << "<!-- Created with custom OpenFrameworks app -->\n";
        svgFile << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";

        // Map to store path points by path ID
        std::map<std::string, std::vector<glm::vec3>> pointsByPath;

        // Group the particle positions by their path ID, starting from index 1
        for (size_t i = 0; i < particlePositions.size(); ++i) {
            const std::string& pathID = equidistantPointsPathIDs[i + 1];  // Skip the midpoint (index 0)
            pointsByPath[pathID].push_back(particlePositions[i]);
        }

        // Write each path's points
        for (const auto& pathPair : pointsByPath) {
            const std::string& pathId = pathPair.first;
            const std::vector<glm::vec3>& pathPoints = pathPair.second;

            svgFile << "<path id=\"" << pathId << "\" style=\"fill:none;stroke:#030303;stroke-width:0.3\" d=\"";
            svgFile << "M ";

            for (size_t i = 0; i < pathPoints.size(); ++i) {
                const glm::vec3& point = pathPoints[i];
                svgFile << point.x << " " << point.y;
                if (i != pathPoints.size() - 1) {
                    svgFile << " L ";
                }
            }

            svgFile << "\" />\n";
        }

        // Write the SVG footer
        svgFile << "</svg>\n";
        svgFile.close();
        ofLogNotice() << "Particle positions written to " << filename;
    } else {
        ofLogError() << "Unable to open file for writing: " << filename;
    }
}
