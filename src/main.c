/**
 * @file main.c
 * @author Luke Holt (elh7695@umoncton.ca)
 * @brief File containing the main function for the OpenThread Greenhouse project. 
 * @version 0.1
 * @date 2022-04-14
 * 
 * 
 */


#include <string.h>
#include <assert.h>

#include <zephyr.h>

#include <openthread-core-zephyr-config.h>
#include <openthread/config.h>
#include <openthread/dataset.h>
#include <openthread/thread.h>
#include <openthread/tasklet.h>

#include "openthread-system.h"

/* Header for UDP functionality */
#include "mqtt_udp.h"

/* Headers for I2C sensors */
#include "am2301b.h"
#include "ltr390.h"
#include "sgp30.h"

/* Header to toggle LED */
#include "led.h"

/* OpenThread instance used for context throughout the app */
static otInstance *instance;

/* UDP message buffer used in handleUdpReceive */
static char UDP_MSG_BUFFER[CONFIG_UDP_BUFFER_SIZE];

static void handleNetifStateChanged(uint32_t aFlags, void *aContext);
static void set_net_config(otInstance *aInstance);
static void send_sensor_data(otInstance *aInstance);


/* Periodic work handler function that is called periodically */
static void sensor_work_handler(struct k_work *work)
{
	/* Periodic processing */

	send_sensor_data(instance);
}
K_WORK_DEFINE(work, sensor_work_handler);

static void timer_handler(struct k_timer *timer)
{
	k_work_submit(&work);
}
K_TIMER_DEFINE(timer, timer_handler, NULL);


// static void handleUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);


int main(int argc, char * argv[])
{

pseudo_reset:

	otSysInit(argc, argv);
	instance = otInstanceInitSingle();
	assert(instance);

	/* Register Thread state change handler */
	otSetStateChangedCallback(instance, handleNetifStateChanged, instance);

	/* Override default network credentials.
		Bad practice. Only used for demonstration purposes. */
	set_net_config(instance);

	/* OT CLI: > ifconfig up */
	otIp6SetEnabled(instance, true);

	/* OT CLI: > thread start */
	otThreadSetEnabled(instance, true);

	/* Custom function to bring up UDP */
	initUdp(instance, handleUdpReceive);
	
	/* Calls to initiate the sensors */
	am2301b_init();
	ltr390_init();
	sgp30_init();

	printk("Setup complete.\r\n");

	/* Start the periodic timer for data read and tx */
	k_timer_start(&timer, K_SECONDS(5), K_SECONDS(5));

	/* Main loop processes */
	while(true)
	{
		otTaskletsProcess(instance);
		otSysProcessDrivers(instance);
	}

	otInstanceFinalize(instance);

	goto pseudo_reset;

	return 0;
}


/**
 * @brief Callback function that handles OpenThread network changes
 * 
 * @param aFlags 
 * @param aContext 
 */
void handleNetifStateChanged(uint32_t aFlags, void *aContext)
{
	if ((aFlags & OT_CHANGED_THREAD_ROLE) != 0)
	{
		otDeviceRole new_role = otThreadGetDeviceRole(aContext);
	
		switch (new_role)
		{
		case OT_DEVICE_ROLE_LEADER:
		 	printk("Role: LEADER.\r\n");
			break;
		case OT_DEVICE_ROLE_ROUTER:
		 	printk("Role: ROUTER.\r\n");
			break;
		case OT_DEVICE_ROLE_CHILD:
		 	printk("Role: CHILD.\r\n");
			break;
		case OT_DEVICE_ROLE_DETACHED:
		 	printk("Role: DETACHED.\r\n");
			break;
		case OT_DEVICE_ROLE_DISABLED:
		 	printk("Role: DISABLED.\r\n");
			break;
		}
	}
}


/**
 * @brief Set the net config for the active network
 * 		Bad practice. This function is only used for testing or demo purposes.
 * 
 * @param aInstance 
 */
