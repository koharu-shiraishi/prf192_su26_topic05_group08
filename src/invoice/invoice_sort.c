#include <stdlib.h>
#include <stdio.h>
#include "invoice_sort.h"

static void invoice_universal_merge(struct invoice main_arr[], int left, int mid, int right,
                                    int (*compare)(const struct invoice *, const struct invoice *))
{
    int first_half = mid - left + 1;
    int second_half = right - mid;

    /* Heap allocation to avoid VLAs and stack overflows, matching your component implementation */
    struct invoice *L = (struct invoice *)malloc((size_t)first_half * sizeof(struct invoice));
    struct invoice *R = (struct invoice *)malloc((size_t)second_half * sizeof(struct invoice));

    if (!L || !R)
    {
        printf("Error: Memory allocation failed during invoice sorting.\n");
        free(L);
        free(R);
        return;
    }

    int i, j, k;
    for (i = 0; i < first_half; i++)
        L[i] = main_arr[left + i];
    for (j = 0; j < second_half; j++)
        R[j] = main_arr[mid + 1 + j];

    i = 0;
    j = 0;
    k = left;
    while (i < first_half && j < second_half)
    {
        if (compare(&L[i], &R[j]) <= 0)
            main_arr[k++] = L[i++];
        else
            main_arr[k++] = R[j++];
    }

    while (i < first_half)
        main_arr[k++] = L[i++];
    while (j < second_half)
        main_arr[k++] = R[j++];

    free(L);
    free(R);
}

void invoice_universal_merge_sort(struct invoice arr[], int left, int right,
                                  int (*compare)(const struct invoice *, const struct invoice *))
{
    if (!arr || !compare)
        return;

    if (left < right)
    {
        int mid = left + (right - left) / 2;
        invoice_universal_merge_sort(arr, left, mid, compare);
        invoice_universal_merge_sort(arr, mid + 1, right, compare);
        invoice_universal_merge(arr, left, mid, right, compare);
    }
}