/**
 * @file led.c
 * @author your name (you@domain.com)
 * @brief Implementation of "set_led()". 
 * @version 0.1
 * @date 2022-04-19
 * 
 * 
 */


#include <drivers/gpio.h>
#include "led.h"


/* Device struct for each LED */
static const struct device *dev_led0;
static const struct device *dev_led1;
static const struct device *dev_led2;
static const struct device *dev_led3;


static void dev_init()
{
    dev_led0 = device_get_binding(LED0);
    dev_led1 = device_get_binding(LED1);
    dev_led2 = device_get_binding(LED2);
    dev_led3 = device_get_binding(LED3);

    if (dev_led0 == NULL)
    {
        printk("LED0 device not found in Device Tree.\r\n");
    }
    else if (dev_led1 == NULL)
    {
        printk("LED1 device not found in Device Tree.\r\n");
    }
    else if (dev_led2 == NULL)
    {
        printk("LED2 device not found in Device Tree.\r\n");
    }
    else if (dev_led3 == NULL)
    {
        printk("LED3 device not found in Device Tree.\r\n");
    }

    int ret_code;
    ret_code = gpio_pin_configure(dev_led0, PIN0, GPIO_OUTPUT_ACTIVE);
    if (ret_code < 0) printk("LED0 configuration failed. RC: %d\r\n", ret_code);

    ret_code = gpio_pin_configure(dev_led1, PIN1, GPIO_OUTPUT_ACTIVE);
    if (ret_code < 0) printk("LED1 configuration failed. RC: %d\r\n", ret_code);

    ret_code = gpio_pin_configure(dev_led2, PIN2, GPIO_OUTPUT_ACTIVE);
    if (ret_code < 0) printk("LED2 configuration failed. RC: %d\r\n", ret_code);

    ret_code = gpio_pin_configure(dev_led3, PIN3, GPIO_OUTPUT_ACTIVE);
    if (ret_code < 0) printk("LED3 configuration failed. RC: %d\r\n", ret_code);
}


void set_led(uint32_t led, uint8_t state)
{
    if (dev_led0 == NULL ||
        dev_led1 == NULL ||
        dev_led2 == NULL ||
        dev_led3 == NULL) dev_init();

    if (led == 0x6c656430)
    {
        if (state == 0x69) // i
        {
            gpio_pin_set(dev_led0, PIN0, GPIO_ACTIVE_HIGH);
        }
        else if (state == 0x6f) // o
        {
            gpio_pin_set(dev_led0, PIN0, GPIO_ACTIVE_LOW);
        }
    }
    else if (led == 0x6c656431)
    {
        if (state == 0x69) // i
        {
            gpio_pin_set(dev_led1, PIN1, GPIO_ACTIVE_HIGH);
        }
        else if (state == 0x6f) // o
        {
            gpio_pin_set(dev_led1, PIN1, GPIO_ACTIVE_LOW);
        }
    }
    else if (led == 0x6c656432)
    {
        if (state == 0x69) // i
        {
            gpio_pin_set(dev_led2, PIN2, GPIO_ACTIVE_HIGH);
        }
        else if (state == 0x6f) // o
        {
            gpio_pin_set(dev_led2, PIN2, GPIO_ACTIVE_LOW);
        }
    }
    else if (led == 0x6c656433)
    {
        if (state == 0x69) // i
        {
            gpio_pin_set(dev_led3, PIN3, GPIO_ACTIVE_HIGH);
        }
        else if (state == 0x6f) // o
        {
            gpio_pin_set(dev_led3, PIN3, GPIO_ACTIVE_LOW);
        }
    }
}
