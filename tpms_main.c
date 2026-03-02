#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "sensor_simulation.h"
#include "tpms_model.h"
#include "tpms_decision.h"
#include "lcd_i2c.h"

int main()
{
    srand(time(NULL));

    float pressure[WINDOW_SIZE];
    float temperature[WINDOW_SIZE];

    float concepts[NUM_CONCEPTS];
    float classes[NUM_CLASSES];

    char line1[17];
    char action[32];
    char explanation[64];
    char message[128];

    // -------------------------------------------------
    // ⭐ Wait for I2C device (important at boot)
    // -------------------------------------------------
    sleep(2);

    sensor_sim_init();
    lcd_init();

    // -------------------------------------------------
    // ⭐ BOOT SPLASH SCREEN
    // -------------------------------------------------
    lcd_set_cursor(0, 0);
    lcd_print("vAIcle TPMS");

    lcd_set_cursor(1, 0);
    lcd_print("Booting...");

    sleep(3);

    lcd_clear();

    // -------------------------------------------------
    // ⭐ INIT MODEL (use absolute path)
    // -------------------------------------------------
    if (tpms_model_init("model_tpms_8.tflite") != 0)
    {
        lcd_set_cursor(0, 0);
        lcd_print("Model Error");
        lcd_set_cursor(1, 0);
        lcd_print("Init Failed");
        while (1);   // Halt but keep message
    }

    // -------------------------------------------------
    // ⭐ START MESSAGE
    // -------------------------------------------------
    scroll_text_line2("Starting TPMS Engine...");
    sleep(2);
    lcd_clear();

    int counter = 0;
    const int SCENARIO_INTERVAL = 1;
    int ts_index = 0;

    // -------- INITIAL SCENARIO --------
    generate_normal(pressure, temperature);

    tpms_model_infer(pressure, temperature, concepts, classes);

    for (ts_index = 0; ts_index < WINDOW_SIZE; ts_index++)
    {
        snprintf(line1, sizeof(line1),
                    "P:%0.1f T:%0.1f",
                    pressure[ts_index],
                    temperature[ts_index]);

        lcd_set_cursor(0, 0);
        lcd_print(line1);

        usleep(20000);   // smoother animation
    }

    // tpms_get_active_concepts(concepts, NUM_CONCEPTS, explanation);
    // tpms_get_action(classes, NUM_CLASSES, action);

    // snprintf(message, sizeof(message),
    //          "%s -> %s", explanation, action);

    // // -------------------------------------------------
    // // ⭐ MAIN LOOP (runs forever)
    // // -------------------------------------------------
    // while (1)
    // {
    //     // ---- Scenario update ----
    //     if (counter >= SCENARIO_INTERVAL)
    //     {
    //         generate_weighted_scenario(pressure, temperature);

    //         tpms_model_infer(pressure, temperature, concepts, classes);

    //         tpms_get_active_concepts(concepts, NUM_CONCEPTS, explanation);
    //         tpms_get_action(classes, NUM_CLASSES, action);

    //         snprintf(message, sizeof(message),
    //                  "%s -> %s", explanation, action);

    //         counter = 0;
    //     }

    //     // ---- Display time-series samples ----
    //     for (ts_index = 0; ts_index < WINDOW_SIZE; ts_index++)
    //     {
    //         snprintf(line1, sizeof(line1),
    //                  "P:%0.1f T:%0.1f",
    //                  pressure[ts_index],
    //                  temperature[ts_index]);

    //         lcd_set_cursor(0, 0);
    //         lcd_print(line1);

    //         usleep(20000);   // smoother animation
    //     }

    //     // ---- Interpretation ----
    //     scroll_text_line2(message);

    //     sleep(1);
    //     counter++;
    // }
}