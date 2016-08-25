#include "LED_Tunnel.h"

Tunnel::Tunnel()
{

}

void Tunnel::init()
{
    m_strips[0] = Adafruit_NeoPixel(6 * 3 * 42, g_led_pins[0], NEO_GRBW + NEO_KHZ800);
    m_strips[1] = Adafruit_NeoPixel(42 + 34 + 42, g_led_pins[1], NEO_GRBW + NEO_KHZ800);
    m_strips[2] = Adafruit_NeoPixel(6 * 3 * 42, g_led_pins[2], NEO_GRBW + NEO_KHZ800);
    uint8_t* data_start[3];

    for(uint8_t i = 0; i < 3; ++i)
    {
        m_strips[i].begin();
        m_strips[i].setBrightness(50);
        m_strips[i].show(); // Initialize all pixels to 'off'
        data_start[i] = const_cast<uint8_t*>(m_strips[i].getPixels());
    }

    const size_t gate_num_bytes = 3 * 42 * sizeof(uint32_t);

    // PIN 10
    m_gates[0] = Gate(data_start[0] + 5 * gate_num_bytes, 42, 42, 42, Gate::REVERSE);
    m_gates[1] = Gate(data_start[0] + 4 * gate_num_bytes, 42, 42, 42, Gate::NORMAL);
    m_gates[2] = Gate(data_start[0] + 3 * gate_num_bytes, 42, 42, 42, Gate::REVERSE);
    m_gates[3] = Gate(data_start[0] + 2 * gate_num_bytes, 42, 42, 42, Gate::NORMAL);
    m_gates[4] = Gate(data_start[0] + 1 * gate_num_bytes, 42, 42, 42, Gate::REVERSE);
    m_gates[5] = Gate(data_start[0] + 0 * gate_num_bytes, 42, 42, 42, Gate::NORMAL);

    // PIN 11
    m_gates[6] = Gate(data_start[1], 42, 34, 42, Gate::NORMAL);

    // PIN 12
    m_gates[7] = Gate(data_start[2] + 0 * gate_num_bytes, 42, 42, 42, Gate::NORMAL);
    m_gates[8] = Gate(data_start[2] + 1 * gate_num_bytes, 42, 42, 42, Gate::REVERSE);
    m_gates[9] = Gate(data_start[2] + 2 * gate_num_bytes, 42, 42, 42, Gate::NORMAL);
    m_gates[10] = Gate(data_start[2] + 3 * gate_num_bytes, 42, 42, 42, Gate::REVERSE);
    m_gates[11] = Gate(data_start[2] + 4 * gate_num_bytes, 42, 42, 42, Gate::NORMAL);
    m_gates[12] = Gate(data_start[2] + 5 * gate_num_bytes, 42, 42, 42, Gate::REVERSE);
}

uint8_t Tunnel::brightness() const
{
    return m_strips[0].getBrightness();
}

void Tunnel::set_brightness(uint8_t the_brightness)
{
    for(uint8_t i = 0; i < 3; ++i)
    {
        m_strips[i].setBrightness(the_brightness);
    }
}

void Tunnel::update()
{
    for(uint8_t i = 0; i < 3; ++i)
    {
        m_strips[i].show();
    }
}

Gate::Gate(uint8_t *data_start, uint16_t num_left, uint16_t num_top, uint16_t num_right,
            Direction the_dir):
m_data(data_start),
m_num_leds({num_left, num_top, num_right}),
m_direction(the_dir)
{

}

void Gate::set_all_pixels(uint32_t the_color)
{
    uint8_t *ptr = m_data,
    *end_ptr = m_data + (m_num_leds[0] + m_num_leds[1] + m_num_leds[2]) * sizeof(uint32_t);
    const size_t inc = sizeof(uint32_t);

    for(; ptr < end_ptr; ptr += inc){ *((uint32_t*)ptr) = the_color; }
}
