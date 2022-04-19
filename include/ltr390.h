
#include "i2c_config.h"

#define LTR390_ADDR         DT_REG_ADDR(DT_NODELABEL(ltr390))
#define LTR390_DEV_NAME     DT_PROP(DT_NODELABEL(ltr390), label)


/* Internal addresses to sensor. See LTR390 datasheet for details. */
#define LTR390_ADDR_MAIN_CTRL       0x00
#define LTR390_ADDR_PART_ID         0x06
#define LTR390_ADDR_MAIN_STATUS     0x07

#define LTR390_ADDR_UVS_GAIN        0x05

#define LTR390_ADDR_ALS_DATA_0      0x0D
#define LTR390_ADDR_ALS_DATA_1      0x0E
#define LTR390_ADDR_ALS_DATA_2      0x0F

#define LTR390_ADDR_UVS_DATA_0      0x10
#define LTR390_ADDR_UVS_DATA_1      0x11
#define LTR390_ADDR_UVS_DATA_2      0x12

// #define LTR390_CHECK_MODE(status)   ((status & 0b1000) == 0b1000) ? "uvs" : "als"


/**
 * @brief Structure for return sensor data.
 *  .als    Ambient Light Signal
 *  .uvs    Ultra-Violet Signal
 */
typedef struct {
    uint32_t als;
    uint32_t uvs;
} ltr390_data_t;

/* Macro for creating return data struct */
#define LTR390_NEW_DATA(_als, _uvs)    \
{                                      \
    .als = _als,                       \
    .uvs = _uvs,                       \
}

void ltr390_init(void);


ltr390_data_t ltr390_get_data(void);