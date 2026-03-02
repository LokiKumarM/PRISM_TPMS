#ifndef SENSOR_SIMULATION_H
#define SENSOR_SIMULATION_H

#define WINDOW_SIZE 64

// Initialization
void sensor_sim_init(void);

// Scenario generators
void generate_normal(float P[], float T[]);
void generate_normal_thermal(float P[], float T[]);
void generate_slow_leak(float P[], float T[]);
void generate_fast_leak(float P[], float T[]);
void generate_underinflated(float P[], float T[]);
void generate_sensor_fault(float P[], float T[]);

#endif