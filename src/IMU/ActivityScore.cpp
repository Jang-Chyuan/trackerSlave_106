#include "ActivityScore.h"


// ============================================================
// clamp01
// ============================================================

float clamp01(
    float value)
{
    if (value <= 0.0f)
    {
        return 0.0f;
    }


    if (value >= 1.0f)
    {
        return 1.0f;
    }


    return value;
}


// ============================================================
// normalize01
// ============================================================

float normalize01(
    float value,
    float minimum,
    float maximum)
{
    if (maximum <= minimum)
    {
        return 0.0f;
    }


    float result =
        (value - minimum) /
        (maximum - minimum);


    return clamp01(result);
}


// ============================================================
// calculateActivityScore
// ============================================================

float calculateActivityScore(
    ActivityFeatures& features)
{
    if (!features.valid)
    {
        features.activityScore = 0.0f;

        return 0.0f;
    }


    // ========================================================
    // ACC Variance
    // ========================================================

    features.accVarianceScore =
        normalize01(
            features.accVariance,
            0.00002028f,
            0.50f
        );


    // ========================================================
    // ACC StdDev
    // ========================================================

    features.accStdDevScore =
        normalize01(
            features.accStdDev,
            0.004503f,
            0.70f
        );


    // ========================================================
    // GYRO magnitude mean
    // ========================================================

    features.gyroMeanScore =
        normalize01(
            features.gyroMagnitudeMean,
            0.1960f,
            100.0f
        );


    // ========================================================
    // GYRO StdDev
    // ========================================================

    features.gyroStdDevScore =
        normalize01(
            features.gyroStdDev,
            0.084927f,
            50.0f
        );


    // ========================================================
    // Weighted score
    //
    // ACC Variance = 35%
    // ACC StdDev   = 20%
    // GYRO Mean    = 15%
    // GYRO StdDev  = 30%
    // ========================================================

    float score =
        0.35f *
        features.accVarianceScore

        +

        0.20f *
        features.accStdDevScore

        +

        0.15f *
        features.gyroMeanScore

        +

        0.30f *
        features.gyroStdDevScore;


    // ========================================================
    // Final clamp
    // ========================================================

    features.activityScore =clamp01(score);


    return features.activityScore;
}