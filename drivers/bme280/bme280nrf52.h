#ifndef _BME280_H__
#define _BME280_H__

typedef struct
{
	uint32_t pressure;
	uint32_t temperature;
	uint32_t humidity;
} bme280_results_t;

typedef void (*bme280_results_handler_t)(bme280_results_t *p_results);

int bme280_start_measurement();
int bme280_sensor_init(bme280_results_handler_t handler, nrf_drv_twi_t const *instance, uint8_t addr);

#endif // _BME280_H__
