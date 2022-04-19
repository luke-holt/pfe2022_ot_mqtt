
#include "i2c_config.h"

#define AM2301B_ADDR        DT_REG_ADDR(DT_NODELABEL(am2301b))
#define AM2301B_DEV_NAME    DT_PROP(DT_NODELABEL(am2301b), label)


/**
 * @brief Structure for return sensor data.
 *  .rhs    Relative Humidity Signal,
 *  .tos    Temperature Ouput Signal.
 */
typedef struct {
    uint32_t rhs;
    uint32_t tos;
} am2301b_data_t;


/* Macro for creating return data struct */
#define AM2301B_NEW_DATA(_rhs, _tos)    \
{                                       \
    .rhs = _rhs,                        \
    .tos = _tos,                        \
}

void am2301b_init(void);


am2301b_data_t am2301b_get_data(void);