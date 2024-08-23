/* building_demo.c */

#include "rotation_demo.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

enum consts {
    init_delay          = 666,          /* 2/3 of a second */
    field_height        = 20,
    field_width         = 10,
    num_of_pieces       = 7,
    small_piece_size    = 3,
    big_piece_size      = 4,
    initial_piece_shift = 4
};

typedef enum tag_piece_action {
    hide_piece, print_piece, hide_ghost, print_ghost
} piece_action;
typedef enum tag_crossing_action { prevention, signal } crossing_action;
typedef enum tag_move_direction { left = 1, right } move_direction;
typedef enum tag_position {
    horizontal_1, vertical_1, horizontal_2, vertical_2, orientation_count
} position;

typedef struct tag_figure {
    unsigned char size;
    union tag_form {
        bool small[small_piece_size][small_piece_size];
        bool big[big_piece_size][big_piece_size];
    } form;
    signed char x_shift, y_decline, ghost_decline;
    bool i_form;
    position orientation;
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

/*
void arr_copy(bool (*dst)[piece_size], bool (*src)[piece_size])
{
    int x, y;
    for (y=0; y < piece_size; y++) {
        for (x=0; x < piece_size; x++)
            dst[y][x] = src[y][x];
    }
} */

void initialize_init_x(int *init_x, bool *initialized)
{
    if (!*initialized) {
        int row, col;
        (void)row;
        getmaxyx(stdscr, row, col);
        *init_x = (col - field_width*2) / 2 - 1;
        *initialized = true;
    }
}

void initialize_init_y(int *init_y, bool *initialized)
{
    if (!*initialized) {
        int row, col;
        (void)col;
        getmaxyx(stdscr, row, col);
        *init_y = row - field_height - 1;
        *initialized = true;
    }
}

void print_field(bool (*field)[field_width])
{
    int y, x;
    static int init_x, init_y;
    static bool initialized_x, initialized_y;
    initialize_init_x(&init_x, &initialized_x);
    initialize_init_y(&init_y, &initialized_y);
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
        case hide_piece:
        case hide_ghost:
            addstr("0 ");
            break;
        case print_piece:
            addstr("1 ");
            break;
        case print_ghost:
            addstr(". ");
    }
}

int ghost_y(int ghost_decline, int y)
{
    static int init_y;
    static bool initialized;
    initialize_init_y(&init_y, &initialized);
    return init_y + ghost_decline + y;
}

int curr_y(figure *piece, int y)
{
    static int init_y;
    static bool initialized;
    initialize_init_y(&init_y, &initialized);
    return init_y + piece->y_decline + y;
}

int curr_x(figure *piece, int x)
{
    static int init_x;
    static bool initialized;
    initialize_init_x(&init_x, &initialized);
    return init_x + piece->x_shift*2 + x*2;
}

void piece_(piece_action action, figure *piece)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*arr)[piece->size] = piece->form.small;
    int x, y;
    for (y=0; y < piece->size; y++) {
        for (x=0; x < piece->size; x++) {
            if (arr[y][x] == 1) {
                if (action == hide_ghost)
                    move(ghost_y(piece->ghost_decline, y), curr_x(piece, x));
                else
                    move(curr_y(piece, y), curr_x(piece, x));
                take_(action);
            }
        }
    }
}

void truncate_piece(figure *piece)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*arr)[piece->size] = piece->form.small;
    int x, y;
    /* checking if the piece's upmost row is empty */
    while (true) {
        for (y=0; y < piece->size; y++) {
            for (x=0; x < piece->size; x++) {
                if (arr[y][x] == 1)
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
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*arr)[piece->size] = piece->form.small;
    int x, y;
    /* fall_checks: looking for every lowest pixel in each column */
    for (y=piece->size-1; y >= 0; y--) {
        for (x=0; x < piece->size; x++) {
            if (arr[y][x] == 1) {
                if (lower_field_pixel_is_occupied(field, piece, x, y))
                    return true;
                if (field_has_ended(piece, y))
                    return true;
            }
        }
    }
    return false;
}

void cast_ghost(
    bool (*field)[field_width], figure piece, signed char *ghost_decline
)
{
    while (!piece_has_fallen(field, &piece))
        piece.y_decline++;
    piece_(print_ghost, &piece);
    *ghost_decline = piece.y_decline;
}

void piece_spawn(bool (*field)[field_width], figure *piece)
{
    truncate_piece(piece);
    cast_ghost(field, *piece, &piece->ghost_decline);
    piece_(print_piece, piece);
    curs_set(0);
    refresh();
}

void field_absorbes_piece(bool (*field)[field_width], figure *piece)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*arr)[piece->size] = piece->form.small;
    int x, y;
    /* `piece` pixels become `field` pixels */
    for (y=0; y < piece->size; y++) {
        for (x=0; x < piece->size; x++) {
            if (arr[y][x] == 1)
                field[y + piece->y_decline][x + piece->x_shift] = 1;
        }
    }
}

