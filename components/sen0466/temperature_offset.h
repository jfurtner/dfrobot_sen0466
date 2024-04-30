#pragma once
#include "esphome/components/number/number.h"
#include "sen0466.h"

namespace esphome {
  namespace sen0466_sensor {
    class temperature_offset : public number::Number, public Parented<Sen0466Sensor> {
      public:
        temperature_offset() = default;

      protected:
        void control(float value) override;
    };
  }
}