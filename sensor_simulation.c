#include "sensor_simulation.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>

static float rand_uniform(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

static float rand_normal(float mean, float stddev) {
    float u1 = ((float)rand() + 1.0f) / (RAND_MAX + 1.0f);
    float u2 = ((float)rand() + 1.0f) / (RAND_MAX + 1.0f);
    float z0 = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * M_PI * u2);
    return z0 * stddev + mean;
}

void sensor_sim_init(void) {
    srand(time(NULL));
}

void generate_normal(float P[], float T[]) {
    float P_base = rand_uniform(200, 270);
    float T_base = rand_uniform(25, 35);

    float P_drift = 0.0f;
    float T_drift = 0.0f;

    for (int i = 0; i < WINDOW_SIZE; i++) {
        P_drift += rand_normal(0, 0.05f);
        T_drift += rand_normal(0, 0.02f);

        P[i] = P_base + P_drift;
        T[i] = T_base + T_drift;
    }
}

void generate_normal_thermal(float P[], float T[]) {

    int mode = rand() % 4;

    float P_ref = 240.0f;
    float T_ref_K = 303.15f;

    float T0, dT, T_const;

    if (mode == 0) { // heat_ramp
        T0 = rand_uniform(20, 40);
        dT = rand_uniform(0.06f, 0.08f);
    }
    else if (mode == 1) { // cold_ramp
        T0 = rand_uniform(40, 70);
        dT = -rand_uniform(0.06f, 0.08f);
    }
    else if (mode == 2) { // hot_stable
        T_const = rand_uniform(70, 90);
        dT = 0.0f;
        T0 = T_const;
    }
    else { // cold_stable
        T_const = rand_uniform(-5, 10);
        dT = 0.0f;
        T0 = T_const;
    }

    for (int i = 0; i < WINDOW_SIZE; i++) {
        T[i] = T0 + i * dT;
        float T_K = T[i] + 273.15f;
        P[i] = P_ref * (T_K / T_ref_K);
    }
}

void generate_slow_leak(float P[], float T[]) {

    int mode = rand() % 4;

    float P_ref = 240.0f;
    float T_ref_K = 303.15f;

    float T_curr;
    float dT;

    if (mode == 0) {
        T_curr = rand_uniform(10, 40);
        dT = rand_uniform(0.04f, 0.06f);
    }
    else if (mode == 1) {
        T_curr = rand_uniform(40, 80);
        dT = -rand_uniform(0.04f, 0.06f);
    }
    else if (mode == 2) {
        T_curr = rand_uniform(70, 90);
        dT = 0.0f;
    }
    else {
        T_curr = rand_uniform(-10, 10);
        dT = 0.0f;
    }

    float leak_rate = rand_uniform(0.12f, 0.20f);

    for (int i = 0; i < WINDOW_SIZE; i++) {
        T_curr += dT;
        float T_K = T_curr + 273.15f;

        float P_thermal = P_ref * (T_K / T_ref_K);
        P[i] = P_thermal - leak_rate * i;

        T[i] = T_curr;
    }
}

void generate_fast_leak(float P[], float T[]) {

    int mode = rand() % 4;

    float P_ref = 240.0f;
    float T_ref_K = 303.15f;

    float T_curr;
    float dT;

    if (mode == 0) {
        T_curr = rand_uniform(10, 40);
        dT = rand_uniform(0.04f, 0.06f);
    }
    else if (mode == 1) {
        T_curr = rand_uniform(40, 80);
        dT = -rand_uniform(0.04f, 0.06f);
    }
    else if (mode == 2) {
        T_curr = rand_uniform(70, 90);
        dT = 0.0f;
    }
    else {
        T_curr = rand_uniform(-10, 10);
        dT = 0.0f;
    }

    float leak_rate = rand_uniform(0.6f, 1.2f);
    int puncture_t = rand() % 15 + 5;

    for (int i = 0; i < WINDOW_SIZE; i++) {
        T_curr += dT;
        float T_K = T_curr + 273.15f;
        float P_thermal = P_ref * (T_K / T_ref_K);

        if (i < puncture_t)
            P[i] = P_thermal;
        else
            P[i] = P_thermal - leak_rate * (i - puncture_t);

        T[i] = T_curr;
    }
}

void generate_underinflated(float P[], float T[]) {

    float T_const = rand_uniform(-10, 90);
    float T_ref_K = 303.15f;

    float P_ref_low = rand_uniform(170, 190);
    float T_K = T_const + 273.15f;

    float P_const = P_ref_low * (T_K / T_ref_K);

    float P_drift = 0.0f;
    float T_drift = 0.0f;

    for (int i = 0; i < WINDOW_SIZE; i++) {
        P_drift += rand_normal(0, 0.002f);
        T_drift += rand_normal(0, 0.001f);

        P[i] = P_const + P_drift;
        T[i] = T_const + T_drift;
    }
}

void generate_sensor_fault(float P[], float T[]) {

    float T_curr = rand_uniform(20, 40);
    float P_ref = 240.0f;
    float T_ref_K = 303.15f;

    int fault_t = rand() % 30 + 10;
    int fault_type = rand() % 5;

    for (int i = 0; i < WINDOW_SIZE; i++) {

        if (i < fault_t) {
            float dT = rand_uniform(-0.02f, 0.02f);
            T_curr += dT;
            float T_K = T_curr + 273.15f;
            P[i] = P_ref * (T_K / T_ref_K);
            T[i] = T_curr;
        }
        else {
            if (fault_type == 0) { // pressure_high
                P[i] = rand_uniform(350, 600);
                T[i] = T_curr;
            }
            else if (fault_type == 1) { // pressure_low
                P[i] = rand_uniform(0, 40);
                T[i] = T_curr;
            }
            else if (fault_type == 2) { // temp_high
                T[i] = rand_uniform(130, 200);
                P[i] = P_ref;
            }
            else if (fault_type == 3) { // temp_low
                T[i] = rand_uniform(-100, -50);
                P[i] = P_ref;
            }
            else { // stuck
                P[i] = rand_uniform(400, 800);
                T[i] = rand_uniform(150, 300);
            }
        }
    }
}

// Adjust probabilities as needed
void generate_weighted_scenario(float pressure[], float temperature[])
{
    int r = rand() % 100;   // 0–99

    if (r < 20) {
        // 20% Normal
        generate_normal(pressure, temperature);
    }
    else if (r < 40) {
        // 20% Slow leak
        generate_normal_thermal(pressure, temperature);
    }
    else if (r < 60) {
        // 20% Slow leak
        generate_slow_leak(pressure, temperature);
    }
    else if (r < 75) {
        // 15% Underinflated
        generate_underinflated(pressure, temperature);
    }
    else if (r < 90) {
        // 15% Fast leak
        generate_fast_leak(pressure, temperature);
    }
    else {
        // 10% Sensor fault
        generate_sensor_fault(pressure, temperature);
    }
}