bool out_of_right_boundary(figure *piece)
{
    return (piece->x_shift > field_width - piece->size) ? true : false;
}

bool out_of_left_boundary(figure *piece)
{
    return (piece->x_shift < 0) ? true : false;
}

bool cell_occupied_by_(bool (*field)[field_width], int x, int y, figure *piece)
{
    return (field[y+piece->y_decline][x+piece->x_shift] == 1) ? true : false;
}

bool out_of_bottom_field_boundary(figure *piece)
{
    return (piece->y_decline > field_height - piece->size) ? true : false;
}

bool out_of_top_field_boundary(figure *piece)
{
    return (piece->y_decline < 0) ? true : false;
}

bool horizontal_orientation(figure *piece)
{
    if (
        (piece->orientation == horizontal_1) ||
        (piece->orientation == horizontal_2)
    )
        return true;
    else
        return false;
}

bool vertical_orientation(figure *piece)
{
    if (
        (piece->orientation == vertical_1) ||
        (piece->orientation == vertical_2)
    )
        return true;
    else
        return false;
}

bool special_i_piece_bottom_top_case(figure *piece)
{
    return ((piece->i_form) && vertical_orientation(piece)) ? true : false;
}

bool special_i_piece_side_case(figure *piece)
{
    return ((piece->i_form) && horizontal_orientation(piece)) ? true : false;
}

bool set_x_cycle_bound_cond(
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
        *start_x = piece->size-1;
        *end_x   = field_width - piece->x_shift - 1;
        *incr_x  = -1;
        return true;
    }
    else
        return false;
}

bool side_boundaries_crossing_(crossing_action action, figure *piece, int *dx)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*arr)[piece->size] = piece->form.small;
    int x, y;
    int start_x, end_x, incr_x;
    if (!set_x_cycle_bound_cond(piece, &start_x, &end_x, &incr_x))
        return false;
    for (y=0; y < piece->size; y++) {
        for (x=start_x; x != end_x; x+=incr_x) {
            if (arr[y][x] == 1) {
                if (action == signal)
                    return true;
                *dx += incr_x;
                if (special_i_piece_side_case(piece))
                    continue;
                else
                    return true;
            }
        }
    }
    return false;
}

bool set_y_cycle_cond(figure *piece, int *start_y, int *end_y, int *incr_y)
{
    if (out_of_top_field_boundary(piece)) {
        /* loop forward */
        *start_y = 0;
        *end_y   = -piece->y_decline;
        *incr_y  = 1;
        return true;
    }
    else
    if (out_of_bottom_field_boundary(piece)) {
        /* loop back */
        *start_y = piece->size-1;
        *end_y   = field_height - piece->y_decline - 1;
        *incr_y  = -1;
        return true;
    }
    else
        return false;
}

bool bottom_top_boundaries_crossing_(
    crossing_action action, figure *piece, int *dy
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*arr)[piece->size] = piece->form.small;
    int x, y;
    int start_y, end_y, incr_y;
    if (!set_y_cycle_cond(piece, &start_y, &end_y, &incr_y))
        return false;
    for (y=start_y; y != end_y; y+=incr_y) {
        for (x=0; x < piece->size; x++) {
            if (arr[y][x] == 1) {
                if (action == signal)
                    return true;
                *dy += incr_y;
                if (special_i_piece_bottom_top_case(piece))
                    continue;
                else
                    return true;
            }
        }
    }
    return false;
}

