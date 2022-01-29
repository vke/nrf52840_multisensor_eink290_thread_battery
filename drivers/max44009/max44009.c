#include "nrf.h"
#include "bsp.h"
#include "nrf_log.h"
#include "settings.h"
#include "nrf_drv_twi.h"
#include "max44009.h"

static max44009_results_handler_t results_handler = NULL;
static nrf_drv_twi_t const *p_sensor_twi_instance = NULL;
static uint8_t max44009_sensor_addr = 0;
static bool max44009_sensor_initialized = false;


static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	ret_code_t err_code = nrf_drv_twi_tx(p_sensor_twi_instance, *(uint8_t *)intf_ptr, &reg_addr, 1, false);

	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_twi_rx(p_sensor_twi_instance, *(uint8_t *)intf_ptr, reg_data, len);

	APP_ERROR_CHECK(err_code);

	return err_code;
}

ret_code_t max44009_configure()
{
	uint8_t config_data[2];
	config_data[0] = 0x02; // max44009 sensor configuration register address
	config_data[1] = 0x80; // continious mode bit for configuration register

	ret_code_t err_code = nrf_drv_twi_tx(p_sensor_twi_instance, max44009_sensor_addr, config_data, sizeof(config_data), false);
	APP_ERROR_CHECK(err_code);

	return err_code;
}

ret_code_t max44009_start_measurement()
{
	if (!max44009_sensor_initialized) {
		results_handler(NULL);
		return NRF_ERROR_INVALID_STATE;
	}

	uint8_t data[2];

	// FIXME: should be called in single transation without STOP
	uint8_t i2c_result1 = user_i2c_read(0x03, data, 1, &max44009_sensor_addr);
	if (i2c_result1) {
		results_handler(NULL);
		return NRF_ERROR_INTERNAL;
	}
	uint8_t i2c_result2 = user_i2c_read(0x04, data + 1, 1, &max44009_sensor_addr);
	if (i2c_result2) {
		results_handler(NULL);
		return NRF_ERROR_INTERNAL;
	}

	uint8_t exponent = (data[0] >> 4) & 0x0F;
	uint32_t mantissa = ((data[0] & 0x0F) << 4) + (data[1] & 0x0F);
	max44009_results_t results;
	results.lux = 0x3FC000; // max sensor value - 188K
	if (exponent != 15)
		results.lux = mantissa << exponent;
	results_handler(&results);

	return NRF_SUCCESS;
}

ret_code_t max44009_sensor_init(max44009_results_handler_t handler, nrf_drv_twi_t const *instance, uint8_t addr)
{
	max44009_sensor_initialized = false;

	results_handler = handler;
	p_sensor_twi_instance = instance;
	max44009_sensor_addr = addr;

	ret_code_t config_error = max44009_configure();

	if (config_error != NRF_SUCCESS) {
		return config_error;
	}

	max44009_sensor_initialized = true;

	return NRF_SUCCESS;
}
