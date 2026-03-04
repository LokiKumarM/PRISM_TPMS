# Functional Specification Document (FSD)
## Traditional Rule-Based TPMS Software

---

## 1. Purpose

This document defines the functional behavior and decision-making rules
for a Traditional Threshold-Based Tire Pressure Monitoring System (TPMS).

The document serves as a baseline reference implementation
for comparison against model-driven TPMS systems.

---

## 2. System Overview

The Traditional TPMS continuously monitors:

- Tire Pressure (psi)
- Tire Temperature (°C)
- Time-series behavior

The system evaluates predefined threshold rules
to classify the tire condition into one of the following states:

- NORMAL
- NORMAL_THERMAL
- SLOW_LEAK
- FAST_LEAK
- UNDERINFLATED
- SENSOR_FAULT

The system is deterministic and rule-based.

---

## 3. Inputs

| Parameter      | Type   | Description |
|---------------|--------|-------------|
| Pressure      | float  | Instantaneous tire pressure (kPa) |
| Temperature   | float  | Instantaneous tire temperature (°C) |

The system shall maintain a sliding window buffer of recent 64 samples.

---

## 4. Outputs

| Output Field | Type | Description |
|-------------|------|-------------|
| Tire Status | Enum | Classification result |
| Warning Flag | Boolean | Indicates alert condition |

---

## 5. Definitions and Constants

### 5.1 Placard Pressure
Manufacturer recommended cold inflation pressure.

### 5.2 Nominal Pressure
Reference pressure used for normal operating condition.

### 5.3 Default Threshold Parameters

These values may be configurable:

- LOW_PRESSURE_THRESHOLD = 0.80 × Placard Pressure
- CRITICAL_PRESSURE_THRESHOLD = 0.75 × Placard Pressure
- FAST_LEAK_DROP = 5 kPa within short window
- SLOW_LEAK_PERCENT_DROP = 5% gradual drop
- OVERHEAT_THRESHOLD = 75°C
- NORMAL_VARIATION = ±5%
- THERMAL_VARIATION = +8%

---

## 6. Functional Rules

---

### 6.1 NORMAL Condition

The system shall classify tire status as **NORMAL** when:

- Pressure within ±5% of nominal pressure
- Temperature within normal operating range
- No abnormal pressure drop detected
- No sensor fault present

**Output:**
- Status = NORMAL
- Warning Flag = FALSE

---

### 6.2 NORMAL_THERMAL Condition

The system shall classify tire status as **NORMAL_THERMAL** when:
 
- Temperature deviates from normal range
- Pressure changes as Temperature changes
- No pressure drop trend detected

This condition accounts for thermal expansion.

**Output:**
- Status = NORMAL_THERMAL
- Warning Flag = FALSE

---

### 6.3 SLOW_LEAK Detection

The system shall classify tire status as **SLOW_LEAK** when:

- Gradual pressure decrease detected over sliding window
- Total drop exceeds slow leak percentage threshold
- Drop rate does not meet fast leak criteria

Example condition:
if (pressure_drop_percent > SLOW_LEAK_PERCENT_DROP
AND drop_rate < FAST_LEAK_DROP)


**Output:**
- Status = SLOW_LEAK
- Warning Flag = TRUE

---

### 6.4 FAST_LEAK Detection

The system shall classify tire status as **FAST_LEAK** when:

- Pressure drop ≥ FAST_LEAK_DROP within defined short window
- Drop cannot be explained by thermal variation

Example condition:
if (pressure(t0) - pressure(t1) >= FAST_LEAK_DROP)


**Output:**
- Status = FAST_LEAK
- Warning Flag = TRUE

---

### 6.5 UNDERINFLATED Condition

The system shall classify tire status as **UNDERINFLATED** when:
pressure < CRITICAL_PRESSURE_THRESHOLD


Regulatory reference typically requires warning at
75–80% of placard pressure.

**Output:**
- Status = UNDERINFLATED
- Confidence ≥ 0.95
- Warning Flag = TRUE

---

### 6.7 SENSOR_FAULT Condition

The system shall classify status as **SENSOR_FAULT** when:

- Pressure reading is outside physical limits (e.g., <0 or >100 psi)
- Temperature outside physical limits
- No data received within timeout window
- Identical readings repeated for abnormal duration
- Data checksum/CRC invalid

**Output:**
- Status = SENSOR_FAULT
- Confidence = 1.0
- Warning Flag = TRUE

---

## 7. Sliding Window and Time-Series Processing

The system shall:

- Maintain a buffer of last 64 samples
- Compute:
  - Average pressure
  - Pressure delta
  - Pressure drop rate
  - Temperature trend

Window size shall be configurable.

## 9. State Priority Order

If multiple rules trigger simultaneously,
priority shall be evaluated in the following order:

1. SENSOR_FAULT
2. UNDERINFLATED
3. FAST_LEAK
4. SLOW_LEAK
6. NORMAL_THERMAL
7. NORMAL

---

## 10. Performance Requirements

- Decision latency < 10 ms
- Deterministic rule evaluation
- No dynamic memory allocation
- Suitable for embedded real-time execution

---

## 11. Assumptions and Limitations

- Static thresholds require calibration
- Thermal compensation is simplified
- No pattern learning capability
- False positives possible under noisy sensor data

---

## 12. Intended Use

This specification defines a traditional baseline
for comparison against AI-based TPMS systems
in terms of:

- False positive rate
- False negative rate
- Code complexity
- Decision latency
- Robustness to noise

---

End of Document

