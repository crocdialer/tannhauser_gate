#include "LED_Tunnel.h"

// update rate in Hz
#define UPDATE_RATE 2

#define SERIAL_BUFSIZE 128
uint8_t g_serial_buf[SERIAL_BUFSIZE];
uint32_t g_buf_index = 0;

// helper variables for time measurement
long g_last_time_stamp = 0;
uint32_t g_time_accum = 0;

// update interval in millis
const int g_update_interval = 1000 / UPDATE_RATE;

bool g_indicator = false;

// tunnel variables
Tunnel g_tunnel;
uint32_t g_current_index = 0;

// Color defines (BRGW)
static const uint32_t
WHITE = Adafruit_NeoPixel::Color(0, 0, 0, 255),
PURPLE = Adafruit_NeoPixel::Color(150, 235, 0, 20),
ORANGE = Adafruit_NeoPixel::Color(0, 255, 50, 40),
BLACK = 0;

void update_tunnel()
{
    g_tunnel.clear();
    g_tunnel.gates()[g_current_index].set_all_pixels(PURPLE);
    g_current_index = (g_current_index + 1) % g_tunnel.num_gates();
    g_tunnel.update();
}

void setup()
{
    // drives our status LED
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    // while(!Serial){ delay(10); }
    Serial.begin(115200);

    g_tunnel.init();

    for(uint8_t i = 0; i < g_tunnel.num_gates(); i++)
    {
        // g_tunnel.gates()[i].set_all_pixels(0);
        sprintf((char*)g_serial_buf, "datastart(%d): %d\n", (int)i, (int)(g_tunnel.gates()[i].data()));
        Serial.print((const char*)g_serial_buf);
    }
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
        // update_tunnel();

        // read debug inputs
        process_serial_input();
    }
}

void process_serial_input()
{
    while(Serial.available())
    {
        // get the new byte:
        uint8_t c = Serial.read();
        if(c == '\0'){ continue; }

        // add it to the buf
        g_serial_buf[g_buf_index % SERIAL_BUFSIZE] = c;
        g_buf_index = (g_buf_index + 1) % SERIAL_BUFSIZE;

        // if the incoming character is a newline, set a flag
        if (c == '\n')
        {
            // memcpy(&col, g_serial_buf, 3);
            int index = atoi((const char*)g_serial_buf);
            g_buf_index = 0;
            memset(g_serial_buf, 0, SERIAL_BUFSIZE);

            Serial.println(index);

            if(index >= 0 && index < g_tunnel.num_gates())
            {
                g_tunnel.clear();
                g_tunnel.gates()[index].set_all_pixels(ORANGE);
                // g_tunnel.gates()[0].set_all_pixels(PURPLE);
                // g_tunnel.gates()[index].set_pixel(0, ORANGE);
                g_tunnel.update();
            }
            else
            {
                for(uint8_t i = 0; i < g_tunnel.num_gates(); i++)
                {
                    g_tunnel.gates()[i].set_all_pixels(ORANGE);
                    g_tunnel.update();
                }
            }
        }
    }
}
