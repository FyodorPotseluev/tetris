/* rotation_demo.c */

#include <ncurses.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

enum consts {
    delay           = 2,
    num_of_turnes   = 4,
    size            = 3
};

typedef void (*piece_callback)(void*, int, int, int);
typedef void (*matrix_callback)(void*, void*, int, int, int);

void print_frame()
{
    int i;
    for (i=0; i < 12; i++) {
        move(8, i);
        addch('_');
    }
    for (i=0; i < 9; i++) {
        move(i, 12);
        addch('|');
    }
    curs_set(0);
    refresh();
}

void print_callback(void *piece, int len, int x, int y)
{
    int (*arr)[len] = piece;
    if (arr[y][x] == 1) {
        move(y*2, x*3);
        addstr("###");
        move(y*2 + 1, x*3);
        addstr("###");
    }
}

void hide_callback(void *piece, int len, int x, int y)
{
    (void)piece;
    (void)len;
    move(y*2, x*3);
    addstr("   ");
    move(y*2 + 1, x*3);
    addstr("   ");
}

void piece_(piece_callback callback, void *piece, int len)
{
    int x, y;
    for (x=0; x < len; x++) {
        for (y=0; y < len; y++)
            callback(piece, len, x, y);
    }
    curs_set(0);
    refresh();
}

int reversion(int num, int arr_len)
{
    int i, count;
    for (i=arr_len-1, count=0; i >= 0; i--, count++) {
        if (count == num)
            return i;
    }
    fprintf(stderr, "reversion function failed\n");
    exit(1);
}

void init_callback(void *dst, void *src, int len, int x, int y)
{
    (void)dst;
    int (*src_arr)[len] = src;
    src_arr[y][x] = 0;
}

void transpose_callback(void *dst, void *src, int len, int x, int y)
{
    int (*dst_arr)[len] = dst;
    int (*src_arr)[len] = src;
    dst_arr[x][y] = src_arr[y][x];
}

void reverse_rows_callback(void *dst, void *src, int len, int x, int y)
{
    int (*dst_arr)[len] = dst;
    int (*src_arr)[len] = src;
    dst_arr[y][reversion(x, len)] = src_arr[y][x];
}

void copy_callback(void *dst, void *src, int len, int x, int y)
{
    int (*dst_arr)[len] = dst;
    int (*src_arr)[len] = src;
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
    int (*arr)[len] = piece;
    int transposed_piece[len][len];
    int row_reverted_piece[len][len];
    matrix_(init_callback, NULL, transposed_piece, len);
    matrix_(init_callback, NULL, row_reverted_piece, len);
    matrix_(transpose_callback, transposed_piece, arr, len);
    matrix_(reverse_rows_callback, row_reverted_piece, transposed_piece, len);
    matrix_(copy_callback, arr, row_reverted_piece, len);

    /* piece_arr turned_piece = {
        { 0, 0, 0 },
        { 0, 0, 0 },
        { 0, 0, 0 }
    };
    int x, y;
    for (x=0; x < 3; x++) {
        for (y=0; y < 3; y++) {
            if (piece[y][x] == 1) {
                int new_x, new_y;
                new_x = (int)round((x-1)*cos(M_PI/2) - (y-1)*sin(M_PI/2));
                new_y = (int)round((x-1)*sin(M_PI/2) + (y-1)*cos(M_PI/2));
                turned_piece[new_y+1][new_x+1] = 1;
            }
        }
    }
    for (x=0; x < 3; x++) {
        for (y=0; y < 3; y++)
            piece[y][x] = turned_piece[y][x];
    } */
}

int main()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, 0);

    int i;
    int tetris_piece[3][3] = {
        { 0, 1, 0 },
        { 0, 1, 0 },
        { 1, 1, 1 }
    };
    /* int tetris_piece[4][4] = {
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
        { 1, 1, 1, 1 },
        { 0, 0, 0, 0 }
    }; */
    print_frame();
    piece_(print_callback, tetris_piece, size);
    sleep(delay);
    for (i=0; i < num_of_turnes; i++) {
        piece_(hide_callback, NULL, size);
        rotate(tetris_piece, size);
        piece_(print_callback, tetris_piece, size);
        sleep(delay);
    }

    endwin();
}
