#include "ActivityFeature.h"

#include <math.h>


// ============================================================
// Global Activity Feature Calculator
// ============================================================

ActivityFeatureCalculator ActivityFeature;


// ============================================================
// Shared Activity Result
// ============================================================
//
// ActivityTask 寫入
// imu2LoraTask 讀取
//
// 必須透過 activityMutex 保護
// ============================================================

ActivityFeatures latestActivityFeatures{};

SemaphoreHandle_t activityMutex = nullptr;

// A window is nominally 10 seconds. Allow scheduling margin, never infinite reuse.
static constexpr uint32_t ACTIVITY_MAX_AGE_MS = 30000;


// ============================================================
// Activity Shared Init
// ============================================================

void activitySharedInit()
{
    if (activityMutex != nullptr)
    {
        return;
    }


    activityMutex =
        xSemaphoreCreateMutex();


    if (activityMutex == nullptr)
    {
        Serial.println(
            "[ACTIVITY] ERROR: mutex create failed"
        );
    }
    else
    {
        Serial.println(
            "[ACTIVITY] Mutex created"
        );
    }
}


// ============================================================
// Get Latest Activity Result
// ============================================================
//
// imu2LoraTask 使用
//
// return:
//     true  = 成功取得資料
//     false = Mutex 尚未建立或取得失敗
//
// 注意：
//     features.valid 可以另外判斷資料是否真的完成
//     一個 10 秒 Activity window。
// ============================================================

bool activityGetLatest(
    ActivityFeatures& features
)
{
    if (activityMutex == nullptr)
    {
        return false;
    }


    if (
        xSemaphoreTake(
            activityMutex,
            pdMS_TO_TICKS(5)
        ) != pdTRUE
    )
    {
        return false;
    }


    features =
        latestActivityFeatures;


    xSemaphoreGive(
        activityMutex
    );


    // Unsigned subtraction remains correct across millis() wraparound.
    if (features.valid &&
        static_cast<uint32_t>(millis() - features.timestamp) > ACTIVITY_MAX_AGE_MS)
    {
        features = {};
    }
    return true;
}


// ============================================================
// ActivityFeatureCalculator::begin()
// ============================================================

void ActivityFeatureCalculator::begin()
{
    reset();
}


// ============================================================
// Reset
// ============================================================
//
// 完整清除：
//     accumulator
//     result
//     resultReady
//
// 啟動時使用。
// ============================================================

void ActivityFeatureCalculator::reset()
{
    resetAccumulator();

    result = {};

    resultReady = false;
}


// ============================================================
// Reset Accumulator
// ============================================================
//
// 注意：
// 這裡「不能」清除 result。
//
// 10 秒 window 完成後只呼叫這個函數，
// 才能保留上一個 ActivityFeatures 給其他 Task 使用。
// ============================================================

void ActivityFeatureCalculator::resetAccumulator()
{
    // --------------------------------------------------------
    // ACC Welford
    // --------------------------------------------------------

    accCount = 0;

    accMean = 0.0f;

    accM2 = 0.0f;


    // --------------------------------------------------------
    // GYRO Welford
    // --------------------------------------------------------

    gyroCount = 0;

    gyroMean = 0.0f;

    gyroM2 = 0.0f;


    // --------------------------------------------------------
    // ACC Dynamic RMS
    // --------------------------------------------------------

    dynamicCount = 0;

    dynamicSumSquares = 0.0f;
}


// ============================================================
// update()
// ============================================================
//
// 每收到一個 IMUSample 呼叫一次。
//
// 假設：
//     IMU = 100 Hz
//
// WINDOW:
//     1000 samples
//
// 所以：
//     1000 / 100 = 10 sec
// ============================================================

