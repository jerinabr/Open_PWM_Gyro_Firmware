/*!
    @file:	sys_config.h
    @brief:	System configuration defaults
*/
#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Stabilization update period in milliseconds */
#define OUTPUT_UPDATE_PERIOD 20

/* Normalized control limits */
#define ROLL_CONTROL_LIMIT  1.0f
#define PITCH_CONTROL_LIMIT 1.0f
#define YAW_CONTROL_LIMIT   1.0f

/* Angle limits in degrees */
#define ROLL_ANG_LIMIT  45.0f
#define PITCH_ANG_LIMIT 45.0f

/* Per-axis stabilization gains */
#define ROLL_GAIN   0.25f
#define PITCH_GAIN  0.25f
#define YAW_GAIN    1.0f

/* Stabilization adjustment direction reversal */
#define ROLL_ADJ_REVERSE    0
#define PITCH_ADJ_REVERSE   0
#define YAW_ADJ_REVERSE     1

/* Channel settings */
#define CHANNEL_CENTER      1500
#define CHANNEL_MAX_THROW   500
#define CHANNEL_DEADBAND    20
#define GAIN_CHANNEL_MIN    1000
#define GAIN_CHANNEL_WIDTH  1000

/* Control input channel mappings (0-indexed) */
#define ROLL_CHANNEL    0
#define PITCH_CHANNEL   1
#define YAW_CHANNEL     3
#define MODE_CHANNEL    5
#define GAIN_CHANNEL    6

/* Control output channel mappings (0-indexed)
    Anything defined as NUM_OUTPUT_CHANNELS indicates it's unused by default */
#define AILERON1_CHANNEL    0
#define AILERON2_CHANNEL    NUM_OUTPUT_CHANNELS
#define ELEVATOR1_CHANNEL   1
#define ELEVATOR2_CHANNEL   NUM_OUTPUT_CHANNELS
#define RUDDER1_CHANNEL     3
#define RUDDER2_CHANNEL     NUM_OUTPUT_CHANNELS
#define ELEVON1_CHANNEL     NUM_OUTPUT_CHANNELS
#define ELEVON2_CHANNEL     NUM_OUTPUT_CHANNELS
#define FLAPERON1_CHANNEL   NUM_OUTPUT_CHANNELS
#define FLAPERON2_CHANNEL   NUM_OUTPUT_CHANNELS
#define CANARD1_CHANNEL     NUM_OUTPUT_CHANNELS
#define CANARD2_CHANNEL     NUM_OUTPUT_CHANNELS

/* Mode corresponding to each position of a 3-way switch input */
#define SWITCH_POS_1_MODE   GYRO_MODE_OFF
#define SWITCH_POS_2_MODE   GYRO_MODE_STABILIZE
#define SWITCH_POS_3_MODE   GYRO_MODE_STABILIZE

#ifdef __cplusplus
}
#endif

#endif