#include <stddef.h>
#include "invoice_search.h"

void invoice_linear_search(const struct invoice arr[], unsigned int count,
                           const void *search_key,
                           int (*match_func)(const struct invoice *, const void *),
                           void (*display_func)(const struct invoice *))
{
    if (!arr || count == 0 || !search_key || !match_func || !display_func)
        return;

    int found = 0;
    for (unsigned int i = 0; i < count; i++)
    {
        if (match_func(&arr[i], search_key))
        {
            display_func(&arr[i]); /* Push to UI */
            found = 1;
        }
    }

    if (!found)
    {
        /* Handle not found quietly, or trigger a UI callback if needed */
    }
}