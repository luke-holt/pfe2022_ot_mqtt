/**
 * @file mqtt_udp.h
 * @author Luke Holt (elh7695@umoncton.ca)
 * @brief Declaration of functions used to abstract UDP and MQTT communications.
 * @version 0.1
 * @date 2022-04-14
 * 
 * 
 */

#include <openthread/instance.h>


extern void handleUdpReceive(
    void *aContext,
    otMessage *aMessage,
    const otMessageInfo *aMessageInfo);


void initUdp(otInstance *aInstance, void *rx_callback);

void sendUdp(otInstance *aInstance);

void mqtt_set_buffer(otInstance *aInstance, uint8_t role, uint8_t topic, uint32_t raw_data);