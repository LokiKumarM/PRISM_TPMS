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