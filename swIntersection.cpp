#include "swIntersection.h"
#include <math.h>

swRay swIntersection::getShadowRay(const swVec3 &L) {
    return swRay(mPosition, L, 0.0f, 0.01f, FLT_MAX);
}

swRay swIntersection::getReflectedRay(void) {
    swVec3 N = mNormal;
    const swVec3 V = -mRay.dir;

    swVec3 R = (2 * (N * V) * N) - V;
    
    swRay rray = swRay(mPosition, R, 0.0f, 0.01f, FLT_MAX);
    return rray;
}

swRay swIntersection::getRefractedRay(void) {
    const swVec3 &D = mRay.dir;
    swVec3 N = mNormal;
    swVec3 O; // O : outgoing refracted vector
    float eta = 1.0f / mMaterial.refractiveIndex;
    if (!mFrontFacing) eta = 1.0f / eta; // Inside material.

    float r = -D * N;
    float c = 1.0f - powf(eta, 2.0f) * (1.0f - powf(r, 2.0f));
    if(c < 0.0f) {
        return getReflectedRay();
    }
    O = eta * D + ((eta * r) - sqrtf(c)) * N;

    return swRay(mPosition, O, 0.0f, 0.01f, FLT_MAX);
}
