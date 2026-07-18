/* invoice_ui.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "invoice_ui.h"
#include "pc_invoice_function.h"
#include "pc_component_function.h"

static void clear_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

static void print_single_invoice(const struct invoice *inv_p)
{
    printf("Invoice ID: %u | Customer: %-15s | Status: %-10s | Total Items: %u\n",
           inv_p->id, inv_p->customer_name, status_names[inv_p->status], inv_p->comp_count);
}

void invoice_ui_show_all(struct invoice_manager *i_mgr)
{
    if (!i_mgr || i_mgr->count == 0)
    {
        printf("No invoices found.\n");
        return;
    }
    printf("\n=== INVOICE LIST ===\n");
    for (uint32_t i = 0; i < i_mgr->count; i++)
    {
        print_single_invoice(&i_mgr->invoices[i]);
    }
}

void invoice_ui_show_single(struct invoice_manager *i_mgr)
{
    uint32_t id;
    printf("Enter Invoice ID to search: ");
    if (scanf("%u", &id) != 1)
    {
        clear_stdin();
        return;
    }
    struct invoice *inv = invoice_find_by_id(i_mgr, id);
    if (inv)
    {
        printf("\n--- INVOICE DETAILS ---\n");
        print_single_invoice(inv);
        printf("Component IDs: ");
        for (uint32_t i = 0; i < inv->comp_count; i++)
        {
            printf("[%u] ", inv->component_ids[i]);
        }
        printf("\n");
    }
    else
    {
        printf("Invoice not found.\n");
    }
}

void invoice_ui_create(struct invoice_manager *i_mgr, struct component_manager *c_mgr)
{
    struct invoice tmp;
    memset(&tmp, 0, sizeof(tmp));
    tmp.status = STATUS_PENDING;

    clear_stdin();
    printf("Enter Customer Name: ");
    fgets(tmp.customer_name, sizeof(tmp.customer_name), stdin);
    tmp.customer_name[strcspn(tmp.customer_name, "\n")] = '\0';

    printf("Enter number of components to add: ");
    uint32_t amount;
    if (scanf("%u", &amount) != 1)
        return;

    for (uint32_t i = 0; i < amount && i < MAXIMUM_INVOICE_COMPONENTS; i++)
    {
        uint32_t cid;
        printf("Enter Component ID #%u: ", i + 1);
        if (scanf("%u", &cid) == 1 && component_find_by_id(c_mgr, cid) != NULL)
        {
            tmp.component_ids[tmp.comp_count++] = cid;
        }
        else
        {
            printf("Invalid or missing component. Skipped.\n");
        }
    }

    char msg[256];
    if (invoice_check_compatibility(&tmp, c_mgr, msg, sizeof(msg)) != 0)
    {
        printf("\n[WARNING] Assembly Compatibility Check Failed:\n%s\n", msg);
        printf("Invoice created anyway, but flagged for review.\n");
    }
    else
    {
        printf("\n[OK] Assembly Check Passed: %s\n", msg);
    }

    if (invoice_add(i_mgr, &tmp) == 0)
    {
        printf("Invoice created successfully!\n");
    }
}

void invoice_ui_edit(struct invoice_manager *i_mgr)
{
    uint32_t id;
    printf("Enter Invoice ID to edit: ");
    if (scanf("%u", &id) != 1)
    {
        clear_stdin();
        return;
    }

    struct invoice *target = invoice_find_by_id(i_mgr, id);
    if (!target)
    {
        printf("Invoice not found.\n");
        return;
    }

    struct invoice tmp = *target;
    printf("Select new status (0: PENDING, 1: ASSEMBLING, 2: COMPLETED, 3: CANCELLED): ");
    uint32_t st;
    if (scanf("%u", &st) == 1 && st < STATUS_COUNT)
    {
        tmp.status = (enum assembly_status)st;
        invoice_update(i_mgr, id, &tmp);
        printf("Invoice updated.\n");
    }
    else
    {
        printf("Invalid status.\n");
    }
}

void invoice_ui_remove(struct invoice_manager *i_mgr)
{
    uint32_t id;
    printf("Enter Invoice ID to delete: ");
    if (scanf("%u", &id) != 1)
    {
        clear_stdin();
        return;
    }

    if (invoice_delete(i_mgr, id) == 0)
        printf("Invoice %u deleted successfully.\n", id);
    else
        printf("Failed to delete invoice.\n");
}

void invoice_ui_check_status(struct invoice_manager *i_mgr, struct component_manager *c_mgr)
{
    uint32_t id;
    printf("Enter Invoice ID to check compatibility/status: ");
    if (scanf("%u", &id) != 1)
    {
        clear_stdin();
        return;
    }

    struct invoice *inv = invoice_find_by_id(i_mgr, id);
    if (!inv)
    {
        printf("Invoice not found.\n");
        return;
    }

    char msg[256];
    printf("\n--- STATUS CHECK ---\n");
    printf("Current Status: %s\n", status_names[inv->status]);

    if (invoice_check_compatibility(inv, c_mgr, msg, sizeof(msg)) == 0)
    {
        printf("Hardware Compatibility: [PASS] %s\n", msg);
    }
    else
    {
        printf("Hardware Compatibility: [FAIL] %s\n", msg);
    }
}