void ActivityFeatureCalculator::update(
    const IMUSample& sample
)
{
    // --------------------------------------------------------
    // Invalid sample
    // --------------------------------------------------------

    if (!sample.valid)
    {
        return;
    }


    // ========================================================
    // ACC magnitude
    // ========================================================
    //
    // sqrt(ax² + ay² + az²)
    //
    // BMI270 accelerometer unit:
    //     g
    // ========================================================

    float accMagnitude =
        sqrtf(
            sample.ax * sample.ax +
            sample.ay * sample.ay +
            sample.az * sample.az
        );


    // ========================================================
    // GYRO magnitude
    // ========================================================
    //
    // sqrt(gx² + gy² + gz²)
    //
    // unit:
    //     deg/s
    // ========================================================

    float gyroMagnitude =
        sqrtf(
            sample.gx * sample.gx +
            sample.gy * sample.gy +
            sample.gz * sample.gz
        );


    // ========================================================
    // ACC Welford
    // ========================================================

    accCount++;


    float accDelta =
        accMagnitude - accMean;


    accMean +=
        accDelta /
        static_cast<float>(accCount);


    float accDelta2 =
        accMagnitude - accMean;


    accM2 +=
        accDelta * accDelta2;


    // ========================================================
    // GYRO Welford
    // ========================================================

    gyroCount++;


    float gyroDelta =
        gyroMagnitude - gyroMean;


    gyroMean +=
        gyroDelta /
        static_cast<float>(gyroCount);


    float gyroDelta2 =
        gyroMagnitude - gyroMean;


    gyroM2 +=
        gyroDelta * gyroDelta2;


    // ========================================================
    // ACC Dynamic component
    // ========================================================
    //
    // static gravity ≈ 1g
    //
    // dynamic acceleration:
    //
    //     |ACC magnitude - 1g|
    //
    // RMS later calculated over window
    // ========================================================

    float dynamicAcc =
        accMagnitude - 1.0f;


    dynamicSumSquares +=
        dynamicAcc * dynamicAcc;


    dynamicCount++;


    // ========================================================
    // Check Window
    // ========================================================

    if (accCount < WINDOW_SAMPLES)
    {
        return;
    }


    // ========================================================
    // Calculate ACC variance
    // ========================================================

    float accVariance = 0.0f;


    if (accCount > 1)
    {
        accVariance =
            accM2 /
            static_cast<float>(
                accCount - 1
            );
    }


    // ========================================================
    // Calculate GYRO variance
    // ========================================================

    float gyroVariance = 0.0f;


    if (gyroCount > 1)
    {
        gyroVariance =
            gyroM2 /
            static_cast<float>(
                gyroCount - 1
            );
    }


    // ========================================================
    // Standard deviation
    // ========================================================

    float accStdDev =
        sqrtf(
            accVariance
        );


    float gyroStdDev =
        sqrtf(
            gyroVariance
        );


    // ========================================================
    // ACC Dynamic RMS
    // ========================================================

    float accDynamicRMS = 0.0f;


    if (dynamicCount > 0)
    {
        accDynamicRMS =
            sqrtf(
                dynamicSumSquares /
                static_cast<float>(
                    dynamicCount
                )
            );
    }


    // ========================================================
    // Build ActivityFeatures
    // ========================================================

    result = {};


    result.timestamp =
        sample.timestamp;


    result.sampleCount =
        static_cast<uint16_t>(
            accCount
        );


    // --------------------------------------------------------
    // ACC
    // --------------------------------------------------------

    result.accMagnitudeMean =
        accMean;

    result.accVariance =
        accVariance;

    result.accStdDev =
        accStdDev;

    result.accDynamicRMS =
        accDynamicRMS;


    // --------------------------------------------------------
    // GYRO
    // --------------------------------------------------------

    result.gyroMagnitudeMean =
        gyroMean;

    result.gyroVariance =
        gyroVariance;

    result.gyroStdDev =
        gyroStdDev;


    // --------------------------------------------------------
    // Score fields
    //
    // ActivityScore.cpp 會填入
    // --------------------------------------------------------

    result.accVarianceScore = 0.0f;

    result.accStdDevScore = 0.0f;

    result.gyroMeanScore = 0.0f;

    result.gyroStdDevScore = 0.0f;

    result.activityScore = 0.0f;


    // --------------------------------------------------------
    // Valid
    // --------------------------------------------------------

    result.valid = true;

    resultReady = true;


    // ========================================================
    // IMPORTANT
    // ========================================================
    //
    // Window 完成後：
    //
    // 1. result 保留
    // 2. resultReady = true
    // 3. accumulator 清零
    //
    // 不可以呼叫 reset()
    // 因為 reset() 會把 result 清掉。
    // ========================================================

    resetAccumulator();
}


// ============================================================
// available()
// ============================================================

bool ActivityFeatureCalculator::available()
{
    return resultReady;
}


// ============================================================
// getResult()
// ============================================================
//
// 取得最後完成的 10 秒結果。
//
// 注意：
//     getResult() 之後 resultReady 會變成 false。
//
// 但是 result 本身仍然保留。
// ============================================================

ActivityFeatures
ActivityFeatureCalculator::getResult()
{
    if (!resultReady)
    {
        ActivityFeatures empty{};

        empty.valid = false;

        return empty;
    }


    resultReady = false;


    return result;
}


// ============================================================
// sampleCount()
// ============================================================

uint32_t
ActivityFeatureCalculator::sampleCount() const
{
    return accCount;
}