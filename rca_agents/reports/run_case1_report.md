# RCA Report: run_case1

## Summary
The observed run is a normal forward-collision warning scenario: `riskScore=80` is above `WARNING_RISK=60` and below `BRAKE_RISK=85`, resulting in `warningRequest=1` and `brakeRequest=0`. The available evidence shows the expected decision path through `SWC2_Run -> EvaluateWarning`, followed by HMI update functions that consume `warningRequest` and activate the lamp, buzzer, and display. No software fault is indicated in this run. Confidence is **medium** because the investigation did not retrieve source snippets or full file mappings for the involved functions.

## Files & Functions Involved

| File | Function | Role |
|---|---|---|
| Unknown | `main` | Entry point for the logged scenario execution path. |
| Unknown | `SWC2_Run` | Invokes warning evaluation for the FEB warning decision path. |
| Unknown | `EvaluateWarning` | Reads `riskScore` and evaluates it against `WARNING_RISK` with `FEATURE_ENABLE` gating to determine warning behavior. |
| Unknown | `EvaluateBrake` | Reads `riskScore` and evaluates it against `BRAKE_RISK` to determine whether brake intervention should remain inactive or trigger. |
| Unknown | `UpdateWarningLamp` | Reads `warningRequest` and drives `warningLamp`. |
| Unknown | `UpdateBuzzer` | Reads `warningRequest` and drives `buzzer`. |
| Unknown | `UpdateDisplay` | Reads `warningRequest` and drives display warning state/message. |
| `src/config.h` | `FEATURE_ENABLE` | Configuration constant used by `EvaluateWarning` to gate feature behavior. |
| `src/config.h` | `WARNING_RISK` | Warning threshold constant compared against `riskScore` in `EvaluateWarning`. |
| `src/config.h` | `ON` | Constant used in warning/HMI state comparisons. |
| `src/config.h` | `OFF` | Constant used in warning/HMI state comparisons. |

## Code Trace (hop by hop)

| Hop | File | Function | Variables at this point | Expected | Actual | Fault point? |
|---|---|---|---|---|---|---|
| 1 | Unknown | `main` | `scenario_id=run_case1` | The application should execute the normal ADAS cycle for the warning scenario. | The run proceeds into the normal ADAS execution path for the logged scenario. | No |
| 2 | Unknown | `SWC2_Run` | `riskScore=80`, `FEATURE_ENABLE=1`, `WARNING_RISK=60`, `BRAKE_RISK=85` | `SWC2_Run` should invoke the warning evaluation logic for the current FEB risk state. | Evidence shows `SWC2_Run` calls `EvaluateWarning`, consistent with the expected warning-decision path. | No |
| 3 | Unknown | `EvaluateWarning` | `riskScore=80`, `WARNING_RISK=60`, `FEATURE_ENABLE=1`, `warningRequest=1` | Because `FEATURE_ENABLE=1` and `riskScore=80 >= WARNING_RISK=60`, the function should assert `warningRequest` to `ON`. | The observed `warningRequest` is `1`, which matches the expected threshold decision. Evidence hop 1 confirms `riskScore` is checked against `WARNING_RISK` here. | No |
| 4 | Unknown | `EvaluateBrake` | `riskScore=80`, `BRAKE_RISK=85`, `brakeRequest=0` | Because `riskScore=80 < BRAKE_RISK=85`, the brake path should remain inactive. | The observed `brakeRequest` is `0`, matching the expected non-brake outcome for this scenario. | No |
| 5 | Unknown | `UpdateWarningLamp` | `warningRequest=1`, `warningLamp=1` | If `warningRequest == ON`, the warning lamp should be turned on. | Evidence hop 3 shows this function reads `warningRequest`, and the observed `warningLamp` is `1` as expected. | No |
| 6 | Unknown | `UpdateBuzzer` | `warningRequest=1`, `buzzer=1` | If `warningRequest == ON`, the buzzer should be activated. | Evidence hop 3 shows this function reads `warningRequest`, and the observed `buzzer` is `1` as expected. | No |
| 7 | Unknown | `UpdateDisplay` | `warningRequest=1`, `displayState=1`, `displayMessage="Forward Collision Warning"` | If `warningRequest == ON`, the display should enter warning state and show the forward collision warning message. | Evidence hop 3 shows this function reads `warningRequest`, and the observed display state/message match the expected warning output. | No |

