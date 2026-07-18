#include <stdlib.h>
#include <stdio.h>
#include "pc_sort.h"

static void universal_merge(struct pc_component main_arr[], int left, int mid, int right,
							int (*compare)(const struct pc_component *, const struct pc_component *))
{
	int first_half = mid - left + 1;
	int second_half = right - mid;

	/*
	 * Cấp phát heap (malloc) thay vì Variable Length Arrays (VLAs) trên stack.
	 * Tránh vi phạm Linux kernel style (-Wvla) và chống lỗi tràn stack (Stack Overflow).
	 */
	struct pc_component *L = (struct pc_component *)malloc((size_t)first_half * sizeof(struct pc_component));
	struct pc_component *R = (struct pc_component *)malloc((size_t)second_half * sizeof(struct pc_component));

	if (!L || !R)
	{
		printf("Error: Memory allocation failed during sorting.\n");
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
		/*
		 * Kiểm tra <= 0 để tương thích với chuẩn C standard/Linux kernel comparator
		 * (trả về <0 khi a < b, 0 khi a == b, >0 khi a > b).
		 */
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

/* Đã loại bỏ từ khóa static để cho phép các module khác gọi được khi liên kết (linking) */
void universal_merge_sort(struct pc_component arr[], int left, int right,
						  int (*compare)(const struct pc_component *, const struct pc_component *))
{
	if (!arr || !compare)
		return;

	if (left < right)
	{
		int mid = left + (right - left) / 2;
		universal_merge_sort(arr, left, mid, compare);
		universal_merge_sort(arr, mid + 1, right, compare);
		universal_merge(arr, left, mid, right, compare);
	}
}