#include "nrf.h"
#include "bsp.h"
#include "nrf_log.h"
#include "settings.h"
#include "nrf_drv_twi.h"
#include "app_timer.h"
#include "sgp40.h"

#include <openthread/platform/alarm-milli.h>

static sgp40_results_handler_t results_handler = NULL;
static nrf_drv_twi_t const *p_sensor_twi_instance = NULL;
static uint8_t sgp40_sensor_addr = 0;
static bool sgp40_sensor_initialized = false;

APP_TIMER_DEF(m_sgp40_measurement_delay_timer_id);

#define SGP40_CRC8_POLYNOMIAL 0x31
#define SGP40_CRC8_INIT 0xFF


static uint8_t generateCRC(uint8_t *data, uint8_t datalen)
{
	uint8_t crc = SGP40_CRC8_INIT;

	for (uint8_t i = 0; i < datalen; i++)
	{
		crc ^= data[i];
		for (uint8_t b = 0; b < 8; b++)
		{
			if (crc & 0x80)
				crc = (crc << 1) ^ SGP40_CRC8_POLYNOMIAL;
			else
				crc <<= 1;
		}
	}
	return crc;
}

bool sgp40_turn_heater_off()
{
	uint8_t cmd[] = {0x36, 0x15};
	
	ret_code_t err_code = nrf_drv_twi_tx(p_sensor_twi_instance, *(uint8_t *)&sgp40_sensor_addr, cmd, 2, false);

	return true;
}

bool sgp40_measure_raw_signal()
{
	if (!sgp40_sensor_initialized) {
		results_handler(NULL);
		return false;
	}

	uint8_t cmd[] = {0x26, 0x0F, 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93};
	
	ret_code_t err_code = nrf_drv_twi_tx(p_sensor_twi_instance, *(uint8_t *)&sgp40_sensor_addr, cmd, 8, false);
	
	err_code = app_timer_start(m_sgp40_measurement_delay_timer_id, APP_TIMER_TICKS(50), NULL);
	APP_ERROR_CHECK(err_code);

	return true;
}

static void sgp40_measurement_delay_timeout_handler(void *p_context)
{
	UNUSED_PARAMETER(p_context);

	uint8_t result[3] = {0};
	ret_code_t err_code = nrf_drv_twi_rx(p_sensor_twi_instance, *(uint8_t *)&sgp40_sensor_addr, result, 3);

	uint16_t signal = result[0] * 256 + result[1];
	
	uint8_t crc = generateCRC(result, 2);
	if (crc != result[2]) {
		NRF_LOG_INFO("Time: %d, sgp40 invalid crc: 0x%02x, 0x%02x, 0x%02x", result[0], result[1], result[2]);
		results_handler(NULL);
	}

//	NRF_LOG_INFO("Time: %d, sgp40 raw data: 0x%08x, %d", otPlatAlarmMilliGetNow(), signal, signal);
	if (signal > 15000 && signal < 55000) {
		sgp40_results_t results;
		results.sensor_raw_value = signal;
		results_handler(&results);
	}
}

int sgp40_sensor_init(sgp40_results_handler_t handler, nrf_drv_twi_t const *instance, uint8_t addr)
{
	sgp40_sensor_initialized = false;

	results_handler = handler;
	p_sensor_twi_instance = instance;
	sgp40_sensor_addr = addr;

	uint32_t error_code = 0;

	// sgp40 measurement delay timer
	error_code = app_timer_create(&m_sgp40_measurement_delay_timer_id, APP_TIMER_MODE_SINGLE_SHOT, sgp40_measurement_delay_timeout_handler);
	APP_ERROR_CHECK(error_code);

	sgp40_sensor_initialized = true;

	return 0;
}
