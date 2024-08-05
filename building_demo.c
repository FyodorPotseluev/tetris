/* building_demo.c */

#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

enum consts {
    init_delay          = 333,          /* 1/3 of a second */
    field_height        = 20,
    field_width         = 10,
    piece_size          = 4,
    initial_piece_shift = 4
};

typedef enum tag_piece_action {
    hide, print, hide_ghost, print_ghost
} piece_action;
typedef enum tag_move_direction { left = 1, right } move_direction;

typedef struct tag_figure {
    bool form[piece_size][piece_size];
    int init_x, init_y;
    int x_shift, y_decline;
} figure;

void time_start(struct timeval *tv1, struct timezone *tz)
{
    gettimeofday(tv1, tz);
}

int time_stop(struct timeval *tv1, struct timeval *tv2, struct timezone *tz)
{
    struct timeval dtv;
    gettimeofday(tv2, tz);
    dtv.tv_sec = tv2->tv_sec - tv1->tv_sec;
    dtv.tv_usec = tv2->tv_usec - tv1->tv_usec;
    /* in case time difference was less than a second: */
    if (dtv.tv_usec < 0) {
        dtv.tv_sec--;
        dtv.tv_usec += 1000000;
    }
    return dtv.tv_sec*1000 + dtv.tv_usec/1000;
}

void arr_copy(bool (*dst)[piece_size], bool (*src)[piece_size])
{
    int x, y;
    for (y=0; y < piece_size; y++) {
        for (x=0; x < piece_size; x++)
            dst[y][x] = src[y][x];
    }
}

void print_field(bool (*field)[field_width], int init_x, int init_y)
{
    int y, x;
    int curr_y = init_y;
    for (y=0; y < field_height; y++) {
        move(curr_y, init_x);
        for (x=0; x < field_width; x++) {
            if (field[y][x] == 0)
                addstr("0 ");
            else
                addstr("1 ");
        }
        curr_y++;
    }
    curs_set(0);
    refresh();
}

void take_(piece_action action)
{
    switch (action) {
        case hide:
        case hide_ghost:
            addstr("0 ");
            break;
        case print:
            addstr("1 ");
            break;
        case print_ghost:
            addstr(". ");
    }
}

int ghost_y(figure *piece, int ghost_decline, int y)
{
    return piece->init_y + ghost_decline + y;
}

int curr_y(figure *piece, int y)
{
    return piece->init_y + piece->y_decline + y;
}

int curr_x(figure *piece, int x)
{
    return piece->init_x + piece->x_shift*2 + x*2;
}

void piece_(piece_action action, figure *piece, int *ghost_decline)
{
    int x, y;
    for (y=0; y < piece_size; y++) {
        for (x=0; x < piece_size; x++) {
            if (piece->form[y][x] == 1) {
                if (action == hide_ghost)
                    move(ghost_y(piece, *ghost_decline, y), curr_x(piece, x));
                else
                    move(curr_y(piece, y), curr_x(piece, x));
                take_(action);
            }
        }
    }
}

void truncate_piece(figure *piece)
{
    int x, y;
    /* checking if the piece's upmost row is empty */
    while (true) {
        for (y=0; y < piece_size; y++) {
            for (x=0; x < piece_size; x++) {
                if (piece->form[y][x] == 1)
                    return;
            }
            /* if so - correcting the initial piece's 'y' coordinate */
            piece->y_decline--;
        }
    }
}

bool field_has_ended(figure *piece, int y)
{
    return (y + piece->y_decline + 1 == field_height) ? true : false;
}

bool lower_field_pixel_is_occupied(
    bool (*field)[field_width], figure *piece, int x, int y
)
{
    if (field[y + piece->y_decline + 1][x + piece->x_shift] ==1)
        return true;
    else
        return false;
}

bool piece_has_fallen(bool (*field)[field_width], figure *piece)
{
    int x, y;
    /* fall_checks: looking for every lowest pixel in each column */
    for (y=piece_size-1; y >= 0; y--) {
        for (x=0; x < piece_size; x++) {
            if (piece->form[y][x] == 1) {
                if (lower_field_pixel_is_occupied(field, piece, x, y))
                    return true;
                if (field_has_ended(piece, y))
                    return true;
            }
        }
    }
    return false;
}

void cast_ghost(bool (*field)[field_width], figure piece, int *ghost_decline)
{
    while (!piece_has_fallen(field, &piece))
        piece.y_decline++;
    piece_(print_ghost, &piece, NULL);
    *ghost_decline = piece.y_decline;
}

void piece_spawn(bool (*field)[field_width], figure *piece, int *ghost_decline)
{
    truncate_piece(piece);
    cast_ghost(field, *piece, ghost_decline);
    piece_(print, piece, NULL);
    curs_set(0);
    refresh();
}

void reset_piece_properties(figure *piece)
{
    piece->y_decline = 0;
    piece->x_shift = initial_piece_shift;
}

void field_absorbes_piece(bool (*field)[field_width], figure *piece)
{
    int x, y;
    /* `piece` pixels become `field` pixels */
    for (y=0; y < piece_size; y++) {
        for (x=0; x < piece_size; x++) {
            if (piece->form[y][x] == 1)
                field[y + piece->y_decline][x + piece->x_shift] = 1;
        }
    }
}

bool out_of_right_boundary(figure *piece)
{
    return (piece->x_shift > field_width - piece_size) ? true : false;
}

bool out_of_left_boundary(figure *piece)
{
    return (piece->x_shift < 0) ? true : false;
}

bool cell_occupied_by_(bool (*field)[field_width], int x, int y, figure *piece)
{
    return (field[y+piece->y_decline][x+piece->x_shift] == 1) ? true : false;
}

