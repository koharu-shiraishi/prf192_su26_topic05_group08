#include <string.h>
#include <ctype.h>
#include "pc_search.h"

/*
 * Hàm tìm kiếm chuỗi con không phân biệt hoa/thường (Case-insensitive substring check).
 * Tương thích hoàn toàn trên cả Windows (msvcrt) và Linux (glibc).
 */
static const char *str_istr(const char *haystack, const char *needle)
{
    if (!haystack || !needle)
        return NULL;
    if (*needle == '\0')
        return haystack;

    for (const char *h = haystack; *h != '\0'; h++)
    {
        const char *h_sub = h;
        const char *n_sub = needle;

        while (*h_sub != '\0' && *n_sub != '\0' &&
               tolower((unsigned char)*h_sub) == tolower((unsigned char)*n_sub))
        {
            h_sub++;
            n_sub++;
        }

        if (*n_sub == '\0')
            return h; /* Tìm thấy chuỗi con khớp */
    }

    return NULL;
}

void pc_linear_search_by_name(const struct pc_component arr[], unsigned int count,
                              const char *keyword,
                              void (*display_func)(const struct pc_component *))
{
    if (!arr || count == 0 || !keyword || !display_func)
        return;

    int found = 0;
    for (unsigned int i = 0; i < count; i++)
    {
        /*
         * Sử dụng tìm kiếm không phân biệt hoa thường để người dùng gõ
         * "rtx", "Intel", hay "DDR5" đều tìm được chính xác.
         */
        if (str_istr(arr[i].name, keyword) != NULL)
        {
            display_func(&arr[i]); /* Bắn dữ liệu về cho UI in ra */
            found = 1;
        }
    }

    if (!found)
    {
        /* Có thể xử lý hoặc callback báo không tìm thấy nếu cần */
    }
}