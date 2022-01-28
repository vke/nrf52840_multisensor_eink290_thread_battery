#include "nrf.h"
#include "bsp.h"
#include "nrf_log.h"
#include "settings.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "bosch_bme280_driver/bme280.h"
#include "app_timer.h"
#include "bme280nrf52.h"

#include <openthread/platform/alarm-milli.h>

static bme280_results_handler_t results_handler = NULL;
static nrf_drv_twi_t const *p_sensor_twi_instance = NULL;
static uint32_t bme280_measurement_delay = 0;
static struct bme280_dev bme280_sensor;
static uint8_t bme280_sensor_addr = 0;
static bool bme280_sensor_initialized = false;

APP_TIMER_DEF(m_bme280_measurement_delay_timer_id);


static void user_delay_us(uint32_t period, void *intf_ptr)
{
	nrf_delay_us(period);
}

static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	ret_code_t err_code = nrf_drv_twi_tx(p_sensor_twi_instance, *(uint8_t *)intf_ptr, &reg_addr, 1, false);
	
	APP_ERROR_CHECK(err_code);
	
	err_code = nrf_drv_twi_rx(p_sensor_twi_instance, *(uint8_t *)intf_ptr, reg_data, len);
	
	APP_ERROR_CHECK(err_code);
	
	return err_code;
}

static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	uint8_t write_data[256];

	if (len > sizeof(write_data) - 1)
		return 1;

	write_data[0] = reg_addr;

	memcpy(&write_data[1], reg_data, len);

	ret_code_t err_code = nrf_drv_twi_tx(p_sensor_twi_instance, *(uint8_t *)intf_ptr, write_data, len + 1, false);

	APP_ERROR_CHECK(err_code);

	return 0;
}

static void bme280_measurement_delay_timeout_handler(void *p_context)
{
	UNUSED_PARAMETER(p_context);

	struct bme280_data comp_data;

	int8_t rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &bme280_sensor);
	if (rslt != BME280_OK) {
		results_handler(NULL);
		return;
	}

	bme280_results_t results;
	results.pressure = comp_data.pressure;
	results.temperature = comp_data.temperature;
	results.humidity = comp_data.humidity;

	results_handler(&results);
}

static bool init_bme280()
{
	bme280_sensor.intf_ptr = &bme280_sensor_addr;
	bme280_sensor.intf = BME280_I2C_INTF;
	bme280_sensor.read = user_i2c_read;
	bme280_sensor.write = user_i2c_write;
	bme280_sensor.delay_us = user_delay_us;

	int8_t rslt = bme280_init(&bme280_sensor);
	if (rslt != BME280_OK)
		return false;
	
	bme280_sensor.settings.osr_h = BME280_OVERSAMPLING_16X;
	bme280_sensor.settings.osr_p = BME280_OVERSAMPLING_16X;
	bme280_sensor.settings.osr_t = BME280_OVERSAMPLING_16X;
	bme280_sensor.settings.filter = BME280_FILTER_COEFF_4;

	uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

	rslt = bme280_set_sensor_settings(settings_sel, &bme280_sensor);
	if (rslt != BME280_OK)
		return false;

	bme280_measurement_delay = bme280_cal_meas_delay(&bme280_sensor.settings);

	return rslt == BME280_OK;
}

int bme280_start_measurement()
{
	if (!bme280_sensor_initialized) {
		results_handler(NULL);
		return 1;
	}

	int8_t rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &bme280_sensor);
	if (rslt != BME280_OK) {
		results_handler(NULL);
		return 1;
	}

	ret_code_t err_code = app_timer_start(m_bme280_measurement_delay_timer_id, APP_TIMER_TICKS(bme280_measurement_delay + 10), NULL);
	APP_ERROR_CHECK(err_code);

	return 0;
}

int bme280_sensor_init(bme280_results_handler_t handler, nrf_drv_twi_t const *instance, uint8_t addr)
{
	bme280_sensor_initialized = false;

	results_handler = handler;
	p_sensor_twi_instance = instance;
	bme280_sensor_addr = addr;

	uint32_t error_code = 0;

	// bme280 measurement delay timer
	error_code = app_timer_create(&m_bme280_measurement_delay_timer_id, APP_TIMER_MODE_SINGLE_SHOT, bme280_measurement_delay_timeout_handler);
	APP_ERROR_CHECK(error_code);

	if (!init_bme280()) {
		return 1;
	}
	
	bme280_sensor_initialized = true;

	return 0;
}
