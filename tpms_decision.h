#ifndef TPMS_DECISION_H
#define TPMS_DECISION_H

// Process model outputs and print human-readable result
void tpms_decision_process(
    float concepts[],
    int concept_count,
    float class_probs[],
    int class_count
);

#endif