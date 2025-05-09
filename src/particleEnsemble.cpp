#include "particleEnsemble.h"
#include "attractor.h"  // Include the attractor class

particleEnsemble::particleEnsemble() {
    // Default constructor
}

void particleEnsemble::initialize(const std::vector<glm::vec3>& initialPositions) {
	ofDisableArbTex();
	ofEnableNormalizedTexCoords();

    // Exclude the first element (midpoint) and copy the rest of the positions
    positions.assign(initialPositions.begin() + 1, initialPositions.end());
    last_positions.assign(initialPositions.begin() + 1, initialPositions.end());
    
    v.resize(positions.size(), glm::vec3(0, 0, 0));
    f.resize(positions.size(), glm::vec3(0, 0, 0));
    last_f.resize(positions.size(), glm::vec3(0, 0, 0));
    radii.resize(positions.size(), 1.0f); // Example radius initialization
    masses.resize(positions.size(), 1.0f); // Example mass initialization

    // Load the particle texture    
	bool textureLoaded = texture.load("textures/particle2.png");
	if (!textureLoaded) {
		ofLogError("particleEnsemble") << "Failed to load particle texture!";
	}

    bool shaderLoaded = shader.load("shaders/particle");
    if (!shaderLoaded) {
        ofLogError("particleEnsemble") << "Failed to load particle shaders!";
	}

    // allocate memory for particle data
	positions.reserve(initialPositions.size());
}

void particleEnsemble::reinitialize(const std::vector<glm::vec3>& initialPositions) {
    // Exclude the first element (midpoint) and copy the rest of the positions
    positions.assign(initialPositions.begin() + 1, initialPositions.end());
    last_positions.assign(initialPositions.begin() + 1, initialPositions.end());

    std::fill(v.begin(), v.end(), glm::vec3(0, 0, 0));
    std::fill(f.begin(), f.end(), glm::vec3(0, 0, 0));
    std::fill(last_f.begin(), last_f.end(), glm::vec3(0, 0, 0));
}

void particleEnsemble::update(const std::vector<glm::vec3>& initialPositions) {
    // Exclude the first element (midpoint) and copy the rest of the positions
    positions.assign(initialPositions.begin() + 1, initialPositions.end());
    last_positions.assign(initialPositions.begin() + 1, initialPositions.end());
}

void particleEnsemble::draw() const {
	ofSetCircleResolution(5);
	ofPushMatrix();
    ofFill();
    for (size_t i = 0; i < positions.size(); ++i) {
        ofDrawCircle(positions[i], radii[i]);
		//ofdraw
    }
	ofPopMatrix();
}


void particleEnsemble::drawVBO() {
	vbo.clear();
	vbo.setVertexData(&positions[0].x, 3, positions.size(), GL_DYNAMIC_DRAW);

    ofEnableBlendMode(OF_BLENDMODE_ADD);

    //shaders for point sprite rendering
    texture.bind();
    shader.begin();

    // set uniforms
    shader.setUniform4f("color",
        1.0f,
        1.0f,
        1.0f,
        1.0f);

    shader.setUniform1f("size", 2.0f);

    // draw all particles in a single call
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    vbo.draw(GL_POINTS, 0, positions.size());

    glDisable(GL_POINT_SPRITE);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

    shader.end();
    texture.unbind();

    // reset blend mode
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}

void particleEnsemble::radial_update(float dt, float angularVelocity, const glm::vec3& midpoint) {
    for (auto& pos : positions) {
        float angle = angularVelocity * dt;
        float newX = cos(angle) * (pos.x - midpoint.x) - sin(angle) * (pos.y - midpoint.y) + midpoint.x;
        float newY = sin(angle) * (pos.x - midpoint.x) + cos(angle) * (pos.y - midpoint.y) + midpoint.y;
        pos.x = newX;
        pos.y = newY;
    }
}

