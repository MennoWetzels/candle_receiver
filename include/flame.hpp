#ifndef flame_h
#define flame_h

#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES LEDC_TIMER_8_BIT // Set duty resolution to 8 bits 0-256
#define LEDC_FREQUENCY (1000)          // Frequency in Hertz. Set frequency at 4 kHz

#define LED1_IO (2) //(32) // Define the output GPIO
#define LED1_CH LEDC_CHANNEL_0
#define LED2_IO (33) // Define the output GPIO
#define LED2_CH LEDC_CHANNEL_1

void init_led_controller();

class led
{
public:
    void init(uint8_t pin, ledc_channel_t channel);
    void setDuty(uint32_t duty);

private:
    ledc_channel_config_t ledc_channel;
    ledc_channel_t output_channel;
    int _pin;
};

class flame
{
public:
    void setup(uint8_t pin, ledc_channel_t channel, uint8_t alpha = 20, uint8_t delay = 100);
    void adjust(uint8_t brightness, uint8_t alpha);
    void adjust_brightness(uint8_t brightness);
    void adjust_alpha(uint8_t alpha);
    void flicker();

private:
    int _pin;
    int _brightness = 0; // soft start
    int _oldBrightness = 0;
    TickType_t _updateDelay;
    TickType_t _nextUpdate;
    int _alpha; // filter coefficient (0..100). Low means slow changes
    int _masterBrightness = 255;
    led output;
};

#endif