void set_net_config(otInstance *aInstance)
{
	static char network_name[] = "PFE2022";

	otOperationalDataset dataset;

	memset(&dataset, 0, sizeof(otOperationalDataset));

	dataset.mActiveTimestamp = 1;
	dataset.mComponents.mIsActiveTimestampPresent = true;

	dataset.mChannel = 13;
	dataset.mComponents.mIsChannelPresent = true;
	
	/* Set PAN ID to BEEF */
	dataset.mPanId = (otPanId)0xbeef;
	dataset.mComponents.mIsPanIdPresent = true;

	/* Set Extended PAN ID to BEEFBEEFBEEFBEEF */
	uint8_t extPanId[OT_EXT_PAN_ID_SIZE] = {0xbe, 0xef, 0xbe, 0xef, 0xbe, 0xef, 0xbe, 0xef};
	memcpy(dataset.mExtendedPanId.m8, extPanId, sizeof(dataset.mExtendedPanId));
	dataset.mComponents.mIsExtendedPanIdPresent = true;

	/* Set Network Key to 0123456789ABCDEF0123456789ABCDEF */
	uint8_t key[OT_NETWORK_KEY_SIZE] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
	memcpy(dataset.mNetworkKey.m8, key, sizeof(dataset.mNetworkKey));
	dataset.mComponents.mIsNetworkKeyPresent = true;

	/* Set Network Name to Thread PFE2022 */
	size_t length = strlen(network_name);
	assert(length <= OT_NETWORK_NAME_MAX_SIZE);
	memcpy(dataset.mNetworkName.m8, network_name, length);
	dataset.mComponents.mIsNetworkNamePresent = true;

	switch (otDatasetSetActive(aInstance, &dataset))
	{
	case OT_ERROR_NONE:
		break;
	case OT_ERROR_NO_BUFS:
		printk("OT_ERROR_NO_BUFS\r\n");
		break;
	case OT_ERROR_NOT_IMPLEMENTED:
		printk("OT_ERROR_NOT_IMPLEMENTED\r\n");
		break;
	}
}


void send_sensor_data(otInstance *aInstance)
{
	printk("Send sensor data\r\n");
	/* Fetch data from sensors */
	am2301b_data_t am2301b_data = am2301b_get_data();
	ltr390_data_t ltr390_data = ltr390_get_data();
	sgp30_data_t sgp30_data = sgp30_get_data();

	/* Send sensor signals to MQTT manager */

	mqtt_set_buffer(
		instance, 
		CONFIG_MQTT_DEVICE_ROLE,
		CONFIG_MQTT_TOPIC_RHS,
		am2301b_data.rhs);
	sendUdp(instance);

	mqtt_set_buffer(
		instance,
		CONFIG_MQTT_DEVICE_ROLE,
		CONFIG_MQTT_TOPIC_TOS,
		am2301b_data.tos);
	sendUdp(instance);

	mqtt_set_buffer(
		instance,
		CONFIG_MQTT_DEVICE_ROLE,
		CONFIG_MQTT_TOPIC_ALS,
		ltr390_data.als);
	sendUdp(instance);

	mqtt_set_buffer(
		instance,
		CONFIG_MQTT_DEVICE_ROLE,
		CONFIG_MQTT_TOPIC_UVS,
		ltr390_data.uvs);
	sendUdp(instance);

	mqtt_set_buffer(
		instance,
		CONFIG_MQTT_DEVICE_ROLE,
		CONFIG_MQTT_TOPIC_CO2,
		sgp30_data.co2);
	sendUdp(instance);

	mqtt_set_buffer(
		instance,
		CONFIG_MQTT_DEVICE_ROLE,
		CONFIG_MQTT_TOPIC_TVOC,
		sgp30_data.tvoc);
	sendUdp(instance);
}


/**
 * @brief Callback function for when a UDP message is received.
 * 
 * @param aContext 
 * @param aMessage 
 * @param aMessageInfo 
 */
void handleUdpReceive(
    void *aContext,
    otMessage *aMessage,
    const otMessageInfo *aMessageInfo)
{
    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aMessageInfo);

	uint16_t n_bytes_rx = otMessageRead(
		aMessage,
		otMessageGetOffset(aMessage),
		UDP_MSG_BUFFER,
		sizeof(UDP_MSG_BUFFER) - 1);

	uint32_t led = 
		UDP_MSG_BUFFER[0] << 24 |
		UDP_MSG_BUFFER[1] << 16 |
		UDP_MSG_BUFFER[2] << 8 |
		UDP_MSG_BUFFER[3];
	
	uint8_t state = UDP_MSG_BUFFER[4];

	set_led(led, state);
}