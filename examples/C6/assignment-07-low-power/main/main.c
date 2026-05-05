#include <stdio.h>
#include "ulp_lp_core.h"

void app_main(void)
{
    printf("Workshop example: low-power / LP core headers resolve for this target.\n");
    (void)ulp_lp_core_wakeup_main_processor;
}
