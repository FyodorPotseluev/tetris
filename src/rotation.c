/* rotation.c */

#include "rotation.h"
#include "constants.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (*matrix_callback)(void*, void*, int, int, int);

int reversion(int num, int arr_len)
{
    int i, count;
    for (i=arr_len-1, count=0; i >= 0; i--, count++) {
        if (count == num)
            return i;
    }
    fprintf(stderr, "`reversion` function failed\n");
    exit(1);
}

void init_callback(void *dst, void *src, int len, int x, int y)
{
    (void)dst;
    bool (*src_arr)[len] = src;
    src_arr[y][x] = 0;
}

void transpose_callback(void *dst, void *src, int len, int x, int y)
{
    bool (*dst_arr)[len] = dst;
    bool (*src_arr)[len] = src;
    dst_arr[x][y] = src_arr[y][x];
}

void reverse_rows_callback(void *dst, void *src, int len, int x, int y)
{
    bool (*dst_arr)[len] = dst;
    bool (*src_arr)[len] = src;
    dst_arr[y][reversion(x, len)] = src_arr[y][x];
}

void copy_callback(void *dst, void *src, int len, int x, int y)
{
    bool (*dst_arr)[len] = dst;
    bool (*src_arr)[len] = src;
    dst_arr[y][x] = src_arr[y][x];
}

void matrix_(
    matrix_callback callback, void *dst, void *src, int len
)
{
    int x, y;
    for (y=0; y < len; y++) {
        for (x=0; x < len; x++)
            callback(dst, src, len, x, y);
    }
}

void rotate(void *piece, int len)
{
    if ((len != small_piece_size) && (len != big_piece_size)) {
        fprintf(
            stderr, "`rotate` function failed: invalid `len` value: %d\n", len
        );
        exit(1);
    }
    bool (*arr)[len] = piece;
    bool transposed_piece[len][len];
    bool row_reverted_piece[len][len];
    matrix_(init_callback, NULL, transposed_piece, len);
    matrix_(init_callback, NULL, row_reverted_piece, len);
    matrix_(transpose_callback, transposed_piece, arr, len);
    matrix_(reverse_rows_callback, row_reverted_piece, transposed_piece, len);
    matrix_(copy_callback, arr, row_reverted_piece, len);
}
