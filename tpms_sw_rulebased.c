#include "tpms_sw_rulebased.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sensor_simulation.h"

// Default threshold parameters (kPa and percent values)
#define DEFAULT_PLACARD_PRESSURE_KPA 220.0f
#define LOW_PRESSURE_FACTOR 0.80f
#define CRITICAL_PRESSURE_FACTOR 0.75f
#define FAST_LEAK_DROP_KPA 5.0f
#define SLOW_LEAK_PERCENT_DROP 5.0f
#define OVERHEAT_THRESHOLD_C 75.0f
#define NORMAL_VARIATION_PCT 20.0f  // Increased: generators create 200-270 range
#define THERMAL_VARIATION_PCT 8.0f
#define SLOW_LEAK_SLOPE_THRESHOLD -0.10f  // Lowered from -0.15
#define FAST_LEAK_SLOPE_THRESHOLD -0.50f  // Very steep slope (fast leak)
#define SLOW_LEAK_DROP_THRESHOLD 1.5f     // Minimum leak_drop to confirm slow leak

void tpms_sw_rulebased_process(
    float pressure[],
    float temperature[],
    int window_size,
    float placard_pressure,
    char status_out[],
    char context_out[]
)
{
    if (window_size <= 0) {
        strcpy(status_out, "SENSOR_FAULT");
        strcpy(context_out, "invalid_window_size");
        return;
    }

    if (placard_pressure <= 0.0f) placard_pressure = DEFAULT_PLACARD_PRESSURE_KPA;

    // === Summary statistics ===
    float sum_p = 0.0f, sum_t = 0.0f;
    float min_p = pressure[0], max_p = pressure[0];
    float min_t = temperature[0], max_t = temperature[0];

    for (int i = 0; i < window_size; i++) {
        float p = pressure[i];
        float t = temperature[i];
        min_p = (p < min_p) ? p : min_p;
        max_p = (p > max_p) ? p : max_p;
        min_t = (t < min_t) ? t : min_t;
        max_t = (t > max_t) ? t : max_t;
        sum_p += p;
        sum_t += t;
    }

    float avg_p = sum_p / window_size;
    float avg_t = sum_t / window_size;
    float temp_range = max_t - min_t;

    // === SENSOR_FAULT CHECKS ===
    if (avg_p <= 40.0f || avg_p > 400.0f) {
        strcpy(status_out, "SENSOR_FAULT");
        strcpy(context_out, "Pressure out of physical limits");
        return;
    }
    if (avg_t < -50.0f || avg_t > 150.0f || temp_range > 140.0f) {
        strcpy(status_out, "SENSOR_FAULT");
        strcpy(context_out, "Temperature out of physical limits");
        return;
    }
    // Detect extreme values (min/max wiping out average)
    if (min_p < 20.0f || max_p > 500.0f || min_t < -70.0f || max_t > 160.0f) {
        strcpy(status_out, "SENSOR_FAULT");
        strcpy(context_out, "Extreme outlier values detected");
        return;
    }

    // === Linear regression on pressure ===
    float sum_i = 0.0f, sum_i2 = 0.0f, sum_ip = 0.0f;
    for (int i = 0; i < window_size; i++) {
        sum_i += i;
        sum_i2 += i * i;
        sum_ip += i * pressure[i];
    }
    float n = (float)window_size;
    float denom = n * sum_i2 - sum_i * sum_i;
    float slope = 0.0f;
    if (fabsf(denom) > 1e-6f) {
        slope = (n * sum_ip - sum_i * sum_p) / denom;
    }

    // Max single-step drop
    float max_step_drop = 0.0f;
    for (int i = 1; i < window_size; i++) {
        float step = pressure[i-1] - pressure[i];
        if (step > max_step_drop) max_step_drop = step;
    }

    // === Thermal compensation (Gay-Lussac's Law) ===
    float T_ref_K = 303.15f;
    float P_ref = 240.0f;
    float P_expected_first = P_ref * ((temperature[0] + 273.15f) / T_ref_K);
    float P_expected_last = P_ref * ((temperature[window_size-1] + 273.15f) / T_ref_K);
    float thermal_expected_drop = P_expected_first - P_expected_last;
    float leak_drop = (max_p - min_p) - thermal_expected_drop;

    // === Decision logic (priority order) ===

    // 1. UNDERINFLATED (check lower baseline, not just average)
    float critical_threshold = CRITICAL_PRESSURE_FACTOR * placard_pressure;
    if (min_p < critical_threshold * 1.1f || avg_p < critical_threshold * 1.2f) {
        strcpy(status_out, "UNDERINFLATED");
        snprintf(context_out, 64, "min_p=%.1f, avg_p=%.1f < crit=%.1f", min_p, avg_p, critical_threshold);
        return;
    }

    // 2. FAST_LEAK via large single step OR very steep negative slope
    if (max_step_drop >= FAST_LEAK_DROP_KPA || slope < FAST_LEAK_SLOPE_THRESHOLD) {
        strcpy(status_out, "FAST_LEAK");
        snprintf(context_out, 64, "max_step_drop=%.2f or slope=%.3f kPa/sample", max_step_drop, slope);
        return;
    }

    // 3. SLOW_LEAK (consistent negative slope and significant leak_drop)
    if (slope < SLOW_LEAK_SLOPE_THRESHOLD && leak_drop > SLOW_LEAK_DROP_THRESHOLD) {
        strcpy(status_out, "SLOW_LEAK");
        snprintf(context_out, 64, "slope=%.3f kPa/s, leak_drop=%.1f", slope, leak_drop);
        return;
    }

    // 4. NORMAL_THERMAL (temp swing detected, pressure follows thermal model, no significant leak)
    if (temp_range > 2.0f && fabsf(leak_drop) < SLOW_LEAK_DROP_THRESHOLD && fabsf(slope) < 0.08f) {
        strcpy(status_out, "NORMAL_THERMAL");
        snprintf(context_out, 64, "temp_range=%.1fC, thermal_model_fit", temp_range);
        return;
    }

    // 5. NORMAL (stable pressure, minimal slope, small leak_drop)
    float nominal = placard_pressure;
    float pct_dev = fabsf((avg_p - nominal) / nominal) * 100.0f;
    if (pct_dev <= NORMAL_VARIATION_PCT && fabsf(slope) < 0.08f && fabsf(leak_drop) < SLOW_LEAK_DROP_THRESHOLD) {
        strcpy(status_out, "NORMAL");
        snprintf(context_out, 64, "stable: avg=%.1f, deviate=%.1f%%", avg_p, pct_dev);
        return;
    }

    // === FALLBACK ===
    if (slope < SLOW_LEAK_SLOPE_THRESHOLD) {
        strcpy(status_out, "SLOW_LEAK");
        snprintf(context_out, 64, "marginal_leak: slope=%.3f", slope);
    } else if (temp_range > 3.0f) {
        strcpy(status_out, "NORMAL_THERMAL");
        snprintf(context_out, 64, "marginal_thermal: range=%.1f", temp_range);
    } else {
        strcpy(status_out, "NORMAL");
        snprintf(context_out, 64, "marginal_stable: avg=%.1f", avg_p);
    }
}

// Main function preserved for standalone testing
// To use this module in comparison, compile without linking main or use
// gcc -c tpms_sw_rulebased.c && link with comparison tool
