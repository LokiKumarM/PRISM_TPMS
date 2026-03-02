#include "tpms_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "tensorflow/lite/c/c_api.h"

// ----------- Static handles -----------
static TfLiteModel* model = NULL;
static TfLiteInterpreterOptions* options = NULL;
static TfLiteInterpreter* interpreter = NULL;

// ----------- Physical bounds -----------
#define PRESSURE_MIN 60.0f
#define PRESSURE_MAX 320.0f
#define TEMP_MIN -40.0f
#define TEMP_MAX 125.0f

// ----------- Normalization -----------
static float normalize(float value, float min, float max) {
    float norm = (value - min) / (max - min);
    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;
    return norm;
}

// ----------- INIT -----------
int tpms_model_init(const char* model_path) {

    model = TfLiteModelCreateFromFile(model_path);
    if (!model) {
        printf("Failed to load model\n");
        return -1;
    }

    options = TfLiteInterpreterOptionsCreate();
    TfLiteInterpreterOptionsSetNumThreads(options, 1);

    interpreter = TfLiteInterpreterCreate(model, options);
    if (!interpreter) {
        printf("Failed to create interpreter\n");
        return -2;
    }

    if (TfLiteInterpreterAllocateTensors(interpreter) != kTfLiteOk) {
        printf("Failed to allocate tensors\n");
        return -3;
    }

    return 0;
}

// ----------- INFERENCE -----------
int tpms_model_infer(
    float pressure[],
    float temperature[],
    float concept_output[],
    float class_output[]
) {

    TfLiteTensor* input_tensor =
        TfLiteInterpreterGetInputTensor(interpreter, 0);

    float input_buffer[WINDOW_SIZE * NUM_FEATURES];

    int idx = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        input_buffer[idx++] =
            normalize(pressure[i], PRESSURE_MIN, PRESSURE_MAX);
        input_buffer[idx++] =
            normalize(temperature[i], TEMP_MIN, TEMP_MAX);
    }

    // Copy data to input tensor
    if (TfLiteTensorCopyFromBuffer(
            input_tensor,
            input_buffer,
            sizeof(input_buffer)) != kTfLiteOk) {
        printf("Failed to copy input data\n");
        return -1;
    }

    // Run inference
    if (TfLiteInterpreterInvoke(interpreter) != kTfLiteOk) {
        printf("Inference failed\n");
        return -2;
    }

    // ----------- OUTPUT HANDLING -----------

    const TfLiteTensor* concept_tensor =
        TfLiteInterpreterGetOutputTensor(interpreter, 1);

    const TfLiteTensor* class_tensor =
        TfLiteInterpreterGetOutputTensor(interpreter, 0);

    TfLiteTensorCopyToBuffer(
        concept_tensor,
        concept_output,
        NUM_CONCEPTS * sizeof(float));

    TfLiteTensorCopyToBuffer(
        class_tensor,
        class_output,
        NUM_CLASSES * sizeof(float));

    return 0;
}

// ----------- DEINIT -----------
void tpms_model_deinit(void) {

    if (interpreter) {
        TfLiteInterpreterDelete(interpreter);
        interpreter = NULL;
    }

    if (options) {
        TfLiteInterpreterOptionsDelete(options);
        options = NULL;
    }

    if (model) {
        TfLiteModelDelete(model);
        model = NULL;
    }
}