bool set_x_cycle_pixel_cond(
    move_direction direction, figure *piece,
    int *start_x, int *end_x, int *incr_x
)
{
    switch (direction) {
        case left:
            /* loop forward */
            *start_x = 0;
            *end_x   = piece->size;
            *incr_x  = 1;
            return true;
        case right:
            /* loop back */
            *start_x = piece->size - 1;
            *end_x   = -1;
            *incr_x  = -1;
            return true;
    }
    return true;
}

void side_pixels_crossing_prevention(
    move_direction direction, bool (*field)[field_width], figure *piece, int *dx
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*arr)[piece->size] = piece->form.small;
    int x, y;
    int start_x, end_x, incr_x;
    set_x_cycle_pixel_cond(direction, piece, &start_x, &end_x, &incr_x);
    for (y=0; y < piece->size; y++) {
        for (x=start_x; x != end_x; x+=incr_x) {
            if ((arr[y][x] == 1) && (cell_occupied_by_(field, x, y, piece))) {
                *dx += incr_x;
                return;
            }
        }
    }
}

void move_(
    move_direction direction, bool (*field)[field_width], figure *piece
)
{
    piece_(hide_ghost, piece);
    piece_(hide_piece, piece);
    switch (direction) {
        case left:
            piece->x_shift--;
            break;
        case right:
            piece->x_shift++;
    }
    int dx = 0;
    side_boundaries_crossing_(prevention, piece, &dx);
    side_pixels_crossing_prevention(direction, field, piece, &dx);
    piece->x_shift += dx;
    cast_ghost(field, *piece, &piece->ghost_decline);
    piece_(print_piece, piece);
}

void piece_fall_step(figure *piece)
{
    piece_(hide_piece, piece);
    piece->y_decline++;
    piece_(print_piece, piece);
    curs_set(0);
    refresh();
}

void next_orientation(figure *piece)
{
    /* traversing a list of enumerated values cyclically (after the last
    value, we get the 1st value again) */
    piece->orientation = (piece->orientation + 1) % orientation_count;
}

void handle_i_piece(figure *piece)
{
    /* after the rotation of the I piece, we need that both of it vertical
    incarnations were at the same column */
    if (piece->i_form) {
        if (piece->orientation == vertical_1) piece->x_shift--;
        else
        if (piece->orientation == vertical_2) piece->x_shift++;
        next_orientation(piece);
    }
}

void handle_rotation(bool (*field)[field_width], figure *piece)
{
    piece_(hide_ghost, piece);
    piece_(hide_piece, piece);
    handle_i_piece(piece);
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    rotate(piece->form.small, piece->size);
    int dx = 0, dy = 0;
    side_boundaries_crossing_(prevention, piece, &dx);
    bottom_top_boundaries_crossing_(prevention, piece, &dy);
    piece->x_shift += dx;
    piece->y_decline += dy;
    /* handle_rotation_conflicts(); */
    cast_ghost(field, *piece, &piece->ghost_decline);
    piece_(print_piece, piece);
}

void process_key(
    int key_pressed, bool (*field)[field_width], figure *piece, bool *hard_drop
)
{
    switch (key_pressed) {
        case KEY_LEFT:
            move_(left, field, piece);
            break;
        case KEY_RIGHT:
            move_(right, field, piece);
            break;
        /* rotate */
        case KEY_UP:
            handle_rotation(field, piece);
            break;
        /* hard drop */
        case ' ':
            while (!piece_has_fallen(field, piece))
                piece_fall_step(piece);
            *hard_drop = true;
    }
}

void process_input(bool (*field)[field_width], figure *piece)
{
    struct timeval tv1, tv2;
    struct timezone tz;
    int key_pressed, delay = init_delay;
    for (;;) {
        bool hard_drop = false;
        timeout(delay);
        time_start(&tv1, &tz);
        key_pressed = getch();
        if ((key_pressed == ERR) || (key_pressed == KEY_DOWN))
            break;
        process_key(key_pressed, field, piece, &hard_drop);
        if (hard_drop)
            break;
        /* new delay value calculation */
        delay -= time_stop(&tv1, &tv2, &tz);
        if (delay < 0)
            break;
        else
            continue;
    }
}

