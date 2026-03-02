#include <stdio.h>
#include "sensor_simulation.h"

int main() {

    float pressure[WINDOW_SIZE];
    float temperature[WINDOW_SIZE];

    sensor_sim_init();

    // Generate scenario
    generate_normal(pressure, temperature);

    printf("Generated Sensor Values:\n\n");

    for (int i = 0; i < WINDOW_SIZE; i++) {
        printf("t=%d  P=%.2f  T=%.2f\n", i, pressure[i], temperature[i]);
    }

    return 0;
}