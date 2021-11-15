/*
 *  main.cpp
 *  swTracer
 *
 *  Created by Michael Doggett on 2021-09-23.
 *  Copyright (c) 2021 Michael Doggett
 */
#define _USE_MATH_DEFINES
#include <cfloat>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "swCamera.h"
#include "swIntersection.h"
#include "swMaterial.h"
#include "swRay.h"
#include "swScene.h"
#include "swSphere.h"
#include "swVec3.h"

inline float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

float uniform() {
    // Will be used to obtain a seed for the random number engine
    static std::random_device rd;
    // Standard mersenne_twister_engine seeded with rd()
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    return dis(gen);
}

void WriteColor(int index, swVec3 p, uint8_t *pixels) {
    // gamma correct for gamma=2.2, x^(1/gamma), more see :
    // https://www.geeks3d.com/20101001/tutorial-gamma-correction-a-story-of-linearity/
    for (int n = 0; n < 3; n++) {
        p.m[n] = pow(p.m[n], 1.0f / 2.2f);
        pixels[index + n] = (uint8_t)(256 * clamp(p.m[n], 0.0f, 0.999f));
    }
}

Color traceRay(const swRay &r, swScene scene, int depth) {
    Color c, directColor;
    if (depth < 0) return c;

    swIntersection hp, si;
    if (!scene.intersect(r, hp)) return Color(0, 0, 0); // Background color

    const swVec3 gLightPos(20, 240, -7);
    swVec3 lightDir = gLightPos - hp.mPosition;
    lightDir.normalize();

    // Task 1 - Diffuse shading
    if(!scene.intersect(hp.getShadowRay(lightDir), si)) {
        c = hp.mMaterial.mColor * (hp.mNormal * lightDir);
    }
    // Task 2 - Shadows
    else {
        c = Color(0, 0, 0);
    }

    float reflectivity = clamp(hp.mMaterial.reflectivity, 0.0f, 1.0f);
    float transparency = clamp(hp.mMaterial.transparency, 0.0f, 1.0f);
    if(depth > 0) {
        c = clamp((1.0f - reflectivity - transparency), 0.0f, 1.0f) * c;
        // Task 3 - Reflection
        if(reflectivity > 0){
            c += reflectivity * traceRay(hp.getReflectedRay(), scene, depth - 1);
        }
        // Task 5 - Refraction
        if(transparency > 0){
            c += transparency * traceRay(hp.getRefractedRay(), scene, depth - 1);
        }
    }

    return c;
}

int main() {
    int imageWidth = 1028;
    int imageHeight = imageWidth;
    const int numChannels = 3;
    uint8_t *pixels = new uint8_t[imageWidth * imageHeight * numChannels];

    // Setup scene
    swScene scene;
    swMaterial mat[4];
    // swMaterial(swVec3 color, reflectivity, refractivity, refractiveIndex)
    mat[0] = swMaterial(swVec3(0.0f, 0.2f, 0.9f), 0.0f, 0.7f, 1.01f);
    mat[1] = swMaterial(swVec3(1.0f, 0.3f, 0.2f), 0.7f, 0.0f, 1.00f);
    mat[2] = swMaterial(swVec3(0.0f, 0.7f, 0.1f), 0.3f, 0.3f, 1.20f);
    mat[3] = swMaterial(swVec3(0.6f, 0.6f, 0.6f), 0.5f, 0.0f, 1.00f);
    for (int i = 0; i < 2 * 2 * 2; i++) {
        scene.push(swSphere(swVec3((float)(-6 + 12 * (i & 1)),
                                   (float)(-6 + 12 * ((i >> 1) & 1)),
                                   (float)(-6 + 12 * ((i >> 2) & 1))),
                            3.5, mat[i % 3]));
    }
    scene.push(swSphere(swVec3(0, -210, 0), 200, mat[3]));

    // Setup camera
    swVec3 eye(27, 17, 21);
    swVec3 lookAt(-1, -3, 0);
    swVec3 up(0, 1, 0);
    swCamera camera(eye, lookAt, up, 52.0f,
                    (float)imageWidth / (float)imageHeight);
    camera.setup(imageWidth, imageHeight);

    // Ray Trace pixels
    int depth = 20;
    int subSampling = 3;
    float ss = static_cast<float>(subSampling);
    float d = 0.5f / ss;
    
    std::cout << "Rendering\n";
    for (int j = 0; j < imageHeight; ++j) {
        for (int i = 0; i < imageWidth; ++i) {
            Color pixel;
            // trace pixel
            for (int x = 0; x < subSampling; x++) {
                for (int y = 0; y < subSampling; y++) {
                    float rx = uniform() - 0.5f;
                    float ry = uniform() - 0.5f;

                    float cx = ((float)i) 
                        + ((float)x) * d
                        + rx * d;
                    float cy = ((float)j) 
                        + ((float)y) * d
                        + ry * d;

                    swRay r = camera.getRay(cx, cy);
                    pixel += traceRay(r, scene, depth);
                }
            }

            pixel = pixel * (1 / powf(subSampling, 2));

            WriteColor((j * imageWidth + i) * numChannels, pixel, pixels);
        }
    }
    stbi_write_png("out.png", imageWidth, imageHeight, numChannels, pixels,
                   imageWidth * numChannels);
    delete[] pixels;
    std::cout << "Done.\n";
}
