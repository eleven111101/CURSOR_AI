# RCA Report: run_case3

## Summary
In `run_case3`, the FEB backend behaved correctly: `riskScore=100` exceeded both thresholds, `warningRequest=1`, `brakeRequest=1`, and the actuator confirms braking was physically applied. The failure is isolated to the SWC3 HMI brake-indicator path: `UpdateBrakeLamp` in `display.c` receives `brakeRequest` but does not use it meaningfully, and the HMI state has no `brakeLamp` field to represent a dedicated brake lamp. Confidence is **high**.

## Files & Functions Involved

| File | Function | Role |
|---|---|---|
| `display.c` | `UpdateDisplay` | Consumes HMI requests and updates display text/state; shows `"Brake Engaged"` in this run. |
| `display.c` | `UpdateBrakeLamp` | Intended brake-lamp update point, but evidence shows it only references `brakeRequest` as `(void)brakeRequest` and does not update HMI state. |
| `null` | `main` | Top-level runtime entry in the call trace; starts FEB flow with enabled feature and critical inputs. |
| `null` | `SWC2_Run` | SWC2 execution stage where risk-based warning/brake evaluation is performed. |
| `null` | `EvaluateWarning` | Reads `riskScore` using `riskScore >= WARNING_RISK` and drives `warningRequest` behavior. |
| `null` | `EvaluateBrake` | Reads `riskScore` using `riskScore >= BRAKE_RISK` and drives `brakeRequest` behavior. |
| `null` | `SWC3_Run` | SWC3 execution stage that propagates warning/brake requests into HMI outputs. |
| `null` | `UpdateBuzzer` | Actively uses `warningRequest` / `brakeRequest` and correctly activates buzzer output. |
| `null` | `UpdateWarningLamp` | Actively uses `warningRequest` and correctly lights the warning lamp. |
| `common.h` | `HMIData` | HMI data structure noted in the observation; does **not** contain a `brakeLamp` field, preventing dedicated brake-lamp state storage. |

## Code Trace (hop by hop)

| Hop | File | Function | Variables at this point | Expected | Actual | Fault point? |
|---|---|---|---|---|---|---|
| 1 | `null` | `main` | `FEATURE_ENABLE=1`, `vehicleSpeed=65`, `objectDistance=6`, `relativeSpeed=65`, `timeToCollision=0` | Runtime should execute the FEB pipeline with feature enabled and critical sensor inputs propagated downstream. | The scenario runs with `FEATURE_ENABLE=1` and critical inputs present; downstream FEB processing occurs. |  |
| 2 | `null` | `SWC2_Run` | `riskScore=100`, `WARNING_RISK=60`, `BRAKE_RISK=85` | SWC2 should evaluate warning and brake conditions from the computed `riskScore`. | SWC2 path is active; evidence shows `riskScore` is consumed by `EvaluateWarning` and `EvaluateBrake`. |  |
| 3 | `null` | `EvaluateWarning` | `riskScore=100`, `WARNING_RISK=60`, `warningRequest=1` | Because `riskScore >= WARNING_RISK`, `warningRequest` should be asserted. | `warningRequest` is `1` in the raw log, matching the threshold logic. |  |
| 4 | `null` | `EvaluateBrake` | `riskScore=100`, `BRAKE_RISK=85`, `brakeRequest=1` | Because `riskScore >= BRAKE_RISK`, `brakeRequest` should be asserted. | `brakeRequest` is `1` in the raw log, matching the threshold logic. |  |
| 5 | `null` | `SWC3_Run` | `warningRequest=1`, `brakeRequest=1` | SWC3 should propagate `warningRequest`/`brakeRequest` into all intended HMI outputs. | SWC3 HMI path is active; display and buzzer reflect the requests, proving propagation into at least part of HMI. |  |
| 6 | `display.c` | `UpdateDisplay` | `warningRequest=1`, `brakeRequest=1`, `displayState=2`, `displayMessage="Brake Engaged"` | Display should reflect active warning/brake state when requests are ON. | Display shows `"Brake Engaged"` with `displayState=2`, so this HMI branch behaves correctly. |  |
| 7 | `null` | `UpdateBuzzer` | `warningRequest=1`, `brakeRequest=1`, `buzzer=1` | Buzzer should activate when warning/brake request is ON. | `buzzer=1`, matching expected HMI behavior. |  |
| 8 | `null` | `UpdateWarningLamp` | `warningRequest=1`, `warningLamp=1` | Warning lamp should be turned on when `warningRequest == ON`. | `warningLamp=1`, confirming warning lamp handling works. |  |
| 9 | `display.c` | `UpdateBrakeLamp` | `brakeRequest=1`, `hmiData.brakeLamp=null` | A dedicated brake-lamp HMI state should be updated when `brakeRequest == ON` so the dashboard lamp can light. | Function only references `brakeRequest` as `(void)brakeRequest`; no HMI state is updated, and `HMIData` has no `brakeLamp` field to carry the result. | **<-- FAULT** |

