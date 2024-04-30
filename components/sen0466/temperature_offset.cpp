#include "temperature_offset.h"

namespace esphome {
  namespace sen0466_sensor {
    void temperature_offset::control(float value) { this->parent_->set_temperature_offset(value); }
  }
}