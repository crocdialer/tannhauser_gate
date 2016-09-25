#include "WaveSimulation.h"

WaveSimulation::WaveSimulation(uint32_t max_num_waves):
m_track_length(12.f),
m_propagation_speed(5.f),
m_decay_secs(5.f)
{
    m_max_num_waves = max_num_waves;
    m_positions = new float[max_num_waves];
    m_intensities = new float[max_num_waves];

    memset(m_positions, 0, sizeof(float) * max_num_waves);
    memset(m_intensities, 0, sizeof(float) * max_num_waves);
}

WaveSimulation::~WaveSimulation()
{
    delete[](m_positions);
    delete[](m_intensities);
}

void WaveSimulation::update(uint32_t the_delta_time)
{
    float delta_secs = the_delta_time / 1000.f;

    for(uint32_t i = 0; i < m_max_num_waves; i++)
    {
        if(m_intensities[i] >= s_epsilon)
        {
            float new_pos = m_positions[i] + m_propagation_speed * delta_secs;

            // end of track?
            if(new_pos <= m_track_length)
            {
                 m_positions[i] = new_pos;
                 m_intensities[i] -= 1.f / m_decay_secs * delta_secs;
            }
            else
            {
                m_positions[i] = m_track_length;
                m_intensities[i] = 0.f;
            }
        }
    }
}

void WaveSimulation::emit_wave(float the_start_intesity, float the_start_pos)
{
    // find unused slot
    for(uint32_t i = 0; i < m_max_num_waves; i++)
    {
        if(m_intensities[i] < s_epsilon)
        {
            // found an unused slot
            m_intensities[i] = the_start_intesity;
            m_positions[i] = the_start_pos;
            break;
        }
    }
}

float WaveSimulation::intensity_at_position(float the_position)
{
    float sum = 0.f;
    float quad_factor = 50.f;

    for(uint32_t i = 0; i < m_max_num_waves; i++)
    {
        if(m_intensities[i] >= s_epsilon)
        {
            float distance = m_positions[i] - the_position;
            sum += m_intensities[i] / (1.f + quad_factor * distance * distance);
        }
    }
    return sum;
}
