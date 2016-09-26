#include "utils.h"
#include "ADC_Sampler.h"
#include "LED_Tunnel.h"
#include "WaveSimulation.h"

// update rate in Hz
#define UPDATE_RATE 60

// some pin defines
#define BARRIER_INTERRUPT_PIN A0
#define MIC_PIN A1
#define POTI_PIN A2
#define ADC_BITS 10
#define SERIAL_BUFSIZE 128

const float ADC_MAX = (1 << ADC_BITS) - 1.f;

char g_serial_buf[SERIAL_BUFSIZE];
uint32_t g_buf_index = 0;

// mic sampling
const uint32_t g_mic_sample_window = 50;
volatile uint32_t g_mic_signal_max = 0;
volatile uint32_t g_mic_signal_min = ADC_MAX;
uint32_t g_mic_start_millis = 0;  // start of sample window
uint32_t g_mic_peak_to_peak = 0;   // peak-to-peak level
float g_mic_lvl = 0.f; // 0.0 ... 1.0
float g_gain = 3.f;

// continuous sampling with timer interrupts and custom ADC settings
ADC_Sampler g_adc_sampler;

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
    MODE_SPARKLE = 1 << 0,
    MODE_WAVES = 1 << 1,
    MODE_NEBULA = 1 << 2,
    MODE_DEBUG = 1 << 3
};
uint32_t g_run_mode = MODE_SPARKLE | MODE_WAVES;

// our wave simulation object
WaveSimulation g_wave_sim;

// disabled when set to 0
int32_t g_random_wave_timer = 1;

uint32_t g_wave_charge_duration = 3000;

const uint32_t g_idle_timeout = 10000;

//! value callback from ADC_Sampler ISR
void adc_callback(uint32_t the_sample)
{
    if(the_sample <= ADC_MAX)
    {
        g_mic_signal_min = min(g_mic_signal_min, the_sample);
        g_mic_signal_max = max(g_mic_signal_max, the_sample);
    }
}

//! interrupt routine for lightbarrier status
void barrier_ISR()
{
     int new_val = digitalRead(BARRIER_INTERRUPT_PIN);

     // barrier released
     if(g_barrier_lock && !new_val)
     {
         g_barrier_timestamp = millis();
         g_wave_sim.emit_wave(random<float>(.8f, 1.2f));
     }
     g_barrier_lock = new_val;
}

void update_sparkling(uint32_t the_delta_time)
{
    uint32_t num_random_pix = g_mic_lvl * 400/*per sec*/ * the_delta_time / 1000.f;
    g_tunnel.add_random_pixels(num_random_pix, 600);
    // Serial.print("num_random_pix: "); Serial.println(num_random_pix);
}

void update_waves(uint32_t the_delta_time)
{
    // update wave simulation
    g_wave_sim.update(the_delta_time);

    g_random_wave_timer -= g_time_accum;

    // idle timeout and wave timer elapsed
    if(millis() - g_barrier_timestamp > g_idle_timeout &&
       g_random_wave_timer < 0)
    {
        // emit wave
        g_wave_sim.emit_wave(random<float>(0.5f, 1.f));

        // schedule next wave
        g_random_wave_timer = random<int32_t>(500, 5000);
    }

    // g_tunnel.set_brightness(20 + 50 * g_mic_lvl);
    auto col = Adafruit_NeoPixel::Color(150, 50, 0, g_gamma[40]);

    // start bias for 1st gate in meters
    float pos_x = 0.3;

    // distance between two gates in meters
    const float step = 0.88f;

    for(int i = 0; i < g_tunnel.num_gates(); i++)
    {
        auto fade_col = fade_color(col, g_wave_sim.intensity_at_position(pos_x));
        g_tunnel.gates()[i].set_all_pixels(fade_col);
        pos_x += step;
    }
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

    // start mic sampling
    g_adc_sampler.set_adc_callback(&adc_callback);
    g_adc_sampler.begin(MIC_PIN, 22050);

    g_tunnel.init();
}

void loop()
{
    // time measurement
    uint32_t delta_time = millis() - g_last_time_stamp;
    g_last_time_stamp = millis();
    g_time_accum += delta_time;

    // update current microphone value
    process_mic_input(delta_time);

    if(g_time_accum >= g_update_interval)
    {
        // flash red indicator LED
        digitalWrite(13, g_indicator);
        g_indicator = !g_indicator;

        // read debug inputs
        process_serial_input();

        // do nothing here while debugging
        if(g_run_mode & MODE_DEBUG){ return; }

        // clear everything to black
        g_tunnel.clear();

        // run stages depending on current mode
        if(g_run_mode & MODE_WAVES){ update_waves(g_time_accum); }
        if(g_run_mode & MODE_SPARKLE){ update_sparkling(g_time_accum); }

        // send new color values to strips
        g_tunnel.update(delta_time);

        // clear time accumulator
        g_time_accum = 0;
    }
}

void process_mic_input(uint32_t the_delta_time)
{
    // decay
    float decay = 1.f * the_delta_time / 1000.f;
    g_mic_lvl = max(0, g_mic_lvl - decay);

    if(millis() > g_mic_start_millis + g_mic_sample_window)
    {
        g_mic_peak_to_peak = g_mic_signal_max - g_mic_signal_min;  // max - min = peak-peak amplitude
        g_mic_signal_max = 0;
        g_mic_signal_min = ADC_MAX;
        g_mic_start_millis = millis();

        g_mic_peak_to_peak = max(0, g_mic_peak_to_peak - 2);

        // read mic val
        float v = clamp<float>(g_gain * g_mic_peak_to_peak / 80.f, 0.f, 1.f);
        g_mic_lvl = max(g_mic_lvl, v);
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
                g_tunnel.update(0);
            }
            else{ g_run_mode = MODE_SPARKLE | MODE_WAVES; }
        }
    }
}
