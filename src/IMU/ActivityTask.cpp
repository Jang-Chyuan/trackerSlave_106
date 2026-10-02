#include "ActivityTask.h"

#include <Arduino.h>

#include "IMU/IMUDriver.h"
#include "IMU/ActivityFeature.h"
#include "IMU/ActivityScore.h"

// ============================================================
// Global IMU Driver
// ============================================================

extern IMUDriver IMU;


// ============================================================
// Activity shared result
// ============================================================

extern ActivityFeatures latestActivityFeatures;

extern SemaphoreHandle_t activityMutex;


// ============================================================
// Activity Task Init
// ============================================================

void activityTaskInit()
{
    Serial.println("[ACTIVITY] Init");
}


// ============================================================
// Activity Task
// ============================================================
//
// BMI270
//    |
//    ▼
// IMU.read()
//    |
//    ▼
// IMUSample
//    |
//    ▼
// ActivityFeature.update()
//    |
//    ▼
// 1000 samples
//    |
//    ▼
// ActivityFeatures
//    |
//    ▼
// ActivityScore
//    |
//    ▼
// latestActivityFeatures
//
// ============================================================

void activityTask(void *pvParameters)
{
    (void)pvParameters;


    Serial.println(
        "[ACTIVITY] Task started"
    );


    // ========================================================
    // Activity Feature initialization
    // ========================================================

    ActivityFeature.begin();


    // ========================================================
    // Debug counter
    // ========================================================

    uint32_t sampleCount = 0;

    uint32_t resultCount = 0;


    // ========================================================
    // Sampling period
    //
    // 10 ms = 100 Hz
    // ========================================================

    const TickType_t samplePeriod =
        pdMS_TO_TICKS(10);


    // ========================================================
    // Task loop
    // ========================================================

    for (;;)
    {
        IMUSample sample;


        // ====================================================
        // Read BMI270
        // ====================================================

        if (IMU.read(sample))
        {
            sampleCount++;


            // =================================================
            // Feed Activity Feature
            // =================================================

            ActivityFeature.update(
                sample
            );


            // =================================================
            // Check completed Activity window
            // =================================================

            if (ActivityFeature.available())
            {
                // ------------------------------------------------
                // Get completed 10 sec result
                // ------------------------------------------------

                ActivityFeatures result =
                    ActivityFeature.getResult();


                // ------------------------------------------------
                // Validate
                // ------------------------------------------------

                if (!result.valid)
                {
                    Serial.println(
                        "[ACTIVITY] Invalid result"
                    );
                }
                else
                {
                    // ============================================
                    // Calculate Activity Score
                    // ============================================

                    result.activityScore =
                        calculateActivityScore(
                            result
                        );


                    // ============================================
                    // Clamp 0 ~ 1
                    // ============================================

                    if (
                        result.activityScore < 0.0f
                    )
                    {
                        result.activityScore = 0.0f;
                    }


                    if (
                        result.activityScore > 1.0f
                    )
                    {
                        result.activityScore = 1.0f;
                    }


                    // ============================================
                    // Result counter
                    // ============================================

                    resultCount++;


                    // ============================================
                    // Activity Result
                    // ============================================

                    Serial.println(
                        "================================"
                    );


                    Serial.print(
                        "[ACTIVITY] RESULT #"
                    );

                    Serial.println(
                        resultCount
                    );


                    Serial.print(
                        "[ACTIVITY] timestamp="
                    );

                    Serial.println(
                        result.timestamp
                    );


                    Serial.print(
                        "[ACTIVITY] samples="
                    );

                    Serial.println(
                        result.sampleCount
                    );


                    Serial.print(
                        "[ACTIVITY] accMean="
                    );

                    Serial.println(
                        result.accMagnitudeMean,
                        4
                    );


                    Serial.print(
                        "[ACTIVITY] accVariance="
                    );

                    Serial.println(
                        result.accVariance,
                        4
                    );


                    Serial.print(
                        "[ACTIVITY] accStdDev="
                    );

                    Serial.println(
                        result.accStdDev,
                        4
                    );


                    Serial.print(
                        "[ACTIVITY] accDynamicRMS="
                    );

                    Serial.println(
                        result.accDynamicRMS,
                        4
                    );


                    Serial.print(
                        "[ACTIVITY] gyroMean="
                    );

                    Serial.println(
                        result.gyroMagnitudeMean,
                        4
                    );


                    Serial.print(
                        "[ACTIVITY] gyroVariance="
                    );

                    Serial.println(
                        result.gyroVariance,
                        4
                    );


                    Serial.print(
                        "[ACTIVITY] gyroStdDev="
                    );

                    Serial.println(
                        result.gyroStdDev,
                        4
                    );


                    Serial.print(
                        "[ACTIVITY] SCORE="
                    );

                    Serial.println(
                        result.activityScore,
                        4
                    );


                    Serial.println(
                        "[ACTIVITY] VALID=YES"
                    );


                    // ============================================
                    // Publish latest Activity result
                    // ============================================

                    if (
                        activityMutex != nullptr
                    )
                    {
                        if (
                            xSemaphoreTake(
                                activityMutex,
                                pdMS_TO_TICKS(5)
                            ) == pdTRUE
                        )
                        {
                            latestActivityFeatures =
                                result;


                            xSemaphoreGive(
                                activityMutex
                            );
                        }
                    }
                }
            }
        }


        // ====================================================
        // 100 Hz
        // ====================================================

        vTaskDelay(
            samplePeriod
        );
    }
}