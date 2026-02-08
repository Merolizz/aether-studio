#pragma once

struct ColorGradeState {
    float liftR = 0.f, liftG = 0.f, liftB = 0.f;
    float gammaR = 1.f, gammaG = 1.f, gammaB = 1.f;
    float gainR = 1.f, gainG = 1.f, gainB = 1.f;
    const float* lut3D = nullptr;
    int lutSize = 0;
};