// code to be added
/*
void particleEnsemble::vv_propagatePositionsVelocities(vector <attractor> attractorVec) {
    
    // define some iterators and the number of attractors
    int i, kk, nAttractors;
    // define timestep & new x,y,z positions, along with a 'factor'
    float dt, pxnew, pynew, pznew, factor;
    
    // get the openframeworks graphics window Width & Height
    float BoxWidth = ofGetWidth();
    float BoxHeight = ofGetHeight();

    // define data strutures to calculate the x,y,z forces
    float fx, fy, fz;
    glm::vec3 forces;
    
    nAttractors = int(attractorVec.size());
    
    //this loop uses verlet scheme (VV) to propagate the positions forward one step
    for (i = 0; i < particleVector.size(); ++i) {
        particleVector[i].setLast_x(particleVector[i].getx());
        particleVector[i].setLast_y(particleVector[i].gety());
        particleVector[i].setLast_z(particleVector[i].getz());
        
        factor = 0.5 * dt * dt / mass;
        pxnew = particleVector[i].getx() + dt * particleVector[i].getvx() + factor * particleVector[i].getfx();
        pynew = particleVector[i].gety() + dt * particleVector[i].getvy() + factor * particleVector[i].getfy();
        pznew = particleVector[i].getz() + dt * particleVector[i].getvz() + factor * particleVector[i].getfz();
        // carry out checks to reflect particles off the edges of the openframeworks graphics window in cases where they are travelling outside the screen space.
        // So long as they are within the screen space, we can carry out the normal update, updating the particle position to pxnew, pynew, pznew
        if (pxnew > 0 && pxnew < ofGetWidth()) {
            particleVector[i].setx(pxnew);
        }
        else {
            particleVector[i].setx(particleVector[i].getLast_x());
            particleVector[i].setvx(-1.0 * particleVector[i].getvx());
        }
        if (pynew > 0 && pynew < ofGetHeight()) {
            particleVector[i].sety(pynew);
        }
        else {
            particleVector[i].sety(particleVector[i].getLast_y());
            particleVector[i].setvy(-1.0 * particleVector[i].getvy());
        }
        if (pznew > 0 && pznew < 3000) {
            particleVector[i].sety(pznew);
        }
        else {
            particleVector[i].setz(particleVector[i].getLast_z());
            particleVector[i].setvz(-1.0 * particleVector[i].getvz());
        }
    }
    
    // save the present forces to Last_fx, Last_fy, Last_fz
    for (i = 0; i < numberOfParticles; ++i) {
        particleVector[i].setLast_fx(particleVector[i].getfx());
        particleVector[i].setLast_fy(particleVector[i].getfy());
        particleVector[i].setLast_fz(particleVector[i].getfz());
    }
    
    // zero out the force vectors & potential energy
    ZeroXForces();
    ZeroYForces();
    ZeroZForces();
        
    // loop through the vector containg all the particles
    for (i = 0; i < particleVector.size(); ++i) {
        // zero out the forces for each particle
        fx = 0.0;
        fy = 0.0;
        fz = 0.0;
        // loop through the attractors
        for (kk = 0; kk < nAttractors; ++kk) {
            forces = particleVector[i].calculateGaussianForce(attractorVec[kk]); // calculate the forces which an attractor exerts on a particle
            fx = fx + forces.x;
            fy = fy + forces.y;
            fz = fz + forces.z;
        }
        // update the x,y,z forces on each particle
        particleVector[i].setfx(fx);
        particleVector[i].setfy(fy);
        particleVector[i].setfz(fz);
    }
    // use Velocity Verlet scheme to propagate the velocities forward
    for (i = 0; i < particleVector.size(); ++i) {
        factor = dt * 0.5 / mass;
        particleVector[i].setvx(particleVector[i].getvx() + (particleVector[i].getfx() + particleVector[i].getLast_fx()) * factor);
        particleVector[i].setvy(particleVector[i].getvy() + (particleVector[i].getfy() + particleVector[i].getLast_fy()) * factor);
        particleVector[i].setvz(particleVector[i].getvz() + (particleVector[i].getfz() + particleVector[i].getLast_fz()) * factor);
    }
}
*/

void particleEnsemble::vv_propagatePositionsVelocities(const std::vector<attractor>& attractorVec, float dt) {
    float factor;
    float mass = 1.0f;  // Assuming mass is 1.0f for all particles

    // Update positions using the Velocity Verlet scheme
    factor = 0.5 * dt * dt / mass;
    for (size_t i = 0; i < positions.size(); ++i) {
        last_positions[i] = positions[i];
        
        positions[i].x = positions[i].x + dt * v[i].x + factor * f[i].x;
        positions[i].y = positions[i].y + dt * v[i].y + factor * f[i].y;
        positions[i].z = positions[i].z + dt * v[i].z + factor * f[i].z;

        // Reflect particles off the edges of the window
        if (positions[i].x < 0 || positions[i].x >= ofGetWidth()) {
            v[i].x *= -1.0;
            positions[i].x = ofClamp(positions[i].x, 0, ofGetWidth());
        }
        if (positions[i].y < 0 || positions[i].y >= ofGetHeight()) {
            v[i].y *= -1.0;
            positions[i].y = ofClamp(positions[i].y, 0, ofGetHeight());
        }
        if (positions[i].z < 0 || positions[i].z >= 3000) {  // Assuming depth limit is 3000
            v[i].z *= -1.0;
            positions[i].z = ofClamp(positions[i].z, 0, 3000);
        }
    }

    // Save the current forces to last_f
    last_f = f;

    // Zero out the current forces
    ZeroForces();

    // Calculate forces acting on each particle due to attractors
    for (size_t i = 0; i < positions.size(); ++i) {
        glm::vec3 totalForce(0.0f);
        for (const auto& attractor : attractorVec) {
            glm::vec3 force = calculateGaussianForce(attractor, positions[i]);
            totalForce += force;
        }
        f[i] = totalForce;
    }

    // Update velocities using the Velocity Verlet scheme
    factor = dt * 0.5 / mass;
    for (size_t i = 0; i < v.size(); ++i) {
        v[i].x += (f[i].x + last_f[i].x) * factor;
        v[i].y += (f[i].y + last_f[i].y) * factor;
        v[i].z += (f[i].z + last_f[i].z) * factor;
    }
}

glm::vec3 particleEnsemble::calculateGaussianForce(const attractor& attractorObject, const glm::vec3& particlePosition) const {
    float dx = particlePosition.x - attractorObject.getCenter().x;
    float dy = particlePosition.y - attractorObject.getCenter().y;
    float dz = particlePosition.z - attractorObject.getCenter().z;

    float exp_numerator = dx * dx + dy * dy + dz * dz;
    float prefactor = -1.0 * attractorObject.get_coefficient() * exp(exp_numerator / attractorObject.get_exp_denominator());

    glm::vec3 forceVector;
    forceVector.x = prefactor * dx;
    forceVector.y = prefactor * dy;
    forceVector.z = prefactor * dz;

    return forceVector;
}

