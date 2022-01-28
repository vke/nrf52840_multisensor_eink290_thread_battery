#ifndef _MAX44009_H__
#define _MAX44009_H__

typedef struct
{
	uint32_t lux;
} max44009_results_t;

typedef void (*max44009_results_handler_t)(max44009_results_t *p_results);

int max44009_start_measurement();
int max44009_sensor_init(max44009_results_handler_t handler, nrf_drv_twi_t const *instance, uint8_t addr);

#endif // _MAX44009_H__
