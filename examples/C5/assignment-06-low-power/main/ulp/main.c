#include <stdint.h>
#include "ulp_lp_core.h"
#include "ulp_lp_core_utils.h"

// Variables shared between HP and LP core
volatile uint32_t counter = 0;

int main(void)
{
    // Increment our counter
    counter++;

    // Wake the main CPU once the counter reaches 5
    if (counter >= 5) {
        ulp_lp_core_wakeup_main_processor();
        counter = 0;
    }

    // Go back to sleep until the LP timer wakes us again
    ulp_lp_core_halt();
    return 0;
}
