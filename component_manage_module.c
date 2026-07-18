#include <stdio.h>
#include <stdlib.h>
#include "pc_component_function.h"
#include "pc_file.h"

void print_menu(void)
{
    printf("\n=========================================\n");
    printf("    PC COMPONENT MANAGEMENT SYSTEM       \n");
    printf("=========================================\n");
    printf("1. Load components from CSV file\n");
    printf("2. Show all components\n");
    printf("3. Add a new component\n");
    printf("4. Update a component (Edit)\n");
    printf("5. Delete a component\n");
    printf("6. Track low stock items (<= 5)\n");
    printf("7. Sort components menu\n");
    printf("8. Save components to CSV file\n");
    printf("0. Save and Exit\n");
    printf("-----------------------------------------\n");
    printf("Your choice: ");
}

int main(void)
{
    struct component_manager c_mgr;
    int choice;
    int keep_running = 1;

    component_mgr_init(&c_mgr);
    file_load_components(&c_mgr);

    while (keep_running)
    {
        print_menu();

        if (scanf("%d", &choice) != 1)
        {
            printf("\n[Error] Invalid input! Please enter a number.\n");
            while (getchar() != '\n')
                ;
            continue;
        }
        /* Drain newline after scanf choice */
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;

        switch (choice)
        {
        case 1:
            printf("\n--- LOADING DATA ---\n");
            file_load_components(&c_mgr);
            break;

        case 2:
            component_ui_show_all(&c_mgr);
            break;

        case 3:
            component_ui_add(&c_mgr);
            break;

        case 4:
            component_ui_update(&c_mgr);
            break;

        case 5:
            component_ui_delete(&c_mgr);
            break;

        case 6:
            component_ui_track_low_stock(&c_mgr);
            break;

        case 7:
            component_ui_sort_menu(&c_mgr);
            break;

        case 8:
            printf("\n--- SAVING DATA ---\n");
            file_save_components(&c_mgr);
            break;

        case 0:
            printf("\nAuto-saving before exit...\n");
            file_save_components(&c_mgr);
            printf("Goodbye!\n");
            keep_running = 0;
            break;

        default:
            printf("\n[Warning] Choice out of range (0-8). Try again.\n");
            break;
        }
    }

    return 0;
}