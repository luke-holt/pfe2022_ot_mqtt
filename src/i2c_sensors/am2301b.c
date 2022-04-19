
#include <drivers/i2c.h>
#include <sys/printk.h>
#include <zephyr.h>

#include "am2301b.h"


/* I2C buffers. 14 bytes. */
static unsigned char i2c_tx_buffer[I2C_BUFFER_SIZE];
static unsigned char i2c_rx_buffer[I2C_BUFFER_SIZE];


/* I2C device structure */
static const struct device * dev_am2301b;


static am2301b_data_t extract_data_from_buffer()
{
    /* See AM2301B datasheet for details on this operation. */

    /* Relative Humidity Signal. */
    uint8_t rhs0 = i2c_rx_buffer[1];
    uint8_t rhs1 = i2c_rx_buffer[2];
    uint8_t rhs2 = i2c_rx_buffer[3] & 0xF0; // Remove 4 LSBs

    uint32_t rhs_data = (rhs0 << 12) | (rhs1 << 4) | (rhs2 >> 4); // 20 bits total

    // double rhs = rhs_data / (1 << 20) * 100.0;


    /* Temperature Output Signal */
    uint8_t tos0 = i2c_rx_buffer[3] & 0xF; // Remove 4 MSBs
    uint8_t tos1 = i2c_rx_buffer[4];
    uint8_t tos2 = i2c_rx_buffer[5];

    uint32_t tos_data = (tos0 << 16) + (tos1 << 8) + tos2; // 20 bits total
    
    // double tos = tos_data / (1 << 20) * 200 - 50;


    am2301b_data_t data = AM2301B_NEW_DATA(rhs_data, tos_data);

    return data;
}

static void am2301b_get_status()
{
    int ret_code;

    i2c_tx_buffer[0] = 0x71; // As per AM2301B datasheet

    /* Send request for status byte */
    ret_code = i2c_write(dev_am2301b, i2c_tx_buffer, 1, AM2301B_ADDR);
    
    I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);
    

    /* Read status byte */
    ret_code = i2c_read(dev_am2301b, i2c_rx_buffer, sizeof(i2c_rx_buffer), AM2301B_ADDR);

    I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);
}


/**
 * @brief Helper function that configures the I2C hardware node using the Zephyr I2C API.
 *  This function enables the I2C driver to use the Device Tree information for AM2301B.
 * 
 */
static void am2301b_i2c_config()
{
    int ret_code;

    dev_am2301b = device_get_binding(DT_LABEL(I2C_NODE));

    if (dev_am2301b == NULL)
    {
        I2C_LOG_DEV_ERROR(__FILE__, __func__, __LINE__);
    }
    else
    {
        // printk("\nI2C device %s found.\r\n", AM2301B_DEV_NAME);

        ret_code = i2c_configure(dev_am2301b, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER);

        I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);
    }
}

void am2301b_init()
{
    k_msleep(100); // As per AM2301B datasheet

    am2301b_i2c_config(); // Zephyr I2C configuration

    if (dev_am2301b == NULL)
    {
        I2C_LOG_DEV_ERROR(__FILE__, __func__, __LINE__);
    }
    else
    {
        am2301b_get_status();

        if (COMPARE_FLAGS(i2c_rx_buffer[0], 0x18)) // As per AM2301B datasheet
        {
            /* STATUS OK */
            k_msleep(10); // As per AM2301B datasheet
        }
        else
        {
            printk("ERR: Sensor registry problem. See AM2301B datasheet for more details.\r\n");
        }
    }
}


am2301b_data_t am2301b_get_data()
{
    if (dev_am2301b == NULL)
    {
        I2C_LOG_DEV_ERROR(__FILE__, __func__, __LINE__);
    }
    else
    {
        int ret_code;    

        /* AM2301B datasheet */
        i2c_tx_buffer[0] = 0xAC;
        i2c_tx_buffer[1] = 0x33;
        i2c_tx_buffer[2] = 0x00;

        ret_code = i2c_write(dev_am2301b, i2c_tx_buffer, 3, AM2301B_ADDR);

        I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);

        k_msleep(80);

        ret_code = i2c_read(dev_am2301b, i2c_rx_buffer, 7, AM2301B_ADDR);

        I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);

        return extract_data_from_buffer();
    }
}

