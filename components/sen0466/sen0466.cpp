#include "sen0466.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
  namespace sen0466_sensor {

    static const char *const TAG = "sen0466_sensor.sensor";

    // void Sen0466Sensor::setup() {
    // 	ESP_LOGCONFIG(TAG, "Setting up sen0466...");
    // }

    void Sen0466Sensor::update() {
      ESP_LOGV(TAG, "update start");
      float temperature = read_temperature_C();

      ESP_LOGD(TAG, "update temperature: %f", temperature);

      float offset = temperature_offset_;
      ESP_LOGD(TAG, "update offset: %f", offset);

      float calc_temperature = temperature + offset;
      ESP_LOGD(TAG, "update calc temperature: %f", calc_temperature);

      temperature_sensor_->publish_state(calc_temperature);

      float gas = read_gas_ppm(calc_temperature);
      carbon_monoxide_sensor_->publish_state(gas);

      ESP_LOGV(TAG, "update end");
    }

    void Sen0466Sensor::dump_config() {
      ESP_LOGCONFIG(TAG, "DF Robot gas sen0466:");
      LOG_I2C_DEVICE(this);
      if (this->is_failed()) {
        ESP_LOGE(TAG, "Communication with sen0466 failed!");
      }
      LOG_UPDATE_INTERVAL(this);
    }

    float Sen0466Sensor::read_temperature_C(void)
    {
      ESP_LOGV(TAG, "read_temperature start");
      uint8_t protocol_data[6] = {0};
      uint8_t result[9] = {0xff};

      protocol_data[0] = CMD_GET_TEMP;

      sProtocol_t writeData = pack_output_buffer(protocol_data, sizeof(protocol_data));
      ESP_LOGD(TAG, "read_temperature setup output buffer");

      bool ret = this->write_bytes(CMD_I2C_REGISTER, (uint8_t*)&writeData, sizeof(writeData));
      ESP_LOGD(TAG, "read_temperature send command %d", ret);

      ret = this->read_bytes(CMD_I2C_REGISTER, result, (uint8_t) 9);
      ESP_LOGD(TAG, "read_temperature read_bytes: %d", ret);

      ESP_LOGV(TAG, "read_temperature result: %02x %02x %02x %02x %02x %02x %02x %02x",
        result[0], result[1], result[2], result[3], result[4], result[5],
        result[6], result[7], result[8]);

      if (result[8] != calculate_data_checksum(result, 8))
      {
        ESP_LOGE(TAG, "read_temperature checksum doesn't match!");
        return -100.0;
      }

      ESP_LOGD(TAG, "read_temperature calculate actual");
      uint16_t temp_ADC = (result[2] << 8) + result[3];

      ESP_LOGD(TAG, "read_temperature temp_ADC: %u", temp_ADC);

      float vPd3 = 3*(float)temp_ADC/1024;
      ESP_LOGD(TAG, "read_temperature vPd3: %f", vPd3);

      float rTh = vPd3*10000/(3-vPd3);
      ESP_LOGD(TAG, "read_temperature rTh: %f", rTh);

      float tBeta = 1/(1/(273.15+25)+1/3380.13*log(rTh/10000))-273.15;
      ESP_LOGD(TAG, "read_temperature tBeta: %f", tBeta);

      ESP_LOGV(TAG, "read_temperature end");
      return tBeta;
    }

    float Sen0466Sensor::read_gas_ppm(float temperature)
    {
      ESP_LOGV(TAG, "read_gas start");
      uint8_t protocol_data[6] = {0};
      uint8_t result[9] = {0xff};
      ESP_LOGD(TAG, "read_gas init");

      protocol_data[0] = CMD_GET_GAS_CONCENTRATION;
      sProtocol_t writeData = pack_output_buffer(protocol_data, sizeof(protocol_data));
      ESP_LOGD(TAG, "read_gas send buffer");


      bool ret = this->write_bytes(CMD_I2C_REGISTER, (uint8_t*)&writeData, sizeof(writeData));
      ESP_LOGD(TAG, "read_gas write_command %d", ret);

      ret = this->read_bytes(CMD_I2C_REGISTER, result, (uint8_t) 9);
      ESP_LOGD(TAG, "read_gas read_bytes: %d", ret);

      ESP_LOGD(TAG, "read_gas result: %02x %02x %02x %02x %02x %02x %02x %02x",
        result[0], result[1], result[2], result[3], result[4], result[5],
        result[6], result[7], result[8]);

      float concentration = 0.0;
      if (calculate_data_checksum(result, 8) != result[8])
      {
        ESP_LOGE(TAG, "read_gas checksum invalid");
        return -2.0;
      }
      concentration = ((result[2]<<8) + result[3])*1.0;
      ESP_LOGD(TAG, "read_gas initial concentration: %f", concentration);

      ESP_LOGD(TAG, "read_gas multiplier: %d", result[5]);
      switch (result[5])
      {
        case 1:
          concentration *= 0.1;
          break;
        case 2:
          concentration *= 0.01;
          break;
        default:
          break;
      }

      ESP_LOGD(TAG, "read_gas temperature correction");
      if (((temperature) > -20) && ((temperature) <= 20))
      {
        ESP_LOGD(TAG, "read_gas temp -20<t<=20");
        concentration = (concentration / (0.005 * (temperature) + 0.9));
      }
      else if (((temperature) > 20) && ((temperature) <= 40))
      {
        ESP_LOGD(TAG, "read_gas temp 20<t<=40");
        concentration = (concentration / (0.005 * (temperature) + 0.9) - (0.3 * (temperature)-6));
      }
      else
      {
        ESP_LOGE(TAG, "read_gas temperature out of operating range");
        return -1.0;
      }

      if (concentration < 0)
      {
        ESP_LOGW(TAG, "read_gas clamping concentration to 0 (was %f)", concentration);
        concentration = 0.0;
      }

      
      return concentration;
    }

    sProtocol_t Sen0466Sensor::pack_output_buffer(uint8_t *pBuf, uint8_t len)
    {
      sProtocol_t _protocol;
      _protocol.head = 0xff;
      _protocol.addr = 0x01;
      memcpy(_protocol.data, pBuf, len);
      _protocol.check = calculate_data_checksum((uint8_t *)&_protocol, 8);
      return _protocol;
    }

    uint8_t Sen0466Sensor::calculate_data_checksum(uint8_t* i, uint8_t ln)
    {
      uint8_t j,tempq=0;
      i+=1;
      for(j=0;j<(ln-2);j++)
      {
        tempq+=*i;i++;
      }
      tempq=(~tempq)+1;
      return(tempq);
    }

  }  // namespace sen0466_sensor
}  // namespace esphome
