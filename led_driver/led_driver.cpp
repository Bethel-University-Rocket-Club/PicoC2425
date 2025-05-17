#include "led_driver.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h" // This is the header generated from your ws2812.pio file

// --- PIO Instance and State Machine (static to this file) ---
static PIO pio_instance = pio0; // Use PIO0 by default
static uint sm_instance = 0;    // Use State Machine 0 by default

/**
 * @brief Low-level function to send a single pixel's color data to the PIO state machine.
 * The WS2812 PIO program expects the 24-bit GRB data to be in the most significant
 * bits of a 32-bit word (shifted left by 8 bits).
 * @param pixel_grb The 24-bit GRB color value.
 */
static inline void _ws2812_pio_put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio_instance, sm_instance, pixel_grb << 8u);
}

// --- Public API Function Implementations ---

void ws2812_leds_init(void) {
    // You can choose pio0 or pio1 if needed.
    // By default, pio_instance is pio0.
    // Example: pio_instance = pio1; // if you wanted to use the other PIO block
    
    // Claim an unused State Machine on the chosen PIO instance.
    // The 'true' argument means pio_claim_unused_sm will panic if no SM is available.
    // This ensures we get a dedicated state machine for our LED strip.
    sm_instance = pio_claim_unused_sm(pio_instance, true);

    // Add the WS2812 PIO program to the PIO instruction memory.
    // This loads the assembly code from ws2812.pio into the PIO block.
    // The function returns the offset in instruction memory where the program was loaded.
    uint offset = pio_add_program(pio_instance, &ws2812_program);

    // Initialize the PIO state machine with the WS2812 program.
    // This function (defined in ws2812.pio.h, generated from ws2812.pio)
    // configures the state machine (clock divider, pin mapping, wrap addresses, etc.)
    // to correctly drive the WS2812 LEDs.
    // - pio_instance: The PIO block (pio0 or pio1).
    // - sm_instance: The state machine number.
    // - offset: The program offset in instruction memory.
    // - WS2812_PIN_CFG: The GPIO pin connected to the LED data line.
    // - 800000: The serial data rate in Hz (800kHz for WS2812).
    // - IS_RGBW_CFG: Boolean indicating if the LEDs are RGBW type.
    ws2812_program_init(pio_instance, sm_instance, offset, WS2812_PIN_CFG, 800000, IS_RGBW_CFG);

    // Ensure LEDs are off at startup for a clean initial state.
    ws2812_leds_turn_off();
}

uint32_t ws2812_convert_rgb_to_grb(uint8_t r, uint8_t g, uint8_t b) {
    // WS2812 LEDs typically expect data in GRB (Green, Red, Blue) order.
    // This function takes standard R, G, B values and packs them into a 
    // 32-bit integer in GRB format.
    // Green is shifted to the most significant byte, Red to the middle, Blue to the least.
    return ((uint32_t)(g) << 16) |
           ((uint32_t)(r) << 8)  |
           (uint32_t)(b);
}

void ws2812_set_leds_solid(uint32_t color_led1_grb, uint32_t color_led2_grb) {
    // Send the color for the first LED.
    _ws2812_pio_put_pixel(color_led1_grb);
    
    // If we are configured to use more than one LED, send the color for the second LED.
    // NUM_LEDS_CFG is defined in ws2812_utils.h
    if (NUM_LEDS_CFG > 1) { 
        _ws2812_pio_put_pixel(color_led2_grb);
    }
    // Note: For a strip with more than 2 LEDs, this function would need to be
    // modified, perhaps to take an array of colors or a variable number of arguments,
    // and then loop through _ws2812_pio_put_pixel for each LED.
}

void ws2812_leds_turn_off(void) {
    // Set both configured LEDs to COLOR_BLACK (which is 0x000000).
    ws2812_set_leds_solid(COLOR_BLACK, COLOR_BLACK);
}

void ws2812_leds_go_for_launch(void) {
    // Set both configured LEDs to COLOR_GREEN to indicate a "go" status.
    ws2812_set_leds_solid(COLOR_GREEN, COLOR_GREEN);
}

void ws2812_leds_blink_code(led_status_code_t code, uint8_t blink_count, uint16_t on_time_ms, uint16_t off_time_ms) {
    uint32_t c1 = COLOR_BLACK, c2 = COLOR_BLACK; // Default to off if code is not found

    // Determine the two colors based on the provided status/error code.
    switch (code) {
        // Both LEDs will now have a defined color for error codes
        case LED_CODE_ERR_1:  c1 = COLOR_RED;     c2 = COLOR_RED;    break;
        case LED_CODE_ERR_2:  c1 = COLOR_RED;     c2 = COLOR_BLUE;  break;
        case LED_CODE_ERR_3:  c1 = COLOR_BLUE;     c2 = COLOR_RED;    break;
        case LED_CODE_ERR_4:  c1 = COLOR_BLUE;    c2 = COLOR_BLUE;  break;
        case LED_CODE_ERR_5:  c1 = COLOR_YELLOW;    c2 = COLOR_YELLOW; break;
        case LED_CODE_ERR_6:  c1 = COLOR_YELLOW;  c2 = COLOR_MAGENTA;    break;
        case LED_CODE_ERR_7:  c1 = COLOR_MAGENTA;  c2 = COLOR_YELLOW; break;
        case LED_CODE_ERR_8:  c1 = COLOR_MAGENTA;    c2 = COLOR_MAGENTA;     break;
        case LED_CODE_ERR_9:  c1 = COLOR_RED; c2 = COLOR_YELLOW;     break;
        case LED_CODE_ERR_10: c1 = COLOR_YELLOW;   c2 = COLOR_RED;    break;
        
        case LED_STATUS_GOOD: c1 = COLOR_GREEN;   c2 = COLOR_GREEN;   break;
        
        case LED_CODE_OFF:    // If LED_CODE_OFF is explicitly passed
        default:              // Or if an undefined code is passed
            // Ensure LEDs are turned off and stay off. No blinking for these cases.
            ws2812_leds_turn_off(); 
            return; // Exit function, no blinking needed for OFF or undefined codes.
    }

    // Proceed with blinking only if it's a defined error/status code that should blink.
    for (uint8_t i = 0; i < blink_count; ++i) {
        // "On" part of the blink: Set the LEDs to their defined colors for the code.
        ws2812_set_leds_solid(c1, c2); 
        sleep_ms(on_time_ms); // Keep them on for the specified duration.
        
        // "Off" part of the blink: Turn both LEDs off (black).
        ws2812_leds_turn_off();        
        sleep_ms(off_time_ms); // Keep them off for the specified duration.
    }
}
