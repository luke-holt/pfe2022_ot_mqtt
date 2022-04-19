
#include <drivers/i2c.h>
#include <sys/printk.h>
#include <zephyr.h>

#include "sgp30.h"


/* I2C buffers. */
static unsigned char i2c_tx_buffer[I2C_BUFFER_SIZE];
static unsigned char i2c_rx_buffer[I2C_BUFFER_SIZE];


/* I2C device structure. */
static const struct device * dev_sgp30;


static void sgp30_i2c_config()
{
    dev_sgp30 = device_get_binding(DT_LABEL(I2C_NODE));

    if (dev_sgp30 == NULL)
    {
        I2C_LOG_DEV_ERROR(__FILE__, __func__, __LINE__);
    }
    else
    {
        int ret_code;

        ret_code = i2c_configure(dev_sgp30, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER);

        I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);
    }
}


static sgp30_data_t extract_data_from_buffer()
{
    /* See SGP30 datasheet for details on this operation.
     *   For the sake of practicality, we are currently ignoring CRC bytes.
     */

    /* CO2 measurement in ppm */
    uint16_t co2_data = (i2c_rx_buffer[0] << 8) | i2c_rx_buffer[1];

    /* TVOC is ppb */
    uint16_t tvoc_data = (i2c_rx_buffer[3] << 8) | i2c_rx_buffer[4];

    sgp30_data_t data = SGP30_NEW_DATA(co2_data, tvoc_data);

    return data;
}



void sgp30_init()
{
    sgp30_i2c_config();

    int ret_code;

    /* init_air_quality (0x2003) */
    i2c_tx_buffer[0] = 0x20;
    i2c_tx_buffer[1] = 0x03;

    ret_code = i2c_write(dev_sgp30, i2c_tx_buffer, 2, SGP30_ADDR);

    I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);

    /* measure_air_quality (0x2008) */
    i2c_tx_buffer[0] = 0x20;
    i2c_tx_buffer[1] = 0x08;

    /* Ensure sensor operation is proper. SGP30 datasheet */
    for (int i = 0; i < 15; i++)
    {
        k_msleep(1000);

        ret_code = i2c_write(
            dev_sgp30,
            i2c_tx_buffer,
            2,
            SGP30_ADDR
        );

        I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);

        k_msleep(12);

        ret_code = i2c_read(
            dev_sgp30,
            i2c_rx_buffer,
            6,
            SGP30_ADDR
        );

        I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);
    }
}



sgp30_data_t sgp30_get_data()
{
    if (dev_sgp30 == NULL)
    {
        I2C_LOG_DEV_ERROR(__FILE__, __func__, __LINE__);
    }

    int ret_code;

    /* measure_air_quality (0x2008) */
    i2c_tx_buffer[0] = 0x20;
    i2c_tx_buffer[1] = 0x08;

    ret_code = i2c_write(
        dev_sgp30,
        i2c_tx_buffer,
        2,
        SGP30_ADDR
    );

    I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);

    k_msleep(12); // SGP30 datasheet (measurement duration)

    ret_code = i2c_read(
        dev_sgp30, 
        i2c_rx_buffer,
        6,
        SGP30_ADDR
    );

    I2C_ERROR_CHECK(ret_code, __FILE__, __func__, __LINE__);

    return extract_data_from_buffer();
}