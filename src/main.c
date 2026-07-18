#include <stdio.h>
#include "pc_component_function.h"
#include "pc_file.h"

int main(void)
{
    struct component_manager c_mgr;

    component_mgr_init(&c_mgr);

    printf("=== STEP 1: LOADING COMPONENTS ===\n");
    file_load_components(&c_mgr);

    printf("\n=== STEP 2: SHOW ALL COMPONENTS ===\n");
    component_ui_show_all(&c_mgr);

    printf("\n=== STEP 3: TRACK LOW STOCK ===\n");
    component_ui_track_low_stock(&c_mgr);

    printf("\n=== STEP 4: SAVING COMPONENTS BACK TO FILE ===\n");
    file_save_components(&c_mgr);

    return 0;
}