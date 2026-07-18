/* invoice_file.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "invoice_file.h"

static void parse_components_string(struct invoice *inv, char *comp_str)
{
    inv->comp_count = 0;
    char *token = strtok(comp_str, "|");
    while (token && inv->comp_count < MAXIMUM_INVOICE_COMPONENTS)
    {
        inv->component_ids[inv->comp_count++] = (uint32_t)strtoul(token, NULL, 10);
        token = strtok(NULL, "|");
    }
}

void file_load_invoices(struct invoice_manager *mgr_p)
{
    if (!mgr_p)
        return;
    mgr_p->count = 0;
    mgr_p->next_id = 1;

    FILE *f = fopen(INVOICE_FILE, "r");
    if (!f)
        return;

    char buf[1024];
    while (fgets(buf, sizeof(buf), f))
    {
        if (strncmp(buf, "id,", 3) == 0)
            continue;

        struct invoice tmp;
        memset(&tmp, 0, sizeof(tmp));

        char comp_buffer[512] = {0};
        int parsed = sscanf(buf, "%u,%127[^,],%u,%u,%511s",
                            &tmp.id, tmp.customer_name, (uint32_t *)&tmp.status,
                            &tmp.comp_count, comp_buffer);

        if (parsed >= 4)
        {
            if (parsed == 5)
                parse_components_string(&tmp, comp_buffer);

            if (tmp.id >= mgr_p->next_id)
                mgr_p->next_id = tmp.id + 1;

            mgr_p->invoices[mgr_p->count++] = tmp;
        }
    }
    fclose(f);
}

void file_save_invoices(struct invoice_manager *mgr_p)
{
    if (!mgr_p)
        return;

    FILE *f = fopen(INVOICE_FILE, "w");
    if (!f)
        return;

    fprintf(f, "id,customer_name,status,comp_count,components\n");
    for (uint32_t i = 0; i < mgr_p->count; i++)
    {
        struct invoice *inv = &mgr_p->invoices[i];
        fprintf(f, "%u,%s,%u,%u,", inv->id, inv->customer_name, inv->status, inv->comp_count);

        for (uint32_t j = 0; j < inv->comp_count; j++)
        {
            fprintf(f, "%u%s", inv->component_ids[j], (j == inv->comp_count - 1) ? "" : "|");
        }
        fprintf(f, "\n");
    }
    fclose(f);
}