#ifndef _SGP40_H__
#define _SGP40_H__

typedef struct
{
	uint16_t sensor_raw_value;
} sgp40_results_t;

typedef void (*sgp40_results_handler_t)(sgp40_results_t *p_results);

bool sgp40_turn_heater_off();
bool sgp40_measure_raw_signal();
int sgp40_sensor_init(sgp40_results_handler_t handler, nrf_drv_twi_t const *instance, uint8_t addr);

#endif // _SGP40_H__
