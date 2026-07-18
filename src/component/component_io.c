#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pc_file.h"
#include "pc_component_function.h"

#define COMPONENT_FILE "pc_component_csv.csv"
#define LINE_BUFFER_SIZE 1024

static void clean_line_buffer(char *buf)
{
    if ((unsigned char)buf[0] == 0xEF &&
        (unsigned char)buf[1] == 0xBB &&
        (unsigned char)buf[2] == 0xBF)
    {
        memmove(buf, buf + 3, strlen(buf + 3) + 1);
    }
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
    {
        buf[len - 1] = '\0';
        len--;
    }
}

static enum component_category parse_category_str(const char *str)
{
    char upper_str[32] = {0};
    for (size_t i = 0; i < sizeof(upper_str) - 1 && str[i] != '\0'; i++)
    {
        upper_str[i] = (char)toupper((unsigned char)str[i]);
    }
    if (strcmp(upper_str, "CPU") == 0)
        return CPU;
    if (strcmp(upper_str, "MAINBOARD") == 0)
        return MAINBOARD;
    if (strcmp(upper_str, "RAM") == 0)
        return RAM;
    if (strcmp(upper_str, "SSD") == 0)
        return SSD;
    if (strcmp(upper_str, "GPU") == 0)
        return GPU;
    if (strcmp(upper_str, "PSU") == 0)
        return PSU;
    return CASE;
}

static int parse_csv_line(const char *line, char fields[][128], int max_fields)
{
    int field_count = 0;
    const char *ptr = line;
    while (*ptr != '\0' && field_count < max_fields)
    {
        char *out = fields[field_count];
        size_t out_idx = 0;
        size_t max_out = 127;
        if (*ptr == '"')
        {
            ptr++;
            while (*ptr != '\0')
            {
                if (*ptr == '"')
                {
                    if (*(ptr + 1) == '"')
                    {
                        if (out_idx < max_out)
                            out[out_idx++] = '"';
                        ptr += 2;
                    }
                    else
                    {
                        ptr++;
                        break;
                    }
                }
                else
                {
                    if (out_idx < max_out)
                        out[out_idx++] = *ptr;
                    ptr++;
                }
            }
            while (*ptr != '\0' && *ptr != ',')
                ptr++;
            if (*ptr == ',')
                ptr++;
        }
        else
        {
            while (*ptr != '\0' && *ptr != ',')
            {
                if (out_idx < max_out)
                    out[out_idx++] = *ptr;
                ptr++;
            }
            if (*ptr == ',')
                ptr++;
        }
        out[out_idx] = '\0';
        field_count++;
    }
    return field_count;
}

static void safe_string_copy(char *dest, const char *src, size_t dest_size)
{
    if (dest_size == 0)
        return;
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

void file_load_components(struct component_manager *mgr_p)
{
    if (!mgr_p)
        return;
    mgr_p->count = 0;
    mgr_p->next_id = 1;

    FILE *f = fopen(COMPONENT_FILE, "r");
    if (!f)
    {
        printf("Warning: Cannot open file %s (file may not exist yet).\n", COMPONENT_FILE);
        return;
    }

    char buf[LINE_BUFFER_SIZE];
    unsigned int line_num = 0;

    while (fgets(buf, sizeof(buf), f))
    {
        line_num++;
        clean_line_buffer(buf);

        if (buf[0] == '\0' || strncmp(buf, "sep=", 4) == 0 || strncmp(buf, "id,", 3) == 0)
            continue;

        if (mgr_p->count >= MAXIMUM_COMPONENTS)
        {
            printf("Warning: Reached maximum component capacity (%u).\n", MAXIMUM_COMPONENTS);
            break;
        }

        char fields[7][128];
        int field_count = parse_csv_line(buf, fields, 7);

        if (field_count == 7)
        {
            struct pc_component *curr_p = &mgr_p->components[mgr_p->count];
            memset(curr_p, 0, sizeof(struct pc_component));

            uint32_t parsed_id = (uint32_t)strtoul(fields[0], NULL, 10);

            /* Automatically resolve duplicate IDs (like the double ID 25) or 0 ID */
            if (parsed_id == 0 || component_find_by_id(mgr_p, parsed_id) != NULL)
            {
                curr_p->id = mgr_p->next_id++;
            }
            else
            {
                curr_p->id = parsed_id;
                if (parsed_id >= mgr_p->next_id)
                {
                    mgr_p->next_id = parsed_id + 1;
                }
            }

            curr_p->category = parse_category_str(fields[1]);
            safe_string_copy(curr_p->name, fields[2], sizeof(curr_p->name));
            safe_string_copy(curr_p->socket, fields[3], sizeof(curr_p->socket));
            curr_p->ddr = (uint32_t)strtoul(fields[4], NULL, 10);

            /* Sanitize corrupted integer underflows from CSV (e.g. 4294967229 -> 0) */
            unsigned long w_val = strtoul(fields[5], NULL, 10);
            curr_p->warranty_period = (w_val > 1200) ? 0 : (uint32_t)w_val;

            unsigned long s_val = strtoul(fields[6], NULL, 10);
            curr_p->stock = (s_val > 1000000) ? 0 : (uint32_t)s_val;

            mgr_p->count++;
        }
        else
        {
            printf("Warning: Skipping malformed line %u (expected 7 fields, found %d): %s\n",
                   line_num, field_count, buf);
        }
    }

    fclose(f);
    printf("Successfully loaded %u components! (Next Auto ID: %u)\n", mgr_p->count, mgr_p->next_id);
}

void file_save_components(struct component_manager *mgr_p)
{
    if (!mgr_p)
        return;

    FILE *f = fopen(COMPONENT_FILE, "w");
    if (!f)
    {
        printf("Error: Cannot write to file %s\n", COMPONENT_FILE);
        return;
    }

    fprintf(f, "sep=,\nid,category,name,socket,ddr,warrantyPeriod,stock\n");

    for (uint32_t i = 0; i < mgr_p->count; i++)
    {
        struct pc_component *c_p = &mgr_p->components[i];
        const char *cat_name = "CASE";
        if (c_p->category >= CPU && c_p->category <= CASE)
        {
            cat_name = category_names[c_p->category];
        }

        fprintf(f, "%u,%s,\"%s\",\"%s\",%u,%u,%u\n",
                c_p->id,
                cat_name,
                c_p->name,
                c_p->socket,
                c_p->ddr,
                c_p->warranty_period,
                c_p->stock);
    }

    fclose(f);
    printf("Successfully saved %u components!\n", mgr_p->count);
}