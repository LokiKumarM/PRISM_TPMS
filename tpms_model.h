#ifndef TPMS_MODEL_H
#define TPMS_MODEL_H

#define WINDOW_SIZE 64
#define NUM_FEATURES 2
#define NUM_CLASSES 6
#define NUM_CONCEPTS 11   // adjust if different

int tpms_model_init(const char* model_path);

int tpms_model_infer(
    float pressure[],
    float temperature[],
    float concept_output[],
    float class_output[]
);

void tpms_model_deinit(void);

#endif