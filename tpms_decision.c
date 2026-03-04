#include "tpms_decision.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ================================
// Concept Names (match model order)
// ================================
static const char* CONCEPT_NAMES[] = {
    "pressure_stable",
    "pressure_trend_up",
    "pressure_trend_down",
    "drop_rate_slow",
    "drop_rate_fast",
    "pressure_low",
    "temp_trend_up",
    "temp_trend_down",
    "temp_high",
    "temp_low",
    "sensor_data_invalid",
};

// ================================
// Class Names (match model order)
// ================================
static const char* CLASS_NAMES[] = {
"NORMAL", "NORMAL_THERMAL", "SLOW_LEAK", "FAST_LEAK", "UNDERINFLATED", "SENSOR_FAULT"
};

// ================================
// Find Argmax Class
// ================================
static int get_predicted_class(float probs[], int count) {

    int best = 0;

    for (int i = 1; i < count; i++) {
        if (probs[i] > probs[best])
            best = i;
    }

    return best;
}

// ================================
// Concept Interpretation
// ================================
static void interpret_concepts(
    float concepts[],
    int concept_count, 
    char concept_str[]
) {

    const float THRESH = 0.5f;
    concept_str[0] = '\0';

    // ---- SENSOR FAULT OVERRIDE ----
    const int SENSOR_FAULT_IDX = 10;   // ⚠️ adjust if needed

    if (concepts[SENSOR_FAULT_IDX] > THRESH) {
        strcpy(concept_str, CONCEPT_NAMES[SENSOR_FAULT_IDX]);
        return;
    }

    for (int i = 0; i < concept_count; i++) {

        if (concepts[i] > THRESH) {

            if (i < (int)(sizeof(CONCEPT_NAMES)/sizeof(CONCEPT_NAMES[0])))
            {
                strcat(concept_str, " ");
                strcat(concept_str, CONCEPT_NAMES[i]);
            }
            else 
                strcat(concept_str, " No_Strong_Concept");
        }
    }
}

// ================================
// Main Decision Function
// ================================
void tpms_decision_process(
    float concepts[],
    int concept_count,
    float class_probs[],
    int class_count,
    char concept_str[],
    char context[],
    float* confidence
) {

    // 1. Concept evidence
    interpret_concepts(concepts, concept_count, concept_str);

    // 2. Predicted class
    int class_id =
        get_predicted_class(class_probs, class_count);

    if (class_id < (int)(sizeof(CLASS_NAMES)/sizeof(CLASS_NAMES[0]))) {
        strcpy(context, CLASS_NAMES[class_id]);
    } else {
        strcpy(context, "UNKNOWN_STATE");
    }

    *confidence = class_probs[class_id];
}