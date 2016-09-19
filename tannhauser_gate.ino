#include "utils.h"
#include "RunningMedian.h"
#include "LED_Tunnel.h"

// update rate in Hz
#define UPDATE_RATE 10

// some pin defines
#define BARRIER_INTERRUPT_PIN A0
#define MIC_PIN A1
#define POTI_PIN A2

#define ADC_BITS 12
const float ADC_MAX = (1 << ADC_BITS) - 1.f;

#define SERIAL_BUFSIZE 128
char g_serial_buf[SERIAL_BUFSIZE];
uint32_t g_buf_index = 0;

// mic sampling
int g_mic_sample_window = 50;
unsigned long g_mic_start_millis = 0;  // start of sample window
volatile unsigned int g_mic_peak_to_peak = 0;   // peak-to-peak level
unsigned int g_mic_signal_max = 0;
unsigned int g_mic_signal_min = 4096;
float g_mic_lvl = 0.f; // 0.0 ... 1.0

// mic filtering
const uint16_t g_num_samples = 3;
const uint16_t g_sense_interval = 0;
RunningMedian g_running_median = RunningMedian(g_num_samples);

// helper variables for time measurement
long g_last_time_stamp = 0;
uint32_t g_time_accum = 0;

// update interval in millis
const int g_update_interval = 1000 / UPDATE_RATE;

// helper for flashing PIN 13 (red onboard LED)
// to indicate update frequency
bool g_indicator = false;

// tunnel variables
Tunnel g_tunnel;
uint32_t g_current_index = 0;

// lightbarrier handling (changed in ISR, must be volatile)
volatile bool g_barrier_lock = false;
volatile long g_barrier_timestamp = 0;

//! define our run-modes here
enum RunMode
{
    MODE_NORMAL,
    MODE_DEBUG
} g_run_mode = MODE_NORMAL;

//! interrupt routine for lightbarrier status
void barrier_ISR()
{
     g_barrier_lock = digitalRead(BARRIER_INTERRUPT_PIN);
     if(g_barrier_lock){ g_barrier_timestamp = millis(); }
}


void update_tunnel(uint32_t the_delta_time)
{
    // not working with current approach
    g_tunnel.set_brightness(50 * g_mic_lvl);

    auto col = Adafruit_NeoPixel::Color(150, 255 * g_mic_lvl, 0, g_gamma[40]);
    g_tunnel.clear();
    g_tunnel.gates()[g_current_index].set_all_pixels(col);
    g_current_index = (g_current_index + 1) % g_tunnel.num_gates();
    g_tunnel.update(the_delta_time);
}

void setup()
{
    // drives our status LED
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    // interrupt from lightbarrier
    pinMode(BARRIER_INTERRUPT_PIN, INPUT);
    attachInterrupt(BARRIER_INTERRUPT_PIN, barrier_ISR, CHANGE);

    // set ADC resolution (default 10 bits, maximum 12 bits)
    analogReadResolution(ADC_BITS);

    // while(!Serial){ delay(10); }
    Serial.begin(115200);

    g_tunnel.init();
}

void loop()
{
    // time measurement
    uint32_t delta_time = millis() - g_last_time_stamp;
    g_last_time_stamp = millis();
    g_time_accum += delta_time;

    // update current microphone value
    process_mic_input();

    if(g_time_accum >= g_update_interval)
    {
        g_time_accum = 0;

        digitalWrite(13, g_indicator);
        g_indicator = !g_indicator;

        switch(g_run_mode)
        {
            case MODE_NORMAL:
            // update animation
            update_tunnel(delta_time);
            break;
        }

        // read debug inputs
        process_serial_input();

        // debug output
        // sprintf(g_serial_buf, "mic-lvl: %d\n", g_mic_peak_to_peak);
        // Serial.write(g_serial_buf);
    }
}

void process_mic_input()
{
    int sample  = analogRead(MIC_PIN);

    // toss out spurious readings
    if(sample <= ADC_MAX)
    {
        g_mic_signal_min = min(g_mic_signal_min, sample);  // save just the min levels
        g_mic_signal_max = max(g_mic_signal_max, sample);  // save just the max levels
    }

    if(millis() > g_mic_start_millis + g_mic_sample_window)
    {
        g_mic_peak_to_peak = g_mic_signal_max - g_mic_signal_min;  // max - min = peak-peak amplitude
        g_mic_signal_max = 0;
        g_mic_signal_min = ADC_MAX;
        g_mic_start_millis = millis();
        g_mic_lvl *= .96f;

        const uint32_t thresh = 15;

        // read mic val
        float v = map_value<float>(g_mic_peak_to_peak, thresh, 200, 0.f, 1.f);
        g_mic_lvl = max(g_mic_lvl, smoothstep(0.f, 1.f, v));
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
                g_run_mode = MODE_DEBUG;
                g_tunnel.clear();
                g_tunnel.gates()[index].set_all_pixels(ORANGE);
                g_tunnel.update(33);
            }
            else{ g_run_mode = MODE_NORMAL; }
        }
    }
}
