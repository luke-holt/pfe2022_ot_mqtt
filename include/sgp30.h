
#include "i2c_config.h"

#define SGP30_ADDR          DT_REG_ADDR(DT_NODELABEL(sgp30))
#define SGP30_DEV_NAME      DT_PROP(DT_NODELABEL(sgp30), label)


/**
 * @brief Structure for return sensor data.
 *  .co2    Carbon Dioxide reading
 *  .tos    Total Volatile Organic Compounds
 */
typedef struct {
    uint16_t co2;
    uint16_t tvoc;
} sgp30_data_t;


#define SGP30_NEW_DATA(_co2, _tvoc)     \
{                                       \
    .co2    =   _co2,                   \
    .tvoc   =   _tvoc,                  \
}



void sgp30_init(void);


sgp30_data_t sgp30_get_data(void);