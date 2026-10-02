#pragma once

#include "ActivityFeature.h"


// ============================================================
// Utility
// ============================================================

float clamp01(
    float value
);


float normalize01(
    float value,
    float minimum,
    float maximum
);


// ============================================================
// Activity Score
// ============================================================

float calculateActivityScore(ActivityFeatures& features);