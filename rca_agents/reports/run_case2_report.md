# RCA Report: run_case2

## Summary
In `run_case2`, the system observed a high-risk FCW condition (`riskScore = 80`) at 72 km/h and 12 m distance, but `warningRequest`, `warningLamp`, `buzzer`, and `displayState` all remained OFF. The root cause is in `ADAS_Project\src\warning.c::EvaluateWarning`, which suppresses warning activation whenever `FEATURE_ENABLE == OFF`; in this run, `FEATURE_ENABLE = 0`, so the function returns `OFF` before checking the risk threshold. Confidence is **high** for the lamp/buzzer/displayState behavior; the separate `displayMessage = "Forward Collision Warning"` remains unexplained by the provided evidence.

## Files & Functions Involved

| File | Function | Role |
|---|---|---|
| `ADAS_Project\src\warning.c` | `EvaluateWarning` | Decides `warningRequest` from `riskScore`, but hard-gates the decision with `FEATURE_ENABLE`. |
| `ADAS_Project\src\warning.c` | `EvaluateBrake` | Reads `riskScore` and `FEATURE_ENABLE`; part of braking decision path, referenced by variable tracing only. |
| `ADAS_Project\src\display.c` | `UpdateWarningLamp` | Sets `hmiData.warningLamp` ON/OFF from `warningRequest`. |
| `ADAS_Project\src\display.c` | `UpdateBrakeLamp` | Reserved/no-op function; does not update HMI brake lamp state. |
| `ADAS_Project\src\display.c` | `UpdateBuzzer` | Sets `hmiData.buzzer` ON if `warningRequest` or `brakeRequest` is ON; otherwise OFF. |
| `ADAS_Project\src\display.c` | `UpdateDisplay` | Sets `hmiData.displayState` to brake, warning, or normal based on `brakeRequest` / `warningRequest`. |
| `ADAS_Project\src\display.c` | `GenerateHMIStatus` | Placeholder for diagnostics/logging; no functional HMI actuation shown in evidence. |

## Code Trace (hop by hop)

| Hop | File | Function | Variables at this point | Expected | Actual | Fault point? |
|---|---|---|---|---|---|---|
| 1 | `ADAS_Project\src\warning.c` | `EvaluateWarning` | `riskScore=80`, `FEATURE_ENABLE=0`, `WARNING_RISK=60` | For this high-risk scenario, `warningRequest` should turn ON because `riskScore >= WARNING_RISK`, assuming feature enabled. | Function checks `FEATURE_ENABLE` first and immediately returns `OFF` because `FEATURE_ENABLE == OFF`; threshold check is never reached, so `warningRequest` remains `0`. | **<-- FAULT** |
| 2 | `ADAS_Project\src\display.c` | `UpdateWarningLamp` | `warningRequest=0`, `hmiData.warningLamp=0` | With upstream `warningRequest = 0`, lamp should be OFF. | Sets `hmiData.warningLamp = OFF`, matching observed output. | No |
| 3 | `ADAS_Project\src\display.c` | `UpdateBuzzer` | `warningRequest=0`, `brakeRequest=0`, `hmiData.buzzer=0` | With both requests OFF, buzzer should be OFF. | Sets `hmiData.buzzer = OFF`, matching observed output. | No |
| 4 | `ADAS_Project\src\display.c` | `UpdateDisplay` | `warningRequest=0`, `brakeRequest=0`, `hmiData.displayState=0` | With both requests OFF, display should go to `DISPLAY_NORMAL` / off-equivalent. | Sets `hmiData.displayState = DISPLAY_NORMAL`, consistent with observed `displayState = 0`. | No |

## Root Cause
The responsible logic is `ADAS_Project\src\warning.c::EvaluateWarning`. Although `riskScore` is high enough to trigger a warning (`80 >= WARNING_RISK 60`), the function contains an earlier guard:

```c
if(FEATURE_ENABLE == OFF)
    return OFF;
```

Because the runtime config has `FEATURE_ENABLE = 0`, `EvaluateWarning` exits early and suppresses `warningRequest`. All downstream HMI outputs then correctly remain OFF as a consequence of that suppressed request.

## Expected vs Actual (Case Comparison)

