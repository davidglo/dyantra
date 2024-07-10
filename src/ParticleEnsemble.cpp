#include "ParticleEnsemble.h"

void ParticleEnsemble::draw() const {
    ofFill();
    for (size_t i = 0; i < positions.size(); ++i) {
        ofDrawCircle(positions[i], radii[i]);
    }
}
