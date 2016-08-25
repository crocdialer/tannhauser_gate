#include "LED_Tunnel.h"

// update rate in Hz
#define UPDATE_RATE 120

// helper variables for time measurement
long g_last_time_stamp = 0;
uint32_t g_time_accum = 0;

// update interval in millis
const int g_update_interval = 1000 / UPDATE_RATE;

bool g_indicator = false;

void setup()
{

}

void loop()
{

}
