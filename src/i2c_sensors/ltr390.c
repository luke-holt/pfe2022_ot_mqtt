
#include <drivers/i2c.h>
#include <sys/printk.h>
#include <zephyr.h>

#include "ltr390.h"


/* I2C buffers. 14 bytes. */
static unsigned char i2c_tx_buffer[I2C_BUFFER_SIZE];
static unsigned char i2c_rx_buffer[I2C_BUFFER_SIZE] = "Begin loopback";

/* I2C device structure */
static const struct device * dev_ltr390;


/**
 * @brief Helper function that configures the I2C hardware node using the Zephyr I2C API.
 *  This function enables the I2C driver to use the Device Tree information for AM2301B.
 * 
 */
static void ltr390_i2c_config()
{
    dev_ltr390 = device_get_binding(DT_LABEL(I2C_NODE));

    if (dev_ltr390 == NULL)
    {
        I2C_LOG_DEV_ERROR(__FILE__, __func__, __LINE__);
    }
    else
    {
        int ret_code;

        ret_code = i2c_configure(dev_ltr390, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER);

        I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);
    }
}


static ltr390_data_t extract_data_from_buffer()
{
    /* See LTR390 datasheet for details on this operation. */

    /*
    int gain_x_int = 3 * 1; // default value from datasheet
    int w_factor = 1; // default settings
    int uvs_sens = 2300;
    */

    /* Ambient light signal. */ 
    uint8_t als0 = i2c_rx_buffer[0];
    uint8_t als1 = i2c_rx_buffer[1];
    uint8_t als2 = i2c_rx_buffer[2] & 0xF;

    uint32_t als_data = als0 | (als1 << 8) | (als2 << 16);

    // double als = 0.6 * w_factor * als_data / gain_x_int;

    /* Ultra-violet signal. */
    uint8_t uvs0 = i2c_rx_buffer[3];
    uint8_t uvs1 = i2c_rx_buffer[4];
    uint8_t uvs2 = i2c_rx_buffer[5] & 0xF;

    uint32_t uvs_data = uvs0 | (uvs1 << 8) | (uvs2 << 16);

    // double uvs = uvs_data / uvs_sens * w_factor;

    ltr390_data_t data = LTR390_NEW_DATA(als_data, uvs_data);
    return data;
}



static uint8_t ltr390_get_status()
{
    int ret_code;

    if (dev_ltr390 == NULL)
    {
        I2C_LOG_DEV_ERROR(__FILE__, __func__, __LINE__);
    }

    uint8_t status;

    uint8_t reg = LTR390_ADDR_MAIN_STATUS;

    ret_code = i2c_write_read(
        dev_ltr390, 
        LTR390_ADDR, 
        &reg, 
        1,
        &status, 
        1);

    I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);

    return status;
}


void ltr390_init()
{
    ltr390_i2c_config();
}


ltr390_data_t ltr390_get_data()
{
    if (dev_ltr390 == NULL)
    {
        I2C_LOG_DEV_ERROR(__FILE__, __func__, __LINE__);
    }

    int ret_code;

    /* Activate sensor in default mode (ALS) */
    i2c_tx_buffer[0] = LTR390_ADDR_MAIN_CTRL;
    i2c_tx_buffer[1] = 0x2; // ENABLE: 1, UVS_MODE: 0

    ret_code = i2c_write(dev_ltr390, i2c_tx_buffer, 2, LTR390_ADDR);

    I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);

    /* Wait for sensor to convert data using ADC */
    k_msleep(200); // See LTR390 datasheet (Conversion time)


    /* Read registry values for ambient light sensor data */
    i2c_tx_buffer[0] = LTR390_ADDR_ALS_DATA_0;
    i2c_tx_buffer[1] = LTR390_ADDR_ALS_DATA_1;
    i2c_tx_buffer[2] = LTR390_ADDR_ALS_DATA_2;

    /* Read ambient light sensor values */
    for (int i = 0; i < 3; i++)
    {
        ret_code = i2c_write_read(
            dev_ltr390, 
            LTR390_ADDR, 
            &i2c_tx_buffer[i], 
            1, 
            &i2c_rx_buffer[i], 
            1);

        I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);
    }


    /* Change sensor state to UVS */
    i2c_tx_buffer[0] = LTR390_ADDR_MAIN_CTRL;
    i2c_tx_buffer[1] = 0b1010; // ENABLE: 1, UVS_MODE: 1

    ret_code = i2c_write(dev_ltr390, i2c_tx_buffer, 2, LTR390_ADDR);

    I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);


    /* Wait for sensor to convert data using ADC */
    k_msleep(200); // See LTR390 datasheet (Conversion time)


    /* Read registry values for ambient light sensor data */
    i2c_tx_buffer[0] = LTR390_ADDR_UVS_DATA_0;
    i2c_tx_buffer[1] = LTR390_ADDR_UVS_DATA_1;
    i2c_tx_buffer[2] = LTR390_ADDR_UVS_DATA_2;

    /* Read ultra-violet sensor values */
    for (int i = 0; i < 3; i++)
    {
        ret_code = i2c_write_read(
            dev_ltr390, 
            LTR390_ADDR, 
            &i2c_tx_buffer[i], 
            1, 
            &i2c_rx_buffer[i + 3], // Next to the existing ALS bytes
            1);

        I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);
    }

    return extract_data_from_buffer();
}