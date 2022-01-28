#ifndef __SETTINGS__H__
#define __SETTINGS__H__

#define INFO_FIRMWARE_TYPE                   "multisensor_eink290b"
#define INFO_FIRMWARE_VERSION                "1.0.0"

#define RADIO_TRANSMIT_POWER                 0

#define SUBSCRIPTION_TIMER_INTERVAL          30000
#define INTERNAL_TEMPERATURE_TIMER_INTERVAL  30000
#define VOLTAGE_TIMER_INTERVAL               30000
#define SENSORS_TIMER_INTERVAL               1000

#define DEFAULT_POLL_PERIOD                  0
#define DEFAULT_POLL_PERIOD_FAST             100
#define DEFAULT_POLL_PERIOD_FAST_TIMEOUT     500
#define DEFAULT_CHILD_TIMEOUT                240

#define ADC_SAMPLES_PER_CHANNEL              4

#define LED_SEND_NOTIFICATION                BSP_BOARD_LED_0
#define LED_RECV_NOTIFICATION                BSP_BOARD_LED_0
#define LED_ROUTER_ROLE                      BSP_BOARD_LED_0
#define LED_CHILD_ROLE                       BSP_BOARD_LED_0

#define TWI_SCL_M                            3
#define TWI_SDA_M                            28

#define DISABLE_OT_ROLE_LIGHTS               1
// #define DISABLE_OT_TRAFFIC_LIGHTS            1

#endif // __SETTINGS__H__
