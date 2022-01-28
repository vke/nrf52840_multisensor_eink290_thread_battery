#include "nrf.h"
#include "bsp.h"
#include "nrf_log.h"
#include "settings.h"
#include "nrf_drv_twi.h"
#include "max44009.h"

#include <openthread/platform/alarm-milli.h>

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

static bool config_max44009()
{
	uint8_t regData = 0x80; // continious mode bit for configuration register
	uint8_t i2c_result = user_i2c_write(0x02, &regData, 1, &max44009_sensor_addr);
	return i2c_result == 0;
}

int max44009_start_measurement()
{
	if (!max44009_sensor_initialized) {
		results_handler(NULL);
		return 1;
	}

	uint8_t data[2];

	// FIXME: should be called in single transation without STOP
	uint8_t i2c_result1 = user_i2c_read(0x03, data, 1, &max44009_sensor_addr);
	if (i2c_result1) {
		results_handler(NULL);
		return 1;
	}
	uint8_t i2c_result2 = user_i2c_read(0x04, data + 1, 1, &max44009_sensor_addr);
	if (i2c_result2) {
		results_handler(NULL);
		return 1;
	}
	uint8_t exponent = (data[0] >> 4) & 0x0F;
	uint32_t mantissa = ((data[0] & 0x0F) << 4) + (data[1] & 0x0F);
	max44009_results_t results;
	results.lux = 0x3FC000; // max sensor value - 188K
	if (exponent != 15)
		results.lux = mantissa << exponent;
	results_handler(&results);

	return 0;
}

int max44009_sensor_init(max44009_results_handler_t handler, nrf_drv_twi_t const *instance, uint8_t addr)
{
	max44009_sensor_initialized = false;

	results_handler = handler;
	p_sensor_twi_instance = instance;
	max44009_sensor_addr = addr;

	if (!config_max44009()) {
		return 1;
	}

	max44009_sensor_initialized = true;
	
	return 0;
}
