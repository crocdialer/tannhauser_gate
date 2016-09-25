#pragma once
#include <Arduino.h>

class WaveSimulation
{
public:
    static constexpr float s_epsilon = 0.001f;

    WaveSimulation(uint32_t max_num_waves = 3);
    ~WaveSimulation();

    void update(uint32_t the_delta_time);

    void emit_wave(float the_start_intesity = 1.f, float the_start_pos = 0.f);

    float intensity_at_position(float the_position);

    float track_legth() const { return m_track_length; }
    void set_track_length(float the_track_length){ m_track_length = the_track_length; }

    float propagation_speed() const;
    void set_propagation_speed(float the_propagation_speed);

    float decay_secs() const { return m_decay_secs; }
    void set_decay_secs(float the_decay_secs){ m_decay_secs = the_decay_secs; }

private:
    float m_track_length;
    float m_decay_secs;
    float m_global_propagation_speed;
    float* m_positions;
    float* m_intensities;
    float* m_propagation_speed;
    uint32_t m_max_num_waves;
};
