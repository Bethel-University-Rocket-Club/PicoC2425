#include "deviceConfig.h"

BMP280* Devices::GetPressureSensor() {
    i2c_inst_t* i2c;
    if(BMP280BUS == 0) {
        i2c = i2c0;
    } else {
        i2c = i2c1;
    }
    i2c_init(i2c, BMP280FREQUENCY);
    gpio_set_function(BMP280SDA, GPIO_FUNC_I2C);
    gpio_set_function(BMP280SCL, GPIO_FUNC_I2C);
    gpio_pull_up(BMP280SDA);
    gpio_pull_up(BMP280SCL);
    sleep_ms(500);
    return new BMP280(BMP280_I2CADDR, i2c);
}

ADXL377* Devices::GetAccelerometer() {
    return new ADXL377(0, 0, ADXL377ANALOG);
}

GTU7 *Devices::GetGPS() {
    uart_init(uart0, 9600);
    gpio_set_function(0, GPIO_FUNC_UART); // TX (GPS RX)
    gpio_set_function(1, GPIO_FUNC_UART); // RX (GPS TX)
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, true); // For DMA
    return new GTU7(uart0);
}

MPX5700GP* Devices::GetPitotTube() {
    return new MPX5700GP(MPX5700GPANALOG);
}
