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

#define MAX44009_REG_ADDR_CONFIG          0x02
#define MAX44009_REG_ADDR_LUX_HIGH        0x03
#define MAX44009_REG_ADDR_LUX_LOW         0x04

#define MAX44009_CONFIG_CONTINIOUS        0x80

#define MAX44009_MAX_SENSOR_VALUE         0x3FC000

#define MAX44009_EXPONENT_OVERFLOW        15


static ret_code_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, uint8_t address)
{
	ret_code_t err_code = nrf_drv_twi_tx(p_sensor_twi_instance, address, &reg_addr, 1, false);

	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_twi_rx(p_sensor_twi_instance, address, reg_data, len);

	APP_ERROR_CHECK(err_code);

	return err_code;
}

ret_code_t max44009_configure()
{
	uint8_t config_data[2];
	config_data[0] = MAX44009_REG_ADDR_CONFIG; // max44009 sensor configuration register address
	config_data[1] = MAX44009_CONFIG_CONTINIOUS; // continious mode bit for configuration register

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

	// FIXME: should be called in one transation without STOP
	ret_code_t i2c_result = NRF_ERROR_INTERNAL;

	i2c_result = user_i2c_read(MAX44009_REG_ADDR_LUX_HIGH, data, 1, max44009_sensor_addr);
	if (i2c_result != NRF_SUCCESS) {
		results_handler(NULL);
		return NRF_ERROR_INTERNAL;
	}

	i2c_result = user_i2c_read(MAX44009_REG_ADDR_LUX_LOW, data + 1, 1, max44009_sensor_addr);
	if (i2c_result != NRF_SUCCESS) {
		results_handler(NULL);
		return NRF_ERROR_INTERNAL;
	}

	uint8_t exponent = (data[0] >> 4) & 0x0F;
	uint32_t mantissa = ((data[0] & 0x0F) << 4) + (data[1] & 0x0F);

	max44009_results_t results;
	results.lux = MAX44009_MAX_SENSOR_VALUE;
	if (exponent != MAX44009_EXPONENT_OVERFLOW)
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
