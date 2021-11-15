#include "swIntersection.h"

swRay swIntersection::getShadowRay(const swVec3 &L) {
    return swRay(mPosition, L, 0.0f, 0.01f, FLT_MAX);
}

swRay swIntersection::getReflectedRay(void) {
    swVec3 N = mNormal;
    const swVec3 V = -mRay.dir;

    swVec3 R = 2 * (N * V) * N - V;
    
    swRay rray = swRay(mPosition, R, 0.0f, 0.01f, FLT_MAX);
    return rray;
}

swRay swIntersection::getRefractedRay(void) {
    const swVec3 &D = mRay.dir;
    swVec3 N = mNormal;
    swVec3 O; // O : outgoing refracted vector
    float eta = 1.0f / mMaterial.refractiveIndex;
    if (!mFrontFacing) eta = 1.0f / eta; // Inside material.

    // TODO: Implement reflection.
    // -------------------
    swVec3 R = D;
    // -------------------

    return swRay(mPosition, O, 0.0f, 0.01f, FLT_MAX);
}
