#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "sensor_simulation.h"
#include "tpms_model.h"
#include "tpms_decision.h"
#include "lcd_i2c.h"

// -------------------------------------------------
// HIGH RESOLUTION TIMER (milliseconds)
// -------------------------------------------------
double get_time_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000.0) + (ts.tv_nsec / 1e6);
}

int main()
{
    srand(time(NULL));

    float pressure[WINDOW_SIZE];
    float temperature[WINDOW_SIZE];

    float concepts[NUM_CONCEPTS];
    float classes[NUM_CLASSES];
    float confidence = 0.0f;

    char line1[17];
    char context_out[64];
    char concept_out[128] = "";
    char message[256];

    sleep(2);

    sensor_sim_init();
    lcd_init();

    if (tpms_model_init("model_tpms_8.tflite") != 0)
    {
        lcd_set_cursor(0, 0);
        lcd_print("Model Error");
        lcd_set_cursor(1, 0);
        lcd_print("Init Failed");
        while (1);
    }

    scroll_text_line2("Starting Intelligent TPMS Engine...");
    sleep(1);
    lcd_clear();

    int counter = 0;
    const int SCENARIO_INTERVAL = 1;
    int ts_index = 0;

    generate_normal_thermal(pressure, temperature);

    // -------------------------------------------------
    // ⭐ MEASURE MODEL INFERENCE TIME
    // -------------------------------------------------
    double infer_start = get_time_ms();

    tpms_model_infer(pressure, temperature, concepts, classes);

    double infer_end = get_time_ms();
    double inference_latency = infer_end - infer_start;

    // -------------------------------------------------
    // ⭐ MEASURE FULL DECISION PIPELINE TIME
    // -------------------------------------------------
    double decision_start = get_time_ms();

    tpms_decision_process(concepts, NUM_CONCEPTS,
                          classes, NUM_CLASSES,
                          concept_out, context_out,
                          &confidence);

    double decision_end = get_time_ms();
    double full_pipeline_latency = decision_end - infer_start;

    printf("Inference Latency: %.3f ms\n", inference_latency);
    printf("Full TPMS Decision Latency: %.3f ms\n", full_pipeline_latency);

    snprintf(message, sizeof(message),
        "Semantics:%s Context:%s Conf:%0.2f",
        concept_out, context_out, confidence);

    scroll_text_line2(message);
    usleep(500000);
    lcd_clear();

    // -------------------------------------------------
    // ⭐ MAIN LOOP
    // -------------------------------------------------
    while (1)
    {
        if (counter >= SCENARIO_INTERVAL)
        {
            generate_weighted_scenario(pressure, temperature);

            // ---- Measure inference ----
            infer_start = get_time_ms();

            tpms_model_infer(pressure, temperature, concepts, classes);

            infer_end = get_time_ms();
            inference_latency = infer_end - infer_start;

            // ---- Measure full pipeline ----
            decision_start = get_time_ms();

            tpms_decision_process(concepts, NUM_CONCEPTS,
                                  classes, NUM_CLASSES,
                                  concept_out, context_out,
                                  &confidence);

            decision_end = get_time_ms();
            full_pipeline_latency = decision_end - infer_start;

            // printf("Inference: %.3f ms | Full Pipeline: %.3f ms\n",
            //        inference_latency,
            //        full_pipeline_latency);

            snprintf(message, sizeof(message), "Semantics:%s Context:%s Conf:%0.2f",
                concept_out, context_out, confidence);

            counter = 0;
        }

        for (ts_index = 0; ts_index < WINDOW_SIZE; ts_index++)
        {
            snprintf(line1, sizeof(line1),
                     "P:%0.1f T:%0.1f",
                     pressure[ts_index],
                     temperature[ts_index]);

            lcd_set_cursor(0, 0);
            lcd_print(line1);

            usleep(20000);
        }

        scroll_text_line2(message);
        sleep(1);
        lcd_clear();
        counter++;
    }
}