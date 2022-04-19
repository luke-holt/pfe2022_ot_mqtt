
/* I2C hardware access information from Device Tree*/
#ifndef I2C_NODE
#define I2C_NODE            DT_NODELABEL(i2c0)
#endif

#ifndef I2C_SCL
#define I2C_SCL             DT_PROP(I2C_NODE, scl_pin)
#endif

#ifndef I2C_SDA
#define I2C_SDA             DT_PROP(I2C_NODE, sda_pin)
#endif

#ifndef I2C_CLK_FREQ
#define I2C_CLK_FREQ        DT_PROP(I2C_NODE, clock_frequency)
#endif

#ifndef I2C_BUFFER_SIZE
#define I2C_BUFFER_SIZE     14
#endif

/* Macro to compare a status byte with a flags byte */
#define COMPARE_FLAGS(status, flags) ((status & flags) == flags) ? 1 : 0

/* Macro to handle generic success verification for I2C transfer functions */
#define I2C_ERROR_CHECK(err, file, func, line) (err != 0)                                                                   \
    ? printk("ERR: I2C general input/output error in %s. File: %s -> line : %u\r\n", func, file, line)                      \
    : 1

/* Macro to log I2C device error in serial */
#define I2C_LOG_DEV_ERROR(file, func, line) do                                                                              \
{                                                                                                                           \
    printk("ERR: I2C device not initiated or not in Device Tree. (%s)\nFile: %s : Line: %u\r\n", func, file, line);         \
} while(0);
