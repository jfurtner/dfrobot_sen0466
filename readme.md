DFRobot multi gas sensor, currently only for CO sensor (SEN0466), others should be easy to add. 
Uses I2C communications mode, no interrupt / threshold supported.

Always does temperature compensation (using algorithm in Arduino library).

Large parts blatently copied from DFRobot MultiGas Arduino library
https://github.com/DFRobot/DFRobot_MultiGasSensor

https://www.dfrobot.com/product-2508.html
https://wiki.dfrobot.com/SKU_SEN0465toSEN0476_Gravity_Gas_Sensor_Calibrated_I2C_UART

Configuration stanza:
```
sensor:
  - platform: sen0466
    temperature:
	  name: Temperature
	  # .. other settings from sensor apply
	carbon_monoxide:
	  name: Carbon Monoxide
	  # .. other settings from sensor apply
    address: 0x74
	  # Default address, see table below
```

Address dip switches (from wiki https://wiki.dfrobot.com/SKU_SEN0465toSEN0476_Gravity_Gas_Sensor_Calibrated_I2C_UART)
| Address | A0 | A1 |
|-|-|-|
| 0x74 | 0 | 0 |
| 0x75 | 0 | 1 |
| 0x76 | 1 | 0 |
| 0x77 | 1 | 1 |

Ensure SEL dip switch set to 0