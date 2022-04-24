import socket
from threading import Thread
# from numpy import Inf
import paho.mqtt.client as mqtt


# MQTT_HOST           = "192.168.1.208"
MQTT_HOST           = "192.168.2.87"
MQTT_PORT           = 1883

MQTT_USER           = "pfe2022"
MQTT_PASS           = "pfe2022"

MQTT_ROOT_TOPIC     = "pfe2022/"
MQTT_DEVICE_SENSOR  = "sensor/"
MQTT_DEVICE_CMD     = "cmd/"

MQTT_TOPIC_RHS      = 1
MQTT_TOPIC_TOS      = 2
MQTT_TOPIC_ALS      = 3
MQTT_TOPIC_UVS      = 4
MQTT_TOPIC_CO2      = 5
MQTT_TOPIC_TVOC     = 6

NET_ROLE_SENSOR     = 1
NET_ROLE_CMD        = 2


# List of OpenThread devices that have sent UDP packets
thread_devices = {}


def on_connect(client, userdata, flags, rc):
    """
    Function called by MQTT Client instance when it connects to broker
    """

    print(f"MQTT Manager connected. RC {str(rc)}")


def on_message(client, userdata, msg):
    """
    Function called when a new message is published to a topic that the
    client is subscribed to.
    """

    # root_topic/node_num/_cmd/*name* *msg.payload*
    topic = msg.topic.split("/")
    value = msg.payload.decode()

    udp_msg = ""

    if topic[-1] == "led0":
        if value == "on":
            udp_msg = "led0i"

        elif value == "off":
            udp_msg = "led0o"

    elif topic[-1] == "led1":
        if value == "on":
            udp_msg = "led1i"

        elif value == "off":
            udp_msg = "led1o"

    elif topic[-1] == "led2":
        if value == "on":
            udp_msg = "led2i"

        elif value == "off":
            udp_msg = "led2o"

    elif topic[-1] == "led3":
        if value == "on":
            udp_msg = "led3i"

        elif value == "off":
            udp_msg = "led3o"

    if udp_msg != "":
        for i, dev in enumerate(list(thread_devices.values())):
            if dev[2] == topic[1]:
                sock_info = dev[0:-1]
        second_socket_thread = Thread(target=thread_tx_udp, args=(udp_msg, sock_info))
        second_socket_thread.start()
        second_socket_thread.join()


def thread_tx_udp(udp_msg, addr_port) -> None:
    """
    Function to send commands to the OpenThread network.
    This function should only be called in a new thread to avoid messing with the listening socket in the main thread.
    """
    s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    s.sendto(udp_msg.encode(), addr_port)


def get_topic_str(topic: int) -> str:
    """
    Gets the topic int value and returns the corresponding topic string
    """

    if topic == MQTT_TOPIC_RHS:
        return "rhs"
    elif topic == MQTT_TOPIC_TOS:
        return "tos"
    elif topic == MQTT_TOPIC_ALS:
        return "als"
    elif topic == MQTT_TOPIC_UVS:
        return "uvs"
    elif topic == MQTT_TOPIC_CO2:
        return "co2"
    elif topic == MQTT_TOPIC_TVOC:
        return "tvoc"
    else:
        return "invalid"


def parse_data(topic: int, data: int):
    """
    Convert raw sensor data into real value (unit) floats
    For RHS and TOS calculations, see AM2301B datahseet
    For ALS and UVS calculations, see LTR390 datasheet
    For CO2 and TVOC calculations, see SGP30 datasheet
    """

    if topic == MQTT_TOPIC_RHS:
        return data / (1 << 20) * 100.0
    elif topic == MQTT_TOPIC_TOS:
        return data / (1 << 20) * 200.0 - 50
    elif topic == MQTT_TOPIC_ALS:
        return 0.6 * data * 1 / 3.0
    elif topic == MQTT_TOPIC_UVS:
        return data / 2300.0 * 1
    elif topic == MQTT_TOPIC_CO2:
        return data
    elif topic == MQTT_TOPIC_TVOC:
        return data
    else:
        return "Invalid"


def get_role_str(role: int) -> str :
    """
    Returns string value of corresponding role number
    """

    if role == NET_ROLE_SENSOR:
        return MQTT_DEVICE_SENSOR
    elif role == NET_ROLE_CMD:
        return MQTT_DEVICE_CMD
    else:
        return "invalid/"


def main():
    """
    Main process that runs on script startup
    """

    # Declare and bind target UDP socket
    s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    s.bind(('', 7131))

    # Initiate MQTT Client instance and register event callbacks
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    # Connect to broker. Default timeout is 60 seconds
    mqtt_client.connect(MQTT_HOST, MQTT_PORT)

    mqtt_client.loop_start()

    while 1:
        s.settimeout(50)
        # packet = s.recv(1024)
        packet, udp_addr = s.recvfrom(1024)

        # Extract the device ID (rloc16), role, topic and sensor reading information
        rloc16 = str(hex((packet[0] << 8) | packet[1]))
        role = packet[2] >> 4
        topic = packet[2] & 0xf
        sensor_data = (packet[3] << 24) | (packet[4] << 16) | (packet[5] << 8) | packet[6]

        # If device is not in list, add it and subscribe to control topic
        if rloc16 not in thread_devices.keys():
            name = f"node{len(thread_devices) + 1}"
            thread_devices[rloc16] = (udp_addr[0], udp_addr[1], name)
            print(thread_devices[rloc16])
            mqtt_client.subscribe(
                MQTT_ROOT_TOPIC
                + name + "/+")

        if role == NET_ROLE_SENSOR:
            mqtt_client.publish(
                MQTT_ROOT_TOPIC                     # Root topic to encapsulate all Thread devices
                + thread_devices[rloc16][2] + "/"   # Device identifier
                + get_topic_str(topic),             # Specific MQTT topic to classify different sensor readings
                parse_data(topic, sensor_data))     # Convert signal to real value


if __name__ == "__main__":
    main()