## Root Cause
No root-cause defect was identified for `run_case1`. The behavior is consistent with the intended implementation: `EvaluateWarning` correctly treats `riskScore=80` as above `WARNING_RISK=60`, while `EvaluateBrake` correctly leaves braking inactive because `riskScore=80` is below `BRAKE_RISK=85`. The HMI path (`UpdateWarningLamp`, `UpdateBuzzer`, `UpdateDisplay`) then correctly reflects `warningRequest=1`.

## Expected vs Actual (Case Comparison)

Since only one case is available, the comparison below shows expected behavior from the available source-logic evidence versus actual observed behavior.

| Expected behavior per source code logic | Actual behavior observed in this run |
|---|---|
| `riskScore >= WARNING_RISK` in `EvaluateWarning` should make the warning path active when `FEATURE_ENABLE=1`. | `riskScore=80`, `WARNING_RISK=60`, `FEATURE_ENABLE=1`, and `warningRequest=1`. |
| `riskScore >= BRAKE_RISK` in `EvaluateBrake` should remain false for `riskScore=80` when `BRAKE_RISK=85`. | `brakeRequest=0`. |
| `UpdateWarningLamp` should turn the lamp on when `warningRequest == ON`. | `warningLamp=1`. |
| `UpdateBuzzer` should activate the buzzer when `warningRequest == ON`. | `buzzer=1`. |
| `UpdateDisplay` should present warning state/message when `warningRequest == ON`. | `displayState=1`, `displayMessage="Forward Collision Warning"`. |

## Supporting Evidence
- **Hop 1:** `variable_readers({'variable_name': 'riskScore'})` confirmed `riskScore` is read by `EvaluateWarning` via `riskScore >= WARNING_RISK` and by `EvaluateBrake` via `riskScore >= BRAKE_RISK`.
- **Hop 3:** `variable_readers({'variable_name': 'warningRequest'})` confirmed `warningRequest` is read by `UpdateWarningLamp`, `UpdateBuzzer`, and `UpdateDisplay` via `warningRequest == ON`.
- **Hop 5:** `variable_readers({'variable_name': 'warningLamp'})` linked `warningLamp` to `UpdateWarningLamp`.
- **Hop 6:** `variable_readers({'variable_name': 'buzzer'})` linked `buzzer` to `UpdateBuzzer`.
- **Hop 7:** `find_callers({'function_name': 'EvaluateWarning'})` confirmed `SWC2_Run` calls `EvaluateWarning`.
- **Hop 8:** `find_callees({'function_name': 'EvaluateWarning'})` showed dependencies on `src/config.h:WARNING_RISK`, `src/config.h:FEATURE_ENABLE`, `src/config.h:ON`, and `src/config.h:OFF`, supporting the threshold-and-gating interpretation.
- Observation log: `riskScore=80`, `warningRequest=1`, `brakeRequest=0`, `warningLamp=1`, `buzzer=1`, and `displayMessage="Forward Collision Warning"` all align with the expected warning-only path.

## Affected Component(s)
- **SWC2**: `SWC2_Run`, `EvaluateWarning`, `EvaluateBrake`
- **SWC3 / HMI path**: `UpdateWarningLamp`, `UpdateBuzzer`, `UpdateDisplay`
- **Configuration**: `src/config.h`

## Investigation Effort
8 hop(s) total: 4x variable_readers, 2x variable_writers, 1x find_callers, 1x find_callees

## Recommended Corrective Actions
- No corrective code change is warranted based on `run_case1`; this run behaves as expected.
- To raise confidence from **medium** to **high**, retrieve source snippets for:
  - `EvaluateWarning`
  - `EvaluateBrake`
  - `UpdateWarningLamp`
  - `UpdateBuzzer`
  - `UpdateDisplay`
- Also retrieve file locations for `main`, `SWC2_Run`, `EvaluateWarning`, `EvaluateBrake`, `UpdateWarningLamp`, `UpdateBuzzer`, and `UpdateDisplay`, since the current evidence does not provide concrete source paths for those functions.
- If this scenario is used as a regression reference, preserve it as a known-good warning-only test case with the key values:
  - `riskScore=80`
  - `WARNING_RISK=60`
  - `BRAKE_RISK=85`
  - expected outputs: `warningRequest=1`, `brakeRequest=0`, `warningLamp=1`, `buzzer=1`, `displayState=1`, `displayMessage="Forward Collision Warning"`