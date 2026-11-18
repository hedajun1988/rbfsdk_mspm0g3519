#ifndef RBF_MC2_H
#define RBF_MC2_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
    RBF_MC2_ANGLE_5_DEGREES = 5,    /**< 5 degrees */
    RBF_MC2_ANGLE_10_DEGREES = 10,  /**< 10 degrees */
    RBF_MC2_ANGLE_15_DEGREES = 15,  /**< 15 degrees */
    RBF_MC2_ANGLE_20_DEGREES = 20,  /**< 20 degrees */
    RBF_MC2_ANGLE_25_DEGREES = 25   /**< 25 degrees */
} rbf_mc2_angle_threshold_t;

/**
 * @brief MC2 external contact processing mode
 */
typedef enum
{
    RBF_MC2_EXTERNAL_MODE_NORMAL = 0,         /**< Normal contact */
    RBF_MC2_EXTERNAL_MODE_ROLLER_SHUTTER = 1  /**< Roller shutter pulse counting */
} rbf_mc2_external_mode_t;

/**
 * @brief MC2 on-device alarm configuration
 */
typedef struct
{
    uint8_t antimasking_enabled;              /**< Magnetic interference detection: 0 - disable, 1 - enable */
    uint8_t angle_alarm_enabled;              /**< Angle alarm detection: 0 - disable, 1 - enable */
    rbf_mc2_angle_threshold_t angle_threshold;/**< Alarm angle, 5-25 degrees */
    uint8_t angle_alarm_delay_seconds;        /**< Angle alarm delay, 1-60 seconds */
    uint8_t shock_alarm_enabled;              /**< Shock alarm detection: 0 - disable, 1 - enable */
    uint8_t shock_level;                      /**< Shock level, 0-3 */
    uint8_t external_input_enabled;           /**< External contact detection: 0 - disable, 1 - enable */
    rbf_mc2_external_mode_t external_mode;    /**< Normal or roller shutter */
    uint8_t external_alarm_delay_seconds;     /**< Roller alarm window, 5-30 seconds */
    uint8_t external_alarm_impulses;          /**< Roller alarm pulse count, 2-7 */
    uint8_t alarm_led_disabled;               /**< Alarm LED: 0 - enabled, 1 - disabled */
    uint8_t tamper_led_disabled;             /**< Tamper LED: 0 enabled, 1 disabled. */
    uint8_t open_door_alarm_disabled;         /**< Door open alarm: 0 - enabled, 1 - disabled */
} rbf_mc2_config_t;


/** Send complete five-byte option. All booleans must be 0 or 1.
 * No implicit read/modify/write: caller supplies all non-LED configuration.
 * set queues; update sends immediately then requests UpdateMe.
 */
int rbf_mc2_set(uint8_t no, const rbf_mc2_config_t *config);
int rbf_mc2_update(uint8_t no, const rbf_mc2_config_t *config);
#ifdef __cplusplus
}
#endif
#endif
