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
        data_start[i] = (uint8_t*)m_strips[i].getPixels();
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

void Tunnel::clear()
{
    for(uint8_t i = 0; i < m_num_gates; i++)
    {
        m_gates[i].set_all_pixels(0);
    }
}

void Tunnel::update()
{
    for(int i = 0; i < 3; ++i){ m_strips[i].show(); }
}

Gate::Gate():
m_data(nullptr),
m_num_leds(0),
m_direction(NORMAL)
{}

Gate::Gate(uint8_t *data_start, uint16_t num_left, uint16_t num_top, uint16_t num_right,
           Direction the_dir):
m_data(data_start),
m_num_leds(num_left + num_top + num_right),
m_direction(the_dir)
{
    m_seq_length[0] = num_left;
    m_seq_length[1] = num_top;
    m_seq_length[2] = num_right;
}

void Gate::set_pixel(uint32_t the_index, uint32_t the_color)
{
    if(m_data && the_index < m_num_leds)
    {
        the_index = m_direction == NORMAL ?
            the_index : (m_num_leds - 1 - the_index);

        uint32_t *ptr = (uint32_t*)m_data;
        ptr[the_index] = the_color;
    }
}

void Gate::set_all_pixels(uint32_t the_color)
{
    if(!m_data) return;
    uint32_t *ptr = (uint32_t*)m_data,
    *end_ptr = ptr + m_num_leds;

    for(; ptr < end_ptr; ++ptr){ *ptr = the_color; }
}
