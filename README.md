# PRISM_TPMS

**Explainable tyre-health monitoring — from raw pressure-sensor signals to a fault decision *with a reason*, light enough to run on embedded hardware.**

PRISM_TPMS takes a window of raw tyre pressure and temperature readings and classifies the tyre's condition. Crucially, it doesn't just output a verdict — it reports the *concepts* that drove the decision (e.g. "pressure trending down", "fast drop rate", "temperature high"), so the result is auditable rather than a black box. It ships with two interchangeable decision engines and a benchmark that proves the trade-offs between them.

---

## Key Features

- **Two decision engines, one interface**
  - **Rule-based:** deterministic threshold logic (the classic TPMS approach), fully specified in [`TPMS_FSD.md`](TPMS_FSD.md).
  - **Model-based:** a quantized TensorFlow Lite **concept-bottleneck** model (`model_tpms_8.tflite`) that predicts 11 interpretable concepts and a 6-class status.
- **Explainability built in.** Every decision is accompanied by the active concepts and a context string, not just a label.
- **Embedded-ready.** Plain C, a tiny `.tflite` model, fixed-size buffers, and an I2C LCD output path (`lcd_i2c.c/.h`). No heavyweight runtime required.
- **Sensor simulation.** Generates realistic NORMAL, thermal, slow-leak, fast-leak, under-inflated and sensor-fault signals for testing without hardware.
- **Built-in benchmarking.** `tpms_analysis.c` runs both engines over edge-case scenarios and reports accuracy, false positives, latency and complexity.

---

## How It Works

Pressure[64] ┐
├─► [ Sliding-window buffer ] ─► [ Decision engine ] ─► Status + Reason ─► LCD / stdout
Temp[64]     ┘                                   │
├─ Rule-based:  threshold rules (FSD)
└─ Model-based: TFLite → 11 concepts + 6 class probs → decision layer

The system maintains a **64-sample** sliding window of two features (pressure, temperature).

**Status classes (6):**
`NORMAL`, `NORMAL_THERMAL`, `SLOW_LEAK`, `FAST_LEAK`, `UNDERINFLATED`, `SENSOR_FAULT`

**Model concepts (11)** — the explainability layer:
`pressure_stable`, `pressure_trend_up`, `pressure_trend_down`, `drop_rate_slow`, `drop_rate_fast`, `pressure_low`, `temp_trend_up`, `temp_trend_down`, `temp_high`, `temp_low`, `sensor_data_invalid`

The decision layer (`tpms_decision.c`) consumes the concept vector and class probabilities and produces the final status, a list of active concepts, a context string and a confidence value.

---

## Repository Structure

| File | Purpose |
|------|---------|
| `tpms_main.c` | Application entry point: init → simulate → infer → decide → display, with latency timing. |
| `tpms_model.c` / `tpms_model.h` | TFLite model wrapper: `tpms_model_init`, `tpms_model_infer`, `tpms_model_deinit`. |
| `tpms_decision.c` / `tpms_decision.h` | Turns concepts + class probabilities into an explainable status, context and confidence. |
| `tpms_sw_rulebased.c` / `tpms_sw_rulebased.h` | Traditional threshold/rule-based engine. |
| `sensor_simulation.c` / `sensor_simulation.h` | Signal generators for each fault scenario (normal, thermal, slow/fast leak, under-inflated, sensor fault). |
| `tpms_analysis.c` | Benchmark harness comparing rule-based vs model-based on edge cases. |
| `lcd_i2c.c` / `lcd_i2c.h` | I2C LCD driver for on-device output. |
| `model_tpms_8.tflite` | Trained, quantized concept-bottleneck model. |
| `TPMS_FSD.md` | Functional Specification for the rule-based engine (thresholds, states, rules). |
| `COMPLEXITY_ANALYSIS.md` | Rule-based vs model-based comparison and results. |
| `tpms_rule`, `tpms_compare` | Prebuilt binaries (rule-based demo and comparison benchmark). |

---

## Rule-Based vs Model-Based

From [`COMPLEXITY_ANALYSIS.md`](COMPLEXITY_ANALYSIS.md):

| Aspect | Rule-Based | Model-Based |
|--------|-----------|-------------|
| Code complexity | ~150 LOC, 11 thresholds | ~50 LOC, learned params |
| False positives (boundary cases) | 9–15% | 2–5% (estimated) |
| Threshold tuning | Manual, error-prone | Automatic via training |
| Thermal compensation | Explicit rules | Learned correlation |
| Sensor-variation robustness | Low | High |
| Runtime latency | ~0.005 ms | ~0.001 ms |

> Figures are from the project's own benchmark/analysis and depend on the test scenarios; treat the model-based false-positive range as an estimate.

---

## Building

> Requires a C toolchain (e.g. `gcc`). The model-based path links against a TensorFlow Lite C runtime; the rule-based path and simulation are dependency-free.

```bash
# Rule-based demo (no ML runtime needed)
gcc tpms_sw_rulebased.c sensor_simulation.c -o tpms_rule

# Comparison benchmark (rule-based vs model-based)
gcc tpms_analysis.c tpms_model.c tpms_decision.c tpms_sw_rulebased.c \
    sensor_simulation.c -o tpms_compare -ltensorflowlite_c

# Full application with LCD output
gcc tpms_main.c tpms_model.c tpms_decision.c sensor_simulation.c \
    lcd_i2c.c -o tpms -ltensorflowlite_c
```

Prebuilt binaries `tpms_rule` and `tpms_compare` are included for convenience.

---

## Usage

```bash
# Run the comparison benchmark
./tpms_compare

# Run the rule-based demo
./tpms_rule

# Run the full pipeline (expects the .tflite model and an I2C LCD)
./tpms
```

The model path is passed to `tpms_model_init(model_path)` — point it at `model_tpms_8.tflite`.

---

## Status & Notes

This is a research/comparison project demonstrating that a small learned model can match or beat hand-tuned thresholds while remaining explainable and embedded-friendly. The TFLite runtime is an external dependency for the model-based path; adjust the link flags to match your platform's TensorFlow Lite C library.

## License

No license file is currently present in the repository. Add one (e.g. MIT or Apache-2.0) to clarify reuse terms.
