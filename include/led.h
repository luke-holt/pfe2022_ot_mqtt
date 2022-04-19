
#include <drivers/gpio.h>


/* Device tree info for builtin LED 0 to 4 */
/* LED0 */
#define LED0_NODE DT_ALIAS(led0)

#define LED0    DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN0    DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS0  DT_GPIO_FLAGS(LED0_NODE, gpios)

/* LED1 */
#define LED1_NODE DT_ALIAS(led1)

#define LED1    DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN1    DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGS1  DT_GPIO_FLAGS(LED1_NODE, gpios)

/* LED2 */
#define LED2_NODE DT_ALIAS(led2)

#define LED2    DT_GPIO_LABEL(LED2_NODE, gpios)
#define PIN2    DT_GPIO_PIN(LED2_NODE, gpios)
#define FLAGS2  DT_GPIO_FLAGS(LED2_NODE, gpios)

/* LED3 */
#define LED3_NODE DT_ALIAS(led3)

#define LED3    DT_GPIO_LABEL(LED3_NODE, gpios)
#define PIN3    DT_GPIO_PIN(LED3_NODE, gpios)
#define FLAGS3  DT_GPIO_FLAGS(LED3_NODE, gpios)



void set_led(uint32_t led, uint8_t state);