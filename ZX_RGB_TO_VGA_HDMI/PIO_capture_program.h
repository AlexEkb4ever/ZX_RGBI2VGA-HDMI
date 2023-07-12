#pragma once

#include "inttypes.h"
#include "hardware/pio.h"
#include "g_config.h"




//extern uint16_t pio_program1_instructions[];
//extern const struct pio_program pio_program1;

extern const struct pio_program pio_program0_CAP;
extern const struct pio_program pio_program1_CAP;
extern const struct pio_program pio_program2_CAP;

extern uint16_t pio_program0_instructions[];
extern uint16_t pio_program1_instructions[];
extern uint16_t pio_program2_instructions[];