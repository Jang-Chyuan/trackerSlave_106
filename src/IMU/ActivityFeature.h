#pragma once

#include <Arduino.h>
#include "IMUDriver.h"


// ============================================================
// Activity Features
// ============================================================

struct ActivityFeatures
{
    uint32_t timestamp;

    uint16_t sampleCount;

    // ACC
    float accMagnitudeMean;
    float accVariance;
    float accStdDev;
    float accDynamicRMS;

    // GYRO
    float gyroMagnitudeMean;
    float gyroVariance;
    float gyroStdDev;

    // Normalized
    float accVarianceScore;
    float accStdDevScore;

    float gyroMeanScore;
    float gyroStdDevScore;

    // Final
    float activityScore;

    bool valid;
};


// ============================================================
// Activity Feature Calculator
// ============================================================

class ActivityFeatureCalculator
{
public:

    void begin();

    void reset();

    void update(
        const IMUSample& sample
    );

    bool available();

    ActivityFeatures getResult();

    uint32_t sampleCount() const;


private:

    uint32_t accCount = 0;

    float accMean = 0.0f;
    float accM2 = 0.0f;


    uint32_t gyroCount = 0;

    float gyroMean = 0.0f;
    float gyroM2 = 0.0f;


    uint32_t dynamicCount = 0;

    float dynamicSumSquares = 0.0f;


    static constexpr uint32_t WINDOW_SAMPLES = 1000;


    ActivityFeatures result{};

    bool resultReady = false;


    void resetAccumulator();
};


extern ActivityFeatureCalculator ActivityFeature;


// ============================================================
// Shared Activity Result
// ============================================================

extern ActivityFeatures latestActivityFeatures;

extern SemaphoreHandle_t activityMutex;


// ============================================================
// Shared Activity API
// ============================================================

void activitySharedInit();

bool activityGetLatest(
    ActivityFeatures& features
);