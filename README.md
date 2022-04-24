# PFE2022 - IoT greenhouse

This repo contains the embedded software written for my senior design project. 

This application is built using the Nordic nRF Connect SDK. This SDK uses Project Zephyr as the real time operating system (RTOS) and the application uses IPv6-based OpenThread stack for the communication layer.

## Files

I2C sensor drivers
- include/am2301b.h
- src/i2c_sensors/am2301b.c
- include/ltr390.h
- src/i2c_sensors/ltr390.c
- include/sgp30.h
- src/i2c_sensors/sgp30.c
- include/i2c_config.h

LED functions
- include/led.h
- src/led.c

"MQTT" UDP functions
- include/mqtt_udp.h
- src/mqtt_udp.c

Main function
- src/main.c

Modified Device Tree information
- boards/nrf52840dk_nrf52840.overlay

Configuration files
- Kconfig
- prj.conf


## MQTT Manager
During the design phase of the project, a data collection system was built to accomodate a MQTT sensor network. This was done, because, initially, we were using the *nRF5 SDK for Thread and Zigbee* because it had native MQTT support for OpenThread. We ran into issues using Nordic's Two-Wire interface (I2C) in this SDK, and due to time constraints, decided to use the more recent nRF Connect SDK. This, in turn created a new problem, where MQTT is not yet supported in the NCS. 

As a solution, we ended up using OpenThread's UDP API to send the sensor data to a Python service (*mqtt_manager.py*) that ran on the OpenThread Border Router, which then published it to our existing MQTT system.

This service also subscribes to the LED topics and sends messages to the Thread devices for them to update the LED states accordingly.

This *works*, but is not scalable and not ideal. This script is a middle-man that should be cut out in future iterations of this project in favour of native MQTT support.