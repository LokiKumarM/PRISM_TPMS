#ifndef TPMS_SW_RULEBASED_H
#define TPMS_SW_RULEBASED_H

#include <stddef.h>

// Rule-based TPMS decision function
// pressure[] and temperature[] are sliding-window arrays of size window_size
// placard_pressure: manufacturer recommended cold inflation pressure (kPa)
// status_out: buffer to receive short status string (recommended >=32 bytes)
// context_out: buffer to receive additional context (recommended >=64 bytes)
void tpms_sw_rulebased_process(
    float pressure[],
    float temperature[],
    int window_size,
    float placard_pressure,
    char status_out[],
    char context_out[]
);

#endif
