#include "flame.hpp"

static const char *TAG = "flame"; // The log tagline.

int constrain(int value, int min, int max)
{
  if (value < min)
    return min;
  else if (value > max)
    return max;
  else
    return value;
}

void init_led_controller()
{
  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_MODE,
      .duty_resolution = LEDC_DUTY_RES,
      .timer_num = LEDC_TIMER,
      .freq_hz = LEDC_FREQUENCY, // Set output frequency at 4 kHz
      .clk_cfg = LEDC_AUTO_CLK,
      .deconfigure = false,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
}

void flame::setup(uint8_t pin, ledc_channel_t channel, uint8_t alpha, uint8_t delay)
{
  constrain(alpha, 2, 100);
  _pin = pin;
  _alpha = alpha;
  _updateDelay = pdMS_TO_TICKS(delay);
  // random first upate to prevent multiple flames looking synchronous
  _nextUpdate = xTaskGetTickCount() + (rand() % (_updateDelay + 1));
  output.init(pin, channel);
}

void flame::adjust(uint8_t brightness, uint8_t alpha)
{
  constrain(alpha, 2, 100);
  constrain(brightness, 20, 255);
  _alpha = alpha;
  _masterBrightness = brightness;
}

void flame::adjust_brightness(uint8_t brightness)
{
  constrain(brightness, 20, 255);
  _masterBrightness = brightness;
}

void flame::adjust_alpha(uint8_t alpha)
{
  constrain(alpha, 2, 100);
  _alpha = alpha;
}

void flame::flicker()
{
  if (xTaskGetTickCount() >= _nextUpdate)
  {
    _nextUpdate += _updateDelay;
    _brightness = (rand() % (255 + 1));
    if (_brightness < 10)
    {
      _brightness = 10;
    }
    // low pass filter the brightness changes
    _brightness = (_alpha * _brightness + (100 - _alpha) * _oldBrightness) / 100;
    _oldBrightness = _brightness;

    if (_masterBrightness > 0)
    {
      _brightness = (int)((float)_brightness * ((float)_masterBrightness) / 255);
    }
    else
    {
      _brightness = 0;
    }

    output.setDuty(_brightness);
    // ledcWrite(_pin, _brightness);
    // analogWrite(_pin, _brightness);
  }
}

void led::init(uint8_t pin, ledc_channel_t channel)
{
  _pin = pin;
  output_channel = channel;
  ledc_channel = {
      .gpio_num = _pin,
      .speed_mode = LEDC_MODE,
      .channel = output_channel,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER,
      .duty = 0, // Set duty to 0%
      .hpoint = 0,
      .flags = {
          .output_invert = 0,
      },
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
  ESP_LOGI(TAG, "Set up flame for io %d and channel %d", pin, (int)channel);
}

void led::setDuty(uint32_t duty)
{
  // ESP_LOGI(TAG, "Setting duty to %d", (int)duty);
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, output_channel, duty));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, output_channel));
}