## Root Cause
The root cause is in **`display.c` / `UpdateBrakeLamp`**. Although `brakeRequest` is correctly generated and propagated into SWC3, `UpdateBrakeLamp` is effectively a stub (`(void)brakeRequest`) and does not write any dedicated brake-lamp HMI state. This is compounded by **`common.h` `HMIData`** lacking a `brakeLamp` field, so there is no data path for a dedicated brake lamp to be stored and displayed.

## Expected vs Actual (Case Comparison)

| Expected behavior per source code logic | Actual behavior observed in this run |
|---|---|
| `riskScore=100` should trigger warning because `riskScore >= WARNING_RISK (60)`. | `warningRequest=1` observed. |
| `riskScore=100` should trigger braking because `riskScore >= BRAKE_RISK (85)`. | `brakeRequest=1` observed. |
| Active brake request should propagate to HMI outputs. | Partial propagation observed: display shows `"Brake Engaged"` and `buzzer=1`. |
| Dedicated brake lamp should have a writable HMI state and be turned on when `brakeRequest == ON`. | No `brakeLamp` field exists in `HMIData`; dedicated brake lamp is not lit / not shown. |
| `UpdateBrakeLamp` should map `brakeRequest` into HMI state. | `UpdateBrakeLamp(brakeRequest)` is a no-op using `(void)brakeRequest`. |
| Backend brake action and HMI brake indication should be consistent. | Backend brake applied (`brakeApplied=true`), but no dedicated brake lamp indication is present. |

## Supporting Evidence
- **Hop 1:** `variable_readers(riskScore)` showed `EvaluateBrake` reads `riskScore >= BRAKE_RISK` and `EvaluateWarning` reads `riskScore >= WARNING_RISK`.
- **Hop 3:** `variable_readers(warningRequest)` showed `UpdateWarningLamp`, `UpdateBuzzer`, and `UpdateDisplay` consume warning state.
- **Hop 5:** `variable_readers(brakeRequest)` showed `UpdateDisplay` and `UpdateBuzzer` actively use `brakeRequest == ON`, while `UpdateBrakeLamp` only uses `(void)brakeRequest`.
- **Hop 7 / Hop 8:** No graph reader/writer evidence was found for `brakeApplied`, which supports keeping the RCA focused on the HMI indication path rather than software actuation tracing.
- Raw log: `riskScore=100`, `WARNING_RISK=60`, `BRAKE_RISK=85`, `warningRequest=1`, `brakeRequest=1`.
- Raw log actuator section: `brakeApplied=true` and `"Brake Actuator : APPLIED"` confirm the brake decision was physically executed.
- Raw log/HMI note: `HMIData` has no `brakeLamp` field and `UpdateBrakeLamp(brakeRequest)` does not write any HMI state.

## Affected Component(s)
- **SWC3**
- **`display.c`**
- **`common.h` `HMIData` definition**

## Investigation Effort
8 hop(s) total: 4x variable_readers, 4x variable_writers

## Recommended Corrective Actions
1. **Implement `UpdateBrakeLamp` in `display.c`** so it performs a real state update instead of `(void)brakeRequest;`.
2. **Add a dedicated `brakeLamp` field to `HMIData` in `common.h`** so brake-indicator state can be stored and consumed by the HMI/rendering path.
3. **Wire `brakeRequest` to `hmiData.brakeLamp` explicitly** inside `UpdateBrakeLamp`, with ON/OFF behavior consistent with the warning-lamp pattern.
4. **Verify SWC3 integration** so the dedicated brake lamp is rendered from `HMIData.brakeLamp`, not inferred only from display text.
5. **Add regression coverage** for the exact case seen here: `FEATURE_ENABLE=1`, `riskScore >= BRAKE_RISK`, `brakeRequest=1`, and expected result `brakeLamp=1`.
6. **Add a consistency check/test** ensuring that when backend braking is requested and/or actuator state is applied, the HMI dedicated brake indicator is present if required by design.