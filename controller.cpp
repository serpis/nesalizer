#include "common.h"

#include "controller.h"
#include "cpu.h"
#include "input.h"

static uint8_t controller_bits[2];

// Set by writing $4016:0. When enabled, the shift registers in the controllers
// are initialized from the buttons (level triggered).
static bool strobe_latch;

uint8_t read_controller(unsigned n) {
    LOG_CONTROLLER("Read controller %u\n", n);

    // Results for standard controller:
    // D7-D5: Open bus (this usually results in an OR by $40)
    // D4-D1: Always 0
    // D0: Result

    // Reading the controllers with the strobe latch on returns the state of A
    // over and over. Happens rarely.
    if (strobe_latch)
        return (cpu_data_bus & 0xE0) | (get_button_states(n) & 1);

    uint8_t const result = (cpu_data_bus & 0xE0) | (controller_bits[n] & 1);
    // 1's are shifted in on an official Nintendo controller, so emulate that
    controller_bits[n] = 0x80 | (controller_bits[n] >> 1);
    return result;
}

void write_controller_strobe(bool strobe) {
    LOG_CONTROLLER("Write $%02X to controller %u\n", value, n);

    // On a real controller the button states are continuously reloaded while
    // the strobe latch is on. Emulate this by latching the button states when
    // it goes from set to unset.
    if (strobe_latch && !strobe)
        for (unsigned n = 0; n < 2; ++n)
            controller_bits[n] = get_button_states(n);

    strobe_latch = strobe;
}
