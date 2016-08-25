#include "LED_Tunnel.h"

// update rate in Hz
#define UPDATE_RATE 5

// helper variables for time measurement
long g_last_time_stamp = 0;
uint32_t g_time_accum = 0;

// update interval in millis
const int g_update_interval = 1000 / UPDATE_RATE;

bool g_indicator = false;

// tunnel variables
Tunnel g_tunnel;
uint32_t g_current_index = 0;

void update_tunnel()
{
    for(uint8_t i = 0; i < g_tunnel.num_gates(); i++)
    {
        g_tunnel.gates()[i].set_all_pixels(0);
    }
    g_tunnel.gates()[g_current_index].set_all_pixels(Adafruit_NeoPixel::Color(0, 0, 0, 255));
    g_current_index = (g_current_index + 1) % g_tunnel.num_gates();
    g_tunnel.update();
}

void setup()
{
    // drives our status LED
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    Serial.begin(115200);

    g_tunnel.init();
}

void loop()
{
    // time measurement
    uint32_t delta_time = millis() - g_last_time_stamp;
    g_last_time_stamp = millis();
    g_time_accum += delta_time;

    if(g_time_accum >= g_update_interval)
    {
        // float delta_secs = g_time_accum / 1000.f;
        g_time_accum = 0;

        digitalWrite(13, g_indicator);
        g_indicator = !g_indicator;

        update_tunnel();
    }
}