| Expected behavior per source code logic | Actual behavior observed in this run |
|---|---|
| If FCW feature is intended to be active for this scenario, `EvaluateWarning` should reach the threshold check and set `warningRequest = ON` when `riskScore = 80` and `WARNING_RISK = 60`. | `FEATURE_ENABLE = 0` caused `EvaluateWarning` to return `OFF` before threshold evaluation, so `warningRequest = 0`. |
| With `warningRequest = ON`, `UpdateWarningLamp` would set `hmiData.warningLamp = ON`. | `warningLamp = 0`. |
| With `warningRequest = ON` (or `brakeRequest = ON`), `UpdateBuzzer` would set `hmiData.buzzer = ON`. | `buzzer = 0`. |
| With `warningRequest = ON`, `UpdateDisplay` would set `hmiData.displayState = DISPLAY_WARNING`. | `displayState = 0` (`DISPLAY_NORMAL` / off-equivalent). |
| HMI warning text/state should be consistent with the actuation state controlled by warning/brake requests. | `displayMessage = "Forward Collision Warning"` is present even though lamp, buzzer, and displayState are OFF; this inconsistency is not explained by the traced functions. |

## Supporting Evidence
- **Hop 1:** `variable_readers({'variable_name': 'riskScore'})` showed `riskScore` is read by `EvaluateWarning` and `EvaluateBrake`.
- **Hop 3:** `variable_readers({'variable_name': 'FEATURE_ENABLE'})` showed `FEATURE_ENABLE` is read by `EvaluateWarning` and `EvaluateBrake`.
- **Hop 5:** `get_source_snippet({'function_name': 'EvaluateWarning'})` confirmed `ADAS_Project\src\warning.c::EvaluateWarning` returns `OFF` immediately when `FEATURE_ENABLE == OFF`, before checking `riskScore >= WARNING_RISK`.
- **Hop 6:** `get_source_snippet({'function_name': 'UpdateWarningLamp'})` confirmed `ADAS_Project\src\display.c::UpdateWarningLamp` drives `hmiData.warningLamp` directly from `warningRequest`.
- **Hop 7:** `get_source_snippet({'function_name': 'UpdateBuzzer'})` confirmed `ADAS_Project\src\display.c::UpdateBuzzer` sets `hmiData.buzzer` OFF when both `warningRequest` and `brakeRequest` are OFF.
- **Hop 8:** `get_source_snippet({'function_name': 'UpdateDisplay'})` confirmed `ADAS_Project\src\display.c::UpdateDisplay` sets `hmiData.displayState = DISPLAY_NORMAL` when both requests are OFF.
- **Hop 4:** `variable_writers({'variable_name': 'displayMessage'})` returned no writers, supporting that the displayed warning text comes from a separate, untraced path.

## Affected Component(s)
- **SWC2**: `ADAS_Project\src\warning.c`
- **SWC3**: `ADAS_Project\src\display.c`

## Investigation Effort
8 hop(s) total: 2x variable_readers, 2x variable_writers, 4x get_source_snippet

## Recommended Corrective Actions
1. **Review intended semantics of `FEATURE_ENABLE` in `ADAS_Project\src\warning.c::EvaluateWarning`.**  
   If the FCW feature should be active in this scenario, the runtime/config value `FEATURE_ENABLE = 0` is incorrect and must be corrected in the configuration source.

2. **If current behavior is too aggressive, revise the gating logic in `EvaluateWarning`.**  
   Specifically assess whether a full early return on `FEATURE_ENABLE == OFF` is intended, or whether diagnostic/warning states should still be surfaced under some conditions.

3. **Trace and align the `displayMessage` production path.**  
   Since no writer for `displayMessage` was found in the evidence, inspect the code path that populates `"Forward Collision Warning"` and ensure it is gated consistently with `warningRequest` / `displayState`.

4. **Add an integration check for configuration-vs-HMI consistency.**  
   Create a test case where `riskScore >= WARNING_RISK` and `FEATURE_ENABLE = 0` to explicitly verify that:
   - `warningRequest = 0`
   - `warningLamp = 0`
   - `buzzer = 0`
   - `displayState = DISPLAY_NORMAL`
   - and that `displayMessage` does **not** show warning text unless separately intended.

5. **Add logging/diagnostics around feature gating.**  
   In or around `EvaluateWarning`, log when warning suppression occurs due to `FEATURE_ENABLE == OFF`, so future runs make the reason for suppressed warning explicit.