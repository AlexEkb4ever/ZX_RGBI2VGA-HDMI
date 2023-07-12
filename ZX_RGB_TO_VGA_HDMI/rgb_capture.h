#pragma once

#include "stdbool.h"
#include "inttypes.h"
#include "g_config.h"







extern uint32_t i_frame;
void set_cap_shx(int sh_x);
void set_cap_shy(int sh_y);
void check_cap_data(cap_set_t *cap_data);



void startCapture(cap_set_t *cap_data);

