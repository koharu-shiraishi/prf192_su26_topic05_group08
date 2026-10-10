#ifndef PC_FILE_H
#define PC_FILE_H

#include <stddef.h>

#include "pc_component_data.h"

#define PC_COMPONENT_DEFAULT_FILE "data/pc_component_csv.csv"

enum pc_file_status
{
    PC_FILE_OK = 0,
    PC_FILE_PARTIAL = 1,
    PC_FILE_ERR_ARGUMENT = -1,
    PC_FILE_ERR_OPEN = -2,
    PC_FILE_ERR_READ = -3,
    PC_FILE_ERR_FORMAT = -4,
    PC_FILE_ERR_MEMORY = -5,
    PC_FILE_ERR_WRITE = -6,
    PC_FILE_ERR_REPLACE = -7,
    PC_FILE_ERR_INVALID_DATA = -8
};

struct pc_file_report
{
    size_t records_seen;
    size_t loaded;
    size_t skipped;
    size_t reassigned_ids;
};

/* Loading is transactional: mgr_p is unchanged when a fatal error occurs. */
int file_load_components(struct component_manager *mgr_p, const char *path,
                         struct pc_file_report *report_p);
int file_save_components(const struct component_manager *mgr_p,
                         const char *path);
const char *pc_file_status_string(int status);

#endif /* PC_FILE_H */
