#ifndef CONFIG_H
#define CONFIG_H

/* Feature Flags */
#define FEATURE_ENABLE              1

/* Sensor Limits */
#define MAX_SPEED                   180
#define MIN_SPEED                   0

#define MAX_DISTANCE                200
#define MIN_DISTANCE                0

/* Warning Thresholds */

#define WARNING_DISTANCE            15
#define BRAKE_DISTANCE              8

#define WARNING_RISK                60
#define BRAKE_RISK                  85

/* Display States */

#define DISPLAY_NORMAL              0
#define DISPLAY_WARNING             1
#define DISPLAY_BRAKE               2

/* Outputs */

#define OFF                         0
#define ON                          1

#endif