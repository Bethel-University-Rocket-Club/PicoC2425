#ifndef WS2812_UTILS_H
#define WS2812_UTILS_H

#include <stdint.h>
#include <stdbool.h>

// --- Configuration ---
#define NUM_LEDS_CFG 2          // We are controlling 2 LEDs
#define WS2812_PIN_CFG 6        // GPIO pin for NeoPixel data line (e.g., GP4)
#define IS_RGBW_CFG false       // Set to true if using RGBW NeoPixels, false for RGB

// --- Color Definitions (GRB format: Green byte, Red byte, Blue byte) ---
#define COLOR_BLACK   0x000000  // (G=0,   R=0,   B=0)
#define COLOR_RED     0x008000  // (G=0,   R=128, B=0)
#define COLOR_GREEN   0x800000  // (G=128, R=0,   B=0)
#define COLOR_BLUE    0x000080  // (G=0,   R=0,   B=128)
#define COLOR_YELLOW  0x808000  // (G=128, R=128, B=0)
#define COLOR_CYAN    0x800080  // (G=128, R=0,   B=128)
#define COLOR_MAGENTA 0x008080  // (G=0,   R=128, B=128)
#define COLOR_WHITE   0x808080  // (G=128, R=128, B=128) // Dim white

// --- LED Status/Error Code Definitions ---
typedef enum {
    LED_CODE_OFF = 0,     // Both LEDs off
    LED_CODE_ERR_1,       // LED1=RED,    LED2=BLUE
    LED_CODE_ERR_2,       // LED1=RED,    LED2=YELLOW
    LED_CODE_ERR_3,       // LED1=RED,    LED2=CYAN
    LED_CODE_ERR_4,       // LED1=BLUE,   LED2=YELLOW
    LED_CODE_ERR_5,       // LED1=BLUE,   LED2=MAGENTA
    LED_CODE_ERR_6,       // LED1=YELLOW, LED2=CYAN
    LED_CODE_ERR_7,       // LED1=YELLOW, LED2=MAGENTA
    LED_CODE_ERR_8,       // LED1=CYAN,   LED2=RED
    LED_CODE_ERR_9,       // LED1=MAGENTA,LED2=RED
    LED_CODE_ERR_10,      // LED1=WHITE,  LED2=BLUE
    LED_STATUS_GOOD       // Both LEDs GREEN
} led_status_code_t;

// --- Function Prototypes ---

// If this header is included by a C++ compiler, use C linkage for these functions
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the PIO state machine and GPIO for WS2812 LED control.
 * Must be called once before any other LED functions.
 */
void ws2812_leds_init(void);

/**
 * @brief Sets the color of the two LEDs.
 *
 * @param color_led1_grb GRB color for LED 1.
 * @param color_led2_grb GRB color for LED 2.
 */
void ws2812_set_leds_solid(uint32_t color_led1_grb, uint32_t color_led2_grb);

/**
 * @brief Displays a status/error code by blinking the LEDs.
 * The "on" phase of the blink will use the two defined colors for the code.
 * The "off" phase of the blink will set both LEDs to COLOR_BLACK.
 *
 * @param code The status/error code to display (from led_status_code_t).
 * @param blink_count Number of times to blink the code.
 * @param on_time_ms Duration (in milliseconds) the LEDs are ON for each blink.
 * @param off_time_ms Duration (in milliseconds) the LEDs are OFF for each blink (between colored states).
 */
void ws2812_leds_blink_code(led_status_code_t code, uint8_t blink_count, uint16_t on_time_ms, uint16_t off_time_ms);

/**
 * @brief Sets both LEDs to solid GREEN (Go For Launch status).
 */
void ws2812_leds_go_for_launch(void);

/**
 * @brief Turns off both LEDs (sets them to COLOR_BLACK).
 */
void ws2812_leds_turn_off(void);

/**
 * @brief Helper function to convert RGB color values to a single 32-bit GRB formatted value.
 * While colors are defined in GRB, this can be useful for dynamic color generation.
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return uint32_t GRB formatted color value.
 */
uint32_t ws2812_convert_rgb_to_grb(uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WS2812_UTILS_H
