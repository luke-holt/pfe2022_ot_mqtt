/**
 * @file mqtt_udp.c
 * @author Luke Holt (elh7695@umoncton.ca)
 * @brief Implementation of functions described in mqtt_udp.h for the OpenThread Greenhouse senior design project.
 * @version 0.1
 * @date 2022-04-14
 * 
 * 
 */



#include <zephyr.h>

#include <string.h>

#include <openthread/instance.h>

#include <openthread/message.h>
#include <openthread/udp.h>
#include <openthread/thread.h>

#include <utils/code_utils.h>

#include "mqtt_udp.h"


static char UDP_DEST_ADDR[] = "fd65:da0e:11d0:0:e65f:1ff:fe73:9779";
static char UDP_PAYLOAD[CONFIG_UDP_BUFFER_SIZE];
static uint8_t cursor_pos;

static otUdpSocket sUdpSocket;



void initUdp(otInstance *aInstance, void *rx_callback)
{
    otSockAddr listenSockAddr;

    memset(&sUdpSocket, 0, sizeof(sUdpSocket));
    memset(&listenSockAddr, 0, sizeof(listenSockAddr));

    listenSockAddr.mPort = CONFIG_UDP_PORT;
    otUdpOpen(aInstance, &sUdpSocket, handleUdpReceive, aInstance);
    otUdpBind(aInstance, &sUdpSocket, &listenSockAddr, OT_NETIF_THREAD);
}


void sendUdp(otInstance *aInstance)
{
    otError err = OT_ERROR_NONE;
    otMessage *message;
    otMessageInfo messageInfo;
    otIp6Address destinationAddr;

    memset(&messageInfo, 0, sizeof(messageInfo));

    otIp6AddressFromString(UDP_DEST_ADDR, &destinationAddr);
    messageInfo.mPeerAddr = destinationAddr;
    messageInfo.mPeerPort = CONFIG_UDP_PORT;

    message = otUdpNewMessage(aInstance, NULL);
    otEXPECT_ACTION(message != NULL, err = OT_ERROR_NO_BUFS);

    /* The "+ 1" is to include a NULL byte at the end */
    err = otMessageAppend(message, UDP_PAYLOAD, cursor_pos + 1);
    otEXPECT(err == OT_ERROR_NONE);

    err = otUdpSend(aInstance, &sUdpSocket, message, &messageInfo);

exit:
    if (err != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }
}


void mqtt_set_buffer(otInstance *aInstance, uint8_t role, uint8_t topic, uint32_t raw_data)
{
    memset(UDP_PAYLOAD, 0, sizeof(UDP_PAYLOAD));
    
    uint16_t rloc16 = otThreadGetRloc16(aInstance);

    /* Write RLOC16 (see openthread.io for details) and MQTT topic to buffer */
    UDP_PAYLOAD[0] = (rloc16 >> 8);
    UDP_PAYLOAD[1] = rloc16 & 0xff;
    UDP_PAYLOAD[2] = (role << 4) | topic;

    /* Write sensor data signal to buffer */
    UDP_PAYLOAD[3] = (raw_data >> 24);
    UDP_PAYLOAD[4] = (raw_data >> 16) & 0xff;
    UDP_PAYLOAD[5] = (raw_data >> 8) & 0xff;
    UDP_PAYLOAD[6] = raw_data & 0xff;

    cursor_pos = 7;
}
