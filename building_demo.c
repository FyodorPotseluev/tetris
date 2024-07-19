/* building_demo.c */

#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

enum consts {
    delay               = 333333,   /* 1/3 of a second */
    field_height        = 20,
    field_width         = 10,
    piece_len           = 4,
    initial_piece_shift = 4
};

typedef enum piece_action { hide, print } piece_action;

typedef struct tag_figure {
    int form[piece_len][piece_len];
    int y_decline, x_shift;
} figure;

void arr_copy(int (*dst)[piece_len], int (*src)[piece_len])
{
    int x, y;
    for (y=0; y < piece_len; y++) {
        for (x=0; x < piece_len; x++)
            dst[y][x] = src[y][x];
    }
}

void print_field(int (*field)[field_width], int init_x, int init_y)
{
    int y, x;
    for (y=0; y < field_height; y++) {
        move(init_y, init_x);
        for (x=0; x < field_width; x++) {
            if (field[y][x] == 0)
                addstr("0 ");
            else
                addstr("1 ");
        }
        init_y++;
    }
    curs_set(0);
    refresh();
}

void piece_(piece_action action, figure *piece, int init_x, int init_y)
{
    int x, y;
    for (y=0; y < piece_len; y++) {
        for (x=0; x < piece_len; x++) {
            if (piece->form[y][x] == 1) {
                move(
                    init_y + piece->y_decline + y,
                    init_x + piece->x_shift*2 + x*2
                );
                switch (action) {
                    case hide:
                        addstr("0 ");
                        break;
                    case print:
                        addstr("1 ");
                }
            }
        }
    }
}

void truncate_piece(figure *piece)
{
    int x, y;
    /* checking if the piece's upmost row is empty */
    for (;;) {
        for (y=0; y < piece_len; y++) {
            for (x=0; x < piece_len; x++) {
                if (piece->form[y][x] == 1)
                    return;
            }
            /* if so - correcting the initial piece's 'y' coordinate */
            piece->y_decline--;
        }
    }
}

void piece_spawn(figure *piece, int init_x, int init_y)
{
    truncate_piece(piece);
    piece_(print, piece, init_x, init_y);
    curs_set(0);
    refresh();
}

void field_absorbes_piece(int (*field)[field_width], figure *piece)
{
    int x, y;
    /* `piece` pixels become `field` pixels */
    for (y=0; y < piece_len; y++) {
        for (x=0; x < piece_len; x++) {
            if (piece->form[y][x] == 1)
                field[y + piece->y_decline][x + piece->x_shift] = 1;
        }
    }
}

bool piece_has_fallen(int (*field)[field_width], figure *piece)
{
    int x, y;
    /* fall_checks: looking for every lowest pixel in each column */
    for (x=0; x < piece_len; x++) {
        for (y=piece_len-1; y >= 0; y--) {
            if (piece->form[y][x] == 1) {
                /* checking if the lower corresponded `field` pixel
                is occupied */
                if (field[y + piece->y_decline + 1][x + piece->x_shift] ==1)
                    return true;
                /* checking if the `field` has ended */
                if (y + piece->y_decline + 1 == field_height)
                    return true;
            }
        }
    }
    return false;
}

void piece_falls(
    int (*field)[field_width], figure *piece, int init_x, int init_y
)
{
    for (;;) {
        usleep(delay);
        if (piece_has_fallen(field, piece)) {
            field_absorbes_piece(field, piece);
            piece->y_decline = 0;
            break;
        }
        piece_(hide, piece, init_x, init_y);
        piece->y_decline++;
        piece_(print, piece, init_x, init_y);
        curs_set(0);
        refresh();
    }
}

int main()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, 1);

    int field[field_height][field_width] = {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    };
    figure piece;
    int arr[piece_len][piece_len] = {
        { 0, 0, 0, 0 },
        { 0, 1, 1, 0 },
        { 0, 1, 1, 0 },
        { 0, 0, 0, 0 }
    };
    arr_copy(piece.form, arr);
    piece.y_decline = 0;
    piece.x_shift = initial_piece_shift;
    int row, col;
    int init_x, init_y;

    getmaxyx(stdscr, row, col);
    init_x = (col - field_width*2) / 2 - 1;
    init_y = row - field_height - 1;
    print_field(field, init_x, init_y);
    for (;;) {
        piece_spawn(&piece, init_x, init_y);
        piece_falls(field, &piece, init_x, init_y);
        print_field(field, init_x, init_y);
    }

    endwin();
    return 0;
}