void piece_falls(bool (*field)[field_width], figure *piece)
{
    for (;;) {
        process_input(field, piece);
        if (piece_has_fallen(field, piece)) {
            field_absorbes_piece(field, piece);
            break;
        }
        piece_fall_step(piece);
    }
}

void init_set_of_pieces(figure *set_of_pieces)
{
    int i = 0;
    figure I_piece = {
        .size = big_piece_size,
        .form.big = {
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 1, 1, 1, 1 },
            { 0, 0, 0, 0 }
        },
        .x_shift = initial_piece_shift,
        .y_decline = 0, .ghost_decline = 0,
        .i_form = true,
        .orientation = horizontal_1
    };
    memcpy(&set_of_pieces[i], &I_piece, sizeof(figure));
    i++;
    figure O_piece = {
        .size = big_piece_size,
        .form.big = {
            { 0, 0, 0, 0 },
            { 0, 1, 1, 0 },
            { 0, 1, 1, 0 },
            { 0, 0, 0, 0 }
        },
        .x_shift = initial_piece_shift,
        .y_decline = 0, .ghost_decline = 0,
        .i_form = false
    };
    memcpy(&set_of_pieces[i], &O_piece, sizeof(figure));
    i++;
    figure T_piece = {
        .size = small_piece_size,
        .form.small = {
            { 0, 0, 0 },
            { 1, 1, 1 },
            { 0, 1, 0 }
        },
        .x_shift = initial_piece_shift,
        .y_decline = 0, .ghost_decline = 0,
        .i_form = false
    };
    memcpy(&set_of_pieces[i], &T_piece, sizeof(figure));
    i++;
    figure S_piece = {
        .size = small_piece_size,
        .form.small = {
            { 0, 0, 0 },
            { 0, 1, 1 },
            { 1, 1, 0 }
        },
        .x_shift = initial_piece_shift,
        .y_decline = 0, .ghost_decline = 0,
        .i_form = false
    };
    memcpy(&set_of_pieces[i], &S_piece, sizeof(figure));
    i++;
    figure Z_piece = {
        .size = small_piece_size,
        .form.small = {
            { 0, 0, 0 },
            { 1, 1, 0 },
            { 0, 1, 1 }
        },
        .x_shift = initial_piece_shift,
        .y_decline = 0, .ghost_decline = 0,
        .i_form = false
    };
    memcpy(&set_of_pieces[i], &Z_piece, sizeof(figure));
    i++;
    figure J_piece = {
        .size = small_piece_size,
        .form.small = {
            { 0, 0, 0 },
            { 1, 1, 1 },
            { 0, 0, 1 }
        },
        .x_shift = initial_piece_shift,
        .y_decline = 0, .ghost_decline = 0,
        .i_form = false
    };
    memcpy(&set_of_pieces[i], &J_piece, sizeof(figure));
    i++;
    figure L_piece = {
        .size = small_piece_size,
        .form.small = {
            { 0, 0, 0 },
            { 1, 1, 1 },
            { 1, 0, 0 }
        },
        .x_shift = initial_piece_shift,
        .y_decline = 0, .ghost_decline = 0,
        .i_form = false
    };
    memcpy(&set_of_pieces[i], &L_piece, sizeof(figure));
}

void init_field(bool (*field)[field_width])
{
    int x, y;
    for (y=0; y < field_height; y++) {
        for (x=0; x < field_width; x++) {
            field[y][x] = 0;
        }
    }
}

int main()
{
    /* ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, 1);

    /* debug */
    /* mvprintw(0, 0, "Sizeof figure is %ld bytes.", sizeof(figure)); */

    /* variables */
    bool field[field_height][field_width];
    init_field(field);
    figure set_of_pieces[num_of_pieces];
    init_set_of_pieces(set_of_pieces);
    figure piece;
    int curr_piece;

    /* MAIN */
    print_field(field);
    for (curr_piece=0;;curr_piece++) {
        piece = set_of_pieces[curr_piece];
        piece_spawn(field, &piece);
        piece_falls(field, &piece);
    }

    /* ncurses */
    endwin();
    return 0;
}
