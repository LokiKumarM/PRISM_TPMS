#include "tpms_decision.h"
#include <stdio.h>

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
    "sensor_fault",
};

// ================================
// Class Names (match model order)
// ================================
static const char* CLASS_NAMES[] = {
"Normal", "Normal_Thermal", "Slow_Leak", "Fast_Leak", "Underinflated", "Sensor_Fault"
};

// ================================
// Action Mapping
// ================================
static const char* get_action(int class_id) {

    switch (class_id) {

        case 0: return "NO_ACTION";

        case 1: return "CHECK_TIRE_SOON";

        case 2: return "PULL_OVER_IMMEDIATELY";

        case 3: return "INFLATE_TIRE";

        case 4: return "CHECK_SENSOR";

        default: return "UNKNOWN";
    }
}

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
    int concept_count
) {

    const float THRESH = 0.5f;

    printf("Evidence: [");

    for (int i = 0; i < concept_count; i++) {

        if (concepts[i] > THRESH) {

            if (i < (int)(sizeof(CONCEPT_NAMES)/sizeof(CONCEPT_NAMES[0])))
                printf("%s ", CONCEPT_NAMES[i]);
            else
                printf("concept_%d ", i);
        }
    }

    printf("]\n");
}

// ================================
// Main Decision Function
// ================================
void tpms_decision_process(
    float concepts[],
    int concept_count,
    float class_probs[],
    int class_count
) {

    printf("\n=== TPMS OUTPUT ===\n");

    // 1. Concept evidence
    interpret_concepts(concepts, concept_count);

    // 2. Predicted class
    int class_id =
        get_predicted_class(class_probs, class_count);

    const char* behaviour =
        (class_id < (int)(sizeof(CLASS_NAMES)/sizeof(CLASS_NAMES[0])))
        ? CLASS_NAMES[class_id]
        : "UNKNOWN";

    float confidence = class_probs[class_id];

    printf("Behaviour: %s\n", behaviour);
    printf("Confidence: %.3f\n", confidence);

    // 3. Action mapping
    const char* action = get_action(class_id);

    printf("Action: %s\n", action);
}