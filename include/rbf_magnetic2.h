#ifndef RBF_MAGNETIC2_H
#define RBF_MAGNETIC2_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/** Independent MC2 events; callback payloads are valid only during invocation. */
typedef struct {
    uint8_t battery_voltage; /**< Raw protocol BattV. */
    uint8_t power; /**< Battery percentage, using the indoor PIR curve. */
    int32_t rssi; /**< Selected link RSSI in dBm. */
} rbf_magnetic2_heartbeat_t;
typedef struct {
    uint8_t alarm;
    uint8_t antimasking;
    uint8_t angle_alarm;
    uint8_t shock_alarm;
    uint8_t external_alarm;
    uint8_t tamper;
    uint8_t uncalibrated;
} rbf_magnetic2_input_status_t;
typedef int (*rbf_magnetic2_heartbeat_callback_t)(uint8_t no, rbf_magnetic2_heartbeat_t* heartbeat);
typedef int (*rbf_magnetic2_input_status_update_callback_t)(uint8_t no, rbf_magnetic2_input_status_t* input_status);
typedef struct {
    rbf_magnetic2_heartbeat_callback_t hb_cb;
    rbf_magnetic2_input_status_update_callback_t input_status_cb;
} rbf_magnetic2_callbacks_t;
/** Copies callbacks. NULL unregisters this family only; returns 0.
 * Register before event processing; concurrent registration is unsupported.
 */
int rbf_magnetic2_register_callbacks(rbf_magnetic2_callbacks_t* cbs);

typedef enum
{
    RBF_MAGNETIC2_ANGLE_5_DEGREES = 5,    /**< 5 degrees */
    RBF_MAGNETIC2_ANGLE_10_DEGREES = 10,  /**< 10 degrees */
    RBF_MAGNETIC2_ANGLE_15_DEGREES = 15,  /**< 15 degrees */
    RBF_MAGNETIC2_ANGLE_20_DEGREES = 20,  /**< 20 degrees */
    RBF_MAGNETIC2_ANGLE_25_DEGREES = 25   /**< 25 degrees */
} rbf_magnetic2_angle_threshold_t;

/**
 * @brief MC2 external contact processing mode
 */
typedef enum
{
    RBF_MAGNETIC2_EXTERNAL_MODE_NORMAL = 0,         /**< Normal contact */
    RBF_MAGNETIC2_EXTERNAL_MODE_ROLLER_SHUTTER = 1  /**< Roller shutter pulse counting */
} rbf_magnetic2_external_mode_t;

/**
 * @brief MC2 on-device alarm configuration
 */
typedef struct
{
    uint8_t antimasking_enabled;              /**< Magnetic interference detection: 0 - disable, 1 - enable */
    uint8_t angle_alarm_enabled;              /**< Angle alarm detection: 0 - disable, 1 - enable */
    rbf_magnetic2_angle_threshold_t angle_threshold;/**< Alarm angle, 5-25 degrees */
    uint8_t angle_alarm_delay_seconds;        /**< Angle alarm delay, 1-60 seconds */
    uint8_t shock_alarm_enabled;              /**< Shock alarm detection: 0 - disable, 1 - enable */
    uint8_t shock_level;                      /**< Shock level, 0-2 */
    uint8_t external_input_enabled;           /**< External contact detection: 0 - disable, 1 - enable */
    rbf_magnetic2_external_mode_t external_mode;    /**< Normal or roller shutter */
    uint8_t external_alarm_delay_seconds;     /**< Roller alarm window, 5-30 seconds */
    uint8_t external_alarm_impulses;          /**< Roller alarm pulse count, 2-7 */
    uint8_t alarm_led_disabled;               /**< Alarm LED: 0 - enabled, 1 - disabled */
    uint8_t tamper_led_disabled;             /**< Tamper LED: 0 enabled, 1 disabled. */
    uint8_t open_door_alarm_disabled;         /**< Door open alarm: 0 - enabled, 1 - disabled */
} rbf_magnetic2_config_t;


/** Send complete five-byte option. All booleans must be 0 or 1.
 * No implicit read/modify/write: caller supplies all non-LED configuration.
 * set queues; update sends immediately then requests UpdateMe.
 */
int rbf_magnetic2_set(uint8_t no, const rbf_magnetic2_config_t *config);
int rbf_magnetic2_update(uint8_t no, const rbf_magnetic2_config_t *config);
#ifdef __cplusplus
}
#endif
#endif