bool undo_piece_shift_condition(
    bool (*field)[field_width], int x, int y, figure *piece
)
{
    if (field) {
        if (piece->form[y][x]==1 && cell_occupied_by_(field, x, y, piece))
            return true;
        else
            return false;
    } else {
        /* condition for crossing the field boundary */
        if (piece->form[y][x]==1)
            return true;
        else
            return false;
    }
}

bool set_cond_not_to_cross_field_boundary(
    figure *piece, int *start_x, int *end_x, int *incr_x
)
{
    if (out_of_left_boundary(piece)) {
        /* loop forward */
        *start_x = 0;
        *end_x   = -piece->x_shift;
        *incr_x  = 1;
        return true;
    }
    else
    if (out_of_right_boundary(piece)) {
        /* loop back */
        *start_x = piece_size-1;
        *end_x   = field_width - piece->x_shift - 1;
        *incr_x  = -1;
        return true;
    }
    else
        return false;
}

bool set_cond_not_to_cross_occupied_side_pixel(
    move_direction direction, int *start_x, int *end_x, int *incr_x
)
{
    switch (direction) {
        case left:
            /* loop forward */
            *start_x = 0;
            *end_x   = piece_size;
            *incr_x  = 1;
            return true;
        case right:
            /* loop back */
            *start_x = piece_size - 1;
            *end_x   = -1;
            *incr_x  = -1;
            return true;
    }
    return true;
}

bool set_init_condition_for_x_cycle(
    move_direction direction, figure *piece,
    int *start_x, int *end_x, int *incr_x
)
{
    if (direction)
        return set_cond_not_to_cross_occupied_side_pixel(
            direction, start_x, end_x, incr_x
        );
    else
        return set_cond_not_to_cross_field_boundary(
            piece, start_x, end_x, incr_x
        );
}

void prevent_crossing(
    move_direction direction, bool (*field)[field_width], figure *piece
)
{
    int x, y;
    int start_x, end_x, incr_x;
    if (!set_init_condition_for_x_cycle(
        direction, piece, &start_x, &end_x, &incr_x
    ))
    {
        return;
    }
    for (y=0; y < piece_size; y++) {
        for (x=start_x; x != end_x; x+=incr_x) {
            if (undo_piece_shift_condition(field, x, y, piece)) {
                piece->x_shift += incr_x;
                return;
            }
        }
    }
}

void move_(
    move_direction direction, figure *piece,
    bool (*field)[field_width], int *ghost_decline)
{
    switch (direction) {
        case left:
            piece_(hide_ghost, piece, ghost_decline);
            piece_(hide, piece, NULL);
            piece->x_shift--;
            break;
        case right:
            piece_(hide_ghost, piece, ghost_decline);
            piece_(hide, piece, NULL);
            piece->x_shift++;
    }
    /* prevention of crossing the field boundary */
    prevent_crossing(0, NULL, piece);
    /* prevention of crossing already occupied side pixel */
    prevent_crossing(direction, field, piece);
    cast_ghost(field, *piece, ghost_decline);
    piece_(print, piece, NULL);
}

void piece_fall_step(figure *piece)
{
    piece_(hide, piece, NULL);
    piece->y_decline++;
    piece_(print, piece, NULL);
    curs_set(0);
    refresh();
}

void process_key(
    int key_pressed, bool (*field)[field_width],
    figure *piece, int *ghost_decline
)
{
    switch (key_pressed) {
        case KEY_LEFT:
            move_(left, piece, field, ghost_decline);
            break;
        case KEY_RIGHT:
            move_(right, piece, field, ghost_decline);
            break;
        /* hard drop */
        case ' ':
            while (!piece_has_fallen(field, piece))
                piece_fall_step(piece);
    }
}

void process_input(
    bool (*field)[field_width], figure *piece, int *ghost_decline
)
{
    struct timeval tv1, tv2;
    struct timezone tz;
    int key_pressed, delay = init_delay;
    for (;;) {
        timeout(delay);
        time_start(&tv1, &tz);
        key_pressed = getch();
        if ((key_pressed == ERR) || (key_pressed == KEY_DOWN))
            break;
        process_key(key_pressed, field, piece, ghost_decline);
        /* new delay value calculation */
        delay -= time_stop(&tv1, &tv2, &tz);
        if (delay < 0)
            break;
        else
            continue;
    }
}

void piece_falls(bool (*field)[field_width], figure *piece, int *ghost_decline)
{
    for (;;) {
        process_input(field, piece, ghost_decline);
        if (piece_has_fallen(field, piece)) {
            field_absorbes_piece(field, piece);
            reset_piece_properties(piece);
            break;
        }
        piece_fall_step(piece);
    }
}

int get_init_x(int col)
{
    return (col - field_width*2) / 2 - 1;
}

int get_init_y(int row)
{
    return row - field_height - 1;
}

int main()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, 1);

    bool field[field_height][field_width] = {
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
    bool arr[piece_size][piece_size] = {
        { 0, 0, 0, 0 },
        { 0, 1, 1, 0 },
        { 0, 1, 1, 0 },
        { 0, 0, 0, 0 },
    };
    arr_copy(piece.form, arr);
    piece.y_decline = 0;
    piece.x_shift = initial_piece_shift;
    int ghost_decline, row, col;

    getmaxyx(stdscr, row, col);
    piece.init_x = get_init_x(col);
    piece.init_y = get_init_y(row);
    print_field(field, piece.init_x, piece.init_y);
    for (;;) {
        piece_spawn(field, &piece, &ghost_decline);
        piece_falls(field, &piece, &ghost_decline);
    }

    endwin();
    return 0;
}
