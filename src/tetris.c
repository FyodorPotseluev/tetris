/* tetris.c */

#include "conflict_resolution.h"
#include "constants.h"
#include "frontend.h"
#include "rotation.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

typedef struct tag_struct_game_info {
    int level;
    int score;
    bool life_lost;
    bool game_on;
} struct_game_info;

typedef enum tag_transform_field_action {
    field_absorbes_piece, record_falling_cells, erase_falling_cells
} transform_field_action;

typedef enum tag_piece_action {
    hide_piece, print_piece, hide_ghost, print_ghost
} piece_action;

typedef enum tag_dude_action {
    hide_dude, print_dude
} dude_action;

typedef enum tag_dude_lives_action {
    print_dude_lives, hide_dude_lives
} dude_lives_action;

typedef enum tag_boundary_side {
    bottom, top, left_side, right_side
} boundary_side;

void time_start(struct timeval *tv1, struct timezone *tz)
{
    gettimeofday(tv1, tz);
}

int time_stop(
    const struct timeval *tv1, struct timeval *tv2, struct timezone *tz
)
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

int get_init_x()
{
    static int init_x = -1;
    if (init_x < 0) {
        int row, col;
        (void)row;
        getmaxyx(stdscr, row, col);
        init_x = (col - field_width * cell_width) / 2;
    }
    return init_x;
}

int get_init_y()
{
    static int init_y = -1;
    if (init_y < 0) {
        int row, col;
        (void)col;
        getmaxyx(stdscr, row, col);
        init_y = row - field_height * cell_height - 1;
    }
    return init_y;
}

void print_cell_(enum_field type, int x, int y)
{
    int i;
    for (i=0; i < cell_height; i++, y++) {
        move(y, x);
        switch (type) {
            case empty:
                addstr(EMPTY_CELL_ROW);
                break;
            case occupied:
                addstr(OCCUPIED_CELL_ROW);
                break;
            case ghost:
                addstr(GHOST_CELL_ROW);
                break;
            default:
                fprintf(stderr, "%s:%d: incorrect value", __FILE__, __LINE__);
                exit(1);
        }
    }
}

void print_bottom_top_boundary()
{
    int i;
    for (i=0; i < field_width*cell_width + 2*side_boundary_width; i++)
        addstr(BOTTOM_TOP_BOUNDARY);
}

void print_side_boundary(int x, int y)
{
    int i;
    for (i=0; i < cell_height; i++, y++) {
        move(y, x);
        addstr(SIDE_BOUNDARY);
    }
}

void print_field_boundary(
    boundary_side side, int *screen_x, const int *screen_y
)
{
    switch (side) {
        case top:
            move(get_init_y()-1, get_init_x()-1);
            print_bottom_top_boundary();
            break;
        case bottom:
            move(get_init_y()+field_height*cell_height, get_init_x()-1);
            print_bottom_top_boundary();
            break;
        case left_side:
            print_side_boundary(*screen_x, *screen_y);
            *screen_x += side_boundary_width;
            break;
        case right_side:
            print_side_boundary(*screen_x, *screen_y);
            break;
        default:
            fprintf(stderr, "%s:%d: incorrect value", __FILE__, __LINE__);
            exit(1);
    }
}

int init_screen_x()
{
    return get_init_x() - side_boundary_width;
}

void print_field(const enum_field (*field)[field_width])
{
    /* field is printed from top to bottom */
    int field_x, field_y, screen_x, screen_y;
    print_field_boundary(top, NULL, NULL);
    for (
        field_y = 0, screen_x = init_screen_x(), screen_y = get_init_y();
        field_y < field_height;
        field_y++, screen_x = init_screen_x(), screen_y += cell_height
    )
    {
        if ((field_y != dude_exit_height - 1) && (field_y != dude_exit_height))
            print_field_boundary(left_side, &screen_x, &screen_y);
        for (field_x=0; field_x < field_width; field_x++) {
            if (field[field_y][field_x] == 0)
                print_cell_(empty, screen_x, screen_y);
            else
                print_cell_(occupied, screen_x, screen_y);
            screen_x += cell_width;
            move(screen_y, screen_x);
        }
        if ((field_y != dude_exit_height - 1) && (field_y != dude_exit_height))
            print_field_boundary(right_side, &screen_x, &screen_y);
    }
    print_field_boundary(bottom, NULL, NULL);
    curs_set(0);
    refresh();
}

void take_(piece_action action, int x, int y)
{
    switch (action) {
        case hide_piece:
        case hide_ghost:
            print_cell_(empty, x, y);
            break;
        case print_piece:
            print_cell_(occupied, x, y);
            break;
        case print_ghost:
            print_cell_(ghost, x, y);
            break;
        default:
            fprintf(stderr, "%s:%d: incorrect value", __FILE__, __LINE__);
            exit(1);
    }
}

int ghost_y(const struct_piece *piece, int y)
{
    return get_init_y() + (piece->ghost_decline + y) * cell_height;
}

int curr_y(const struct_piece *piece, int y)
{
    return get_init_y() + (piece->y_decline + y) * cell_height;
}

int curr_x(const struct_piece *piece, int x)
{
    return get_init_x() + (piece->x_shift + x) * cell_width;
}

bool dude_ghost_piece_overlap(
    const struct_dude *dude, const struct_piece *piece, int x, int y
)
{
    return (
        (x + piece->x_shift == dude->x_shift) &&
        ((y + piece->ghost_decline == dude->y_decline) ||
            (y + piece->ghost_decline == dude->y_decline - 1))
    );
}

bool piece_ghost_piece_overlap(
    const enum_field (*field)[field_width],
    const struct_piece *piece, int x, int y
)
{
    return (field[y + piece->ghost_decline][x + piece->x_shift] == falling);
}

void piece_(
    piece_action action, const enum_field (*field)[field_width],
    const struct_piece *piece, const struct_dude *dude
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    const bool (*matrix)[piece->size] = piece->form.small;
    int x, y;
    for (y=0; y < piece->size; y++) {
        for (x=0; x < piece->size; x++) {
            if (matrix[y][x] == 1) {
                if (((action == print_ghost) || (action == hide_ghost)) &&
                    (dude_ghost_piece_overlap(dude, piece, x, y) ||
                    piece_ghost_piece_overlap(field, piece, x, y)))
                {
                    continue;
                }
                if ((action == print_ghost) || (action == hide_ghost))
                    take_(action, curr_x(piece, x), ghost_y(piece, y));
                else
                    take_(action, curr_x(piece, x), curr_y(piece, y));
            }
        }
    }
}

void truncate_piece(struct_piece *piece)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*matrix)[piece->size] = piece->form.small;
    int x, y;
    /* checking if the piece's upmost row is empty */
    while (true) {
        for (y=0; y < piece->size; y++) {
            for (x=0; x < piece->size; x++) {
                if (matrix[y][x] == 1)
                    return;
            }
            /* if so - correcting the initial piece's 'y' coordinate */
            piece->y_decline--;
        }
    }
}

bool field_has_ended(const struct_piece *piece, int y)
{
    return (y + piece->y_decline + 1 == field_height) ? true : false;
}

bool lower_field_cell_is_occupied(
    const enum_field (*field)[field_width],
    const struct_piece *piece, int x, int y
)
{
    return (field_has_ended(piece, y) ||
        field[y + piece->y_decline + 1][x + piece->x_shift] == 1);
}

bool piece_has_fallen(
    const enum_field (*field)[field_width], const struct_piece *piece
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    const bool (*matrix)[piece->size] = piece->form.small;
    int x, y;
    /* fall_checks: looking for every lowest cell in each column */
    for (y=piece->size-1; y >= 0; y--) {
        for (x=0; x < piece->size; x++) {
            if (matrix[y][x] == 1) {
                if (lower_field_cell_is_occupied(field, piece, x, y))
                    return true;
            }
        }
    }
    return false;
}

bool select_and_perform_transformation(
    transform_field_action action, enum_field (*field)[field_width],
    const struct_piece *piece, int x, int y
)
{
    bool leave = false;
    switch (action) {
        case field_absorbes_piece:
            field[y + piece->y_decline][x + piece->x_shift] = occupied;
            break;
        case record_falling_cells:
            if (field[y + piece->y_decline][x + piece->x_shift] != occupied)
                field[y + piece->y_decline][x + piece->x_shift] = falling;
            break;
        case erase_falling_cells:
            if (field[y + piece->y_decline][x + piece->x_shift] != occupied)
                field[y + piece->y_decline][x + piece->x_shift] = empty;
            break;
        default:
            fprintf(
                stderr, "%s:%d: incorrect value %d", __FILE__, __LINE__, action
            );
            exit(1);
    }
    return leave;
}

void transform_field_array(
    transform_field_action action, enum_field (*field)[field_width],
    const struct_piece *piece
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    const bool (*matrix)[piece->size] = piece->form.small;
    bool quit;
    int x, y;
    for (y=0; y < piece->size; y++) {
        for (x=0; x < piece->size; x++) {
            if (matrix[y][x] == 1) {
                quit = select_and_perform_transformation(
                    action, field, piece, x, y
                );
                if (quit) return;
            }
        }
    }
}

void cast_ghost(
    const enum_field (*field)[field_width], struct_piece piece,
    signed char *ghost_decline, const struct_dude *dude
)
{
    while (!piece_has_fallen(field, &piece))
        piece.y_decline++;
    piece.ghost_decline = piece.y_decline;
    *ghost_decline = piece.y_decline;
    piece_(print_ghost, field, &piece, dude);
}

void piece_spawn(
    enum_field (*field)[field_width], struct_piece *piece,
    const struct_dude *dude
)
{
    truncate_piece(piece);
    cast_ghost(field, *piece, &piece->ghost_decline, dude);
    transform_field_array(record_falling_cells, field, piece);
    piece_(print_piece, NULL, piece, NULL);
    curs_set(0);
    refresh();
}

void move_(
    move_direction direction, enum_field (*field)[field_width],
    struct_piece *piece, const struct_dude *dude
)
{
    int x_shift_backup = piece->x_shift;
    piece_(hide_ghost, field, piece, dude);
    transform_field_array(erase_falling_cells, field, piece);
    piece_(hide_piece, NULL, piece, NULL);
    switch (direction) {
        case left:
            piece->x_shift--;
            break;
        case right:
            piece->x_shift++;
            break;
        default:
            fprintf(stderr, "%s:%d: incorrect value", __FILE__, __LINE__);
            exit(1);
    }
    if (field_or_side_boundaries_conflict(field, piece))
        piece->x_shift = x_shift_backup;
    cast_ghost(field, *piece, &piece->ghost_decline, dude);
    transform_field_array(record_falling_cells, field, piece);
    piece_(print_piece, NULL, piece, NULL);
}

void choose_curr_dude_img_array(
    const struct_dude *dude, const char *const **dude_img_arr
)
{
    if (dude->posture == straight) {
        if (dude->direction == forward)
            *dude_img_arr = straight_dude_goes_forth;
        else
        if (dude->direction == backward)
            *dude_img_arr = straight_dude_goes_back;
        else
            goto error_msg;
    }
    else
    if (dude->posture == squat) {
        if (dude->direction == forward)
            *dude_img_arr = squat_dude_goes_forth;
        else
        if (dude->direction == backward)
            *dude_img_arr = squat_dude_goes_back;
        else
            goto error_msg;
    }
    else {
        error_msg:
        fprintf(stderr, "%s:%d: incorrect value", __FILE__, __LINE__);
        exit(1);
    }
}

int get_screen_x(int x)
{
    return get_init_x() + (x * cell_width);
}

int get_screen_y(int y)
{
    return get_init_y() + (y * cell_height);
}

int get_dude_screen_x(const struct_dude *dude)
{
    return get_init_x() + (dude->x_shift * cell_width);;
}

int get_last_dude_y(const struct_dude *dude)
{
    return dude->y_decline * cell_height + cell_height - 1;
}

int dude_cell_height(const struct_dude *dude)
{
    return dude->height * cell_height;
}

int get_dude_screen_y(const struct_dude *dude)
{
    return get_init_y() + get_last_dude_y(dude) - (dude_cell_height(dude) - 1);
}

void dude_(dude_action action, const struct_dude *dude)
{
    const char *const *dude_img_arr;
    if (action == print_dude)
        choose_curr_dude_img_array(dude, &dude_img_arr);
    int i, x, y;
    for (i = 0, x = get_dude_screen_x(dude), y = get_dude_screen_y(dude);
        i < dude_cell_height(dude);
        i++, y++)
    {
        if (action == print_dude)
            mvprintw(y, x, "%s", dude_img_arr[i]);
        else
        if (action == hide_dude)
            mvprintw(y, x, EMPTY_CELL_ROW);
        else {
            fprintf(
                stderr, "%s:%d: incorrect `action` value: %d\n",
                __FILE__, __LINE__, action
            );
            exit(1);
        }
    }
}

int dude_lives_y()
{
    return (get_init_y() + lives_row * cell_height);
}

int side_panel_x()
{
    return get_init_x() + (field_width + side_panel_gap) * cell_width;
}

void dude_lives_(dude_lives_action action, const struct_dude *dude)
{
    const char *const *dude_img_arr = straight_dude_goes_forth;
    int i, x, y;
    for (i = 0, x = side_panel_x(), y = dude_lives_y();
        i < dude_straight_height * cell_height;
        i++, x = side_panel_x(), y++)
    {
        int j;
        for (j=0; j < dude->lives; j++, x += dude_width * cell_width + 1)
        {
            if (action == print_dude_lives)
                mvprintw(y, x, "%s ", dude_img_arr[i]);
            else
            if (action == hide_dude_lives)
                mvprintw(y, x, "%s ", EMPTY_CELL_ROW);
            else {
                fprintf(
                    stderr, "%s:%d: incorrect `action` value: %d\n",
                    __FILE__, __LINE__, action
                );
                exit(1);
            }
        }
    }
}

void piece_fall_step(
    enum_field (*field)[field_width], struct_piece *piece
)
{
    piece_(hide_piece, NULL, piece, NULL);
    transform_field_array(erase_falling_cells, field, piece);
    piece->y_decline++;
    transform_field_array(record_falling_cells, field, piece);
    piece_(print_piece, NULL, piece, NULL);
    curs_set(0);
    refresh();
}

bool dude_escaped(const struct_dude *dude)
{
    return (
        (dude->y_decline == dude_exit_height) &&
        ((dude->x_shift == -1) || (dude->x_shift == field_width))
    );
}

int mid_field_y()
{
    return (get_init_y() + (field_height * cell_height) / 2 - 1);
}

int dude_esc_msg_field_x()
{
    return (
        get_init_x() +
        (field_width * cell_width - strlen(DUDE_ESCAPED_MESSAGE)) / 2
    );
}

void hide_escape_message()
{
    long unsigned int i;
    move(mid_field_y(), dude_esc_msg_field_x());
    for (i=0; i < strlen(DUDE_ESCAPED_MESSAGE); i++)
        addch(' ');
}

void reset_dude_characteristics(struct_dude *dude)
{
        dude->x_shift = 0;
        dude->y_decline = last_field_row_num;
        dude->height = dude_straight_height;
        dude->posture = straight;
        dude->direction = forward;
}

void reset_piece_characteristics(struct_piece *piece)
{
        piece->x_shift = initial_piece_shift,
        piece->y_decline = 0;
        piece->ghost_decline = 0;
}

int side_panel_y(int y)
{
    return get_init_y() + y * cell_height;
}

void print_side_panel(int info, int position)
{
    mvprintw(side_panel_y(position), side_panel_x(), "%d", info);
    curs_set(0);
    refresh();
}

void clear_game_field(enum_field (*field)[field_width])
{
    int field_x, screen_x, field_y, screen_y;
    for(field_y = 0, screen_y = get_init_y();
        field_y < field_height;
        field_y++, screen_y += cell_height)
    {
        for(field_x = 0, screen_x = get_init_x();
            field_x < field_width;
            field_x++, screen_x += cell_width)
        {
            field[field_y][field_x] = 0;
            print_cell_(empty, screen_x, screen_y);
        }
    }
}

bool dude_escape_handling(
    enum_field (*field)[field_width], struct_piece *piece,
    struct_dude *dude, struct_game_info *game_info
)
{
    if (dude_escaped(dude)) {
        dude_(print_dude, dude);
        clear_game_field(field);
        mvprintw(
            mid_field_y(), dude_esc_msg_field_x(), "%s", DUDE_ESCAPED_MESSAGE
        );
        refresh();
        sleep(escape_message_delay_in_sec);
        dude_(hide_dude, dude);
        hide_escape_message();
        reset_dude_characteristics(dude);
        dude_(print_dude, dude);
        reset_piece_characteristics(piece);
        piece_spawn(field, piece, dude);
        game_info->score += 1000;
        print_side_panel(game_info->score, score_row);
    }
    return false;
}

void dude_step(
    enum_field (*field)[field_width], struct_piece *piece,
    struct_dude *dude, struct_game_info *game_info
)
{
    bool dude_escaped = false;
    field[dude->y_decline][dude->x_shift] = empty;
    dude_(hide_dude, dude);
    if (dude->direction == forward)
        dude->x_shift++;
    else
        dude->x_shift--;
    dude_escaped = dude_escape_handling(field, piece, dude, game_info);
    if (dude_escaped)
        return;
    conflict_resolution_after_dude_took_a_step(field, dude);
    field[dude->y_decline][dude->x_shift] =  dude_cell;
    dude_(print_dude, dude);
    piece_(print_ghost, field, piece, dude);
    curs_set(0);
    refresh();
}

void handle_rotation(
    enum_field (*field)[field_width], struct_piece *piece,
    const struct_dude *dude
)
{
    bool backup_matrix[piece->size][piece->size];
    make_backup(backup_matrix, piece);
    piece_(hide_ghost, field, piece, dude);
    transform_field_array(erase_falling_cells, field, piece);
    piece_(hide_piece, NULL, piece, NULL);
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    rotate(piece->form.small, piece->size);
    handle_rotation_conflicts(field, piece, backup_matrix);
    cast_ghost(field, *piece, &piece->ghost_decline, dude);
    transform_field_array(record_falling_cells, field, piece);
    piece_(print_piece, NULL, piece, NULL);
}

struct_piece get_random_piece(const struct_piece *set_of_pieces)
{
    int i = (int)(((double)num_of_pieces) * rand() / (RAND_MAX+1.0));
    return set_of_pieces[i];
}

void show_next_piece_preview(struct_piece *piece, struct_piece *next_piece)
{
    piece->x_shift = field_width + side_panel_gap;
    next_piece->x_shift = field_width + side_panel_gap;
    piece->y_decline = next_row;
    next_piece->y_decline = next_row;
    piece_(hide_piece, NULL, piece, NULL);
    piece_(print_piece, NULL, next_piece, NULL);
    piece->x_shift = initial_piece_shift;
    piece->y_decline = 0;
    next_piece->x_shift = initial_piece_shift;
    next_piece->y_decline = 0;
    curs_set(0);
    refresh();
}

void minus_dude_life(
    enum_field (*field)[field_width], struct_dude *dude,
    struct_game_info *game_info
)
{
    refresh();
    sleep(dude_death_delay_in_sec);
    reset_dude_characteristics(dude);
    clear_game_field(field);
    dude_lives_(hide_dude_lives, dude);
    dude->lives--;
    dude_lives_(print_dude_lives, dude);
    dude_(print_dude, dude);
    refresh();
    if (dude->lives == 0) {
        game_info->game_on = false;
    }
}

void resolve_dude_moving_piece_conflict_and_reprint(
    enum_field (*field)[field_width], struct_dude *dude,
    struct_game_info *game_info
)
{
    bool reprint = false;
    dude_conflict_resolution_after_piece_move(
        field, dude, &reprint, &game_info->life_lost
    );
    if (reprint)
        dude_(print_dude, dude);
    if (game_info->life_lost)
        minus_dude_life(field, dude, game_info);
}

void process_rotation(
    enum_field (*field)[field_width], struct_piece *piece,
    struct_dude *dude, struct_game_info *game_info
    )
{
    handle_rotation(field, piece, dude);
    resolve_dude_moving_piece_conflict_and_reprint(field, dude, game_info);
}

void process_move_(
    move_direction direction, enum_field (*field)[field_width],
    struct_piece *piece, struct_dude *dude, struct_game_info *game_info
)
{
    move_(direction, field, piece, dude);
    resolve_dude_moving_piece_conflict_and_reprint(field, dude, game_info);
}

void process_hard_drop(
    enum_field (*field)[field_width], struct_piece *piece,
    struct_dude *dude, struct_game_info *game_info
)
{
    const dude_check_func dude_crushed_and_died[] = {
        NULL, NULL, &is_falling, NULL, NULL
    };
    const dude_check_func dude_squat[] = {
        NULL, &is_falling, NULL, NULL, NULL
    };
    while (!piece_has_fallen(field, piece))
        piece_fall_step(field, piece);
    if (dude_check(dude_crushed_and_died, field, dude, 0)) {
        minus_dude_life(field, dude, game_info);
        game_info->life_lost = true;
        return;
    }
    if (dude_check(dude_squat, field, dude, straight)) {
        dude->posture = squat;
        dude->height = dude_squat_height;
        dude_(print_dude, dude);
    }
}

void process_key(
    int key_pressed, enum_field (*field)[field_width],
    struct_piece *piece, struct_dude *dude,
    bool *hard_drop, struct_game_info *game_info
)
{
    switch (key_pressed) {
        case KEY_LEFT:
            process_move_(left, field, piece, dude, game_info);
            break;
        case KEY_RIGHT:
            process_move_(right, field, piece, dude, game_info);
            break;
        case KEY_UP:
            process_rotation(field, piece, dude, game_info);
            break;
        case ' ':
            process_hard_drop(field, piece, dude, game_info);
            *hard_drop = true;
            break;
        /* exit the game */
        case key_esc:
            game_info->game_on = false;
            break;
        case KEY_DOWN:
        case ERR:
            ;
    }
}

void input_processed_and_dude_takes_step(
    enum_field (*field)[field_width], struct_piece *piece,
    struct_dude *dude, struct_game_info *game_info
)
{
    struct timeval tv1, tv2;
    struct timezone tz;
    speed_list speed[] = {
        zero,     first,       second,      third,      fourth,    fifth,
        sixth,    seventh,     eighth,      ninth,      tenth,     eleventh,
        twelfth,  thirteenth,  fourteenth,  fifteenth
    };
    int key_pressed, time_passed, piece_delay = speed[game_info->level];
    static int dude_delay;
    if (dude_delay <=0)
        dude_delay = speed[game_info->level];
    for (;;) {
        bool hard_drop = false;
        timeout(1);
        time_start(&tv1, &tz);
        /* `getch()` also works as `refresh()` */
        key_pressed = getch();
        process_key(key_pressed, field, piece, dude, &hard_drop, game_info);
        if ((key_pressed == KEY_DOWN) || (hard_drop) ||
            (game_info->life_lost) || (!game_info->game_on))
        {
            break;
        }
        time_passed = time_stop(&tv1, &tv2, &tz);
        piece_delay -= time_passed;
        dude_delay -= time_passed;
        if (dude_delay < 0) {
            dude_step(field, piece, dude, game_info);
            dude_delay = speed[game_info->level];
        }
        if (piece_delay < 0)
            break;
        else
            continue;
    }
}

void piece_falls_and_dude_takes_step(
    enum_field (*field)[field_width], struct_piece *piece,
    struct_dude *dude, struct_game_info *game_info
)
{
    const dude_check_func dude_squat[] = {
        NULL, &is_falling, NULL, NULL, NULL
    };
    const dude_check_func dude_crushed_and_died[] = {
        NULL, NULL, &is_falling, NULL, NULL
    };
    while ((game_info->game_on)) {
        input_processed_and_dude_takes_step(field, piece, dude, game_info);
        if ((game_info->life_lost) || (!game_info->game_on))
            break;
        if (piece_has_fallen(field, piece)) {
            transform_field_array(field_absorbes_piece, field, piece);
            break;
        }
        piece_fall_step(field, piece);
        if (dude_check(dude_squat, field, dude, straight)) {
            dude->posture = squat;
            dude->height = dude_squat_height;
            dude_(print_dude, dude);
        }
        if (dude_check(dude_crushed_and_died, field, dude, 0)) {
            minus_dude_life(field, dude, game_info);
            game_info->life_lost = true;
            return;
        }
    }
}

void init_set_of_pieces(struct_piece *set_of_pieces)
{
    int i = 0;
    struct_piece I_piece = {
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
    memcpy(&set_of_pieces[i], &I_piece, sizeof(struct_piece));
    i++;
    struct_piece O_piece = {
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
    memcpy(&set_of_pieces[i], &O_piece, sizeof(struct_piece));
    i++;
    struct_piece T_piece = {
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
    memcpy(&set_of_pieces[i], &T_piece, sizeof(struct_piece));
    i++;
    struct_piece S_piece = {
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
    memcpy(&set_of_pieces[i], &S_piece, sizeof(struct_piece));
    i++;
    struct_piece Z_piece = {
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
    memcpy(&set_of_pieces[i], &Z_piece, sizeof(struct_piece));
    i++;
    struct_piece J_piece = {
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
    memcpy(&set_of_pieces[i], &J_piece, sizeof(struct_piece));
    i++;
    struct_piece L_piece = {
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
    memcpy(&set_of_pieces[i], &L_piece, sizeof(struct_piece));
}

void print_labels()
{
    mvprintw(side_panel_y(level_label_row), side_panel_x(), "LEVEL");
    mvprintw(side_panel_y(score_label_row), side_panel_x(), "SCORE");
    mvprintw(side_panel_y(next_label_row)+cell_height-1, side_panel_x(),"NEXT");
    mvprintw(side_panel_y(lives_label_row), side_panel_x(), "LIVES");
    curs_set(0);
    refresh();
}

void print_centered_format_msg(
    int *y, int col, const char *msg, int format_1, int format_2
)
{
    char assist_str[max_msg_str_size];
    sprintf(assist_str, msg, format_1, format_2);
    msg = assist_str;
    int x = (col - strlen(msg)) / 2;
    mvprintw(*y, x, "%s", msg);
    (*y)++;
}

void print_score_message(int score)
{
    int row, col;
    int y;
    getmaxyx(stdscr, row, col);
    y = row / 2;
    print_centered_format_msg(&y, col, FINAL_SCORE_MSG, score, 0);
}

void wait_until_esc_is_pressed_then_exit()
{
    timeout(-1);
    while (getch() != key_esc)
        ;
    endwin();
    exit(0);
}

void end_game(int score)
{
    clear();
    print_score_message(score);
    wait_until_esc_is_pressed_then_exit();
}

void process_uncompleted_line(
    bool we_are_checking_block_with_completed_lines,
    bool **sequence_of_completed_lines
)
{
    if (we_are_checking_block_with_completed_lines)
        (*sequence_of_completed_lines)++;
}

void process_completed_line(
    int *num_of_completed_lines, bool **sequence_of_completed_lines,
    int *row_num_of_first_completed_line, int y
)
{
    (*num_of_completed_lines)++;
    **sequence_of_completed_lines = 1;
    (*sequence_of_completed_lines)++;
    if (!(*row_num_of_first_completed_line))
        *row_num_of_first_completed_line = y;
}

bool there_are_completed_lines(
    const enum_field (*field)[field_width], int *num_of_completed_lines,
    int *row_num_of_first_completed_line, bool *sequence_of_completed_lines
)
{
    int x, y;
    bool we_are_checking_block_with_completed_lines = false;
    /* searching for completed lines from the bottom to the top */
    for (y=field_height-1; y > 0; y--) {
        bool completed_line = true, empty_line = true;
        for (x=0; x < field_width; x++) {
            if (field[y][x] == 1)
                empty_line = false;
            else
                completed_line = false;
        }
        if (completed_line) {
            we_are_checking_block_with_completed_lines = true;
            process_completed_line(
                num_of_completed_lines, &sequence_of_completed_lines,
                row_num_of_first_completed_line, y
            );
        } else {
            process_uncompleted_line(
                we_are_checking_block_with_completed_lines,
                &sequence_of_completed_lines
            );
        }
        if (empty_line ||
            /* since we started to check the block of completed lines and we
            have already looked through the maximum number of possible
            completed lines so we can break now */
            (*row_num_of_first_completed_line - y ==
            max_num_of_completed_lines - 1))
        {
            break;
        }
    }
    return (*num_of_completed_lines) ? true : false;
}

void shift_down_upper_not_empty_lines_for_num_positions(
    enum_field (*field)[field_width], int init_row_to_replace,
    int num, int *field_y, int *screen_y
)
{
    int field_x, screen_x;
    for(*field_y = init_row_to_replace,
            *screen_y = get_init_y() + init_row_to_replace * cell_height;
        *field_y > 0;
        (*field_y)--, *screen_y -= cell_height)
    {
        bool empty_line = true;
        for(field_x = 0, screen_x = get_init_x();
            field_x < field_width;
            field_x++, screen_x += cell_width)
        {
            field[*field_y][field_x] = field[*field_y-num][field_x];
            if (field[*field_y-num][field_x] == dude_cell)
                field[*field_y-num][field_x] = 0;
            if (field[*field_y][field_x] == 1) {
                empty_line = false;
                print_cell_(occupied, screen_x, *screen_y);
            } else
                print_cell_(empty, screen_x, *screen_y);
        }
        if (empty_line)
            break;
    }
}

void replace_upmost_not_empty_lines_with_empty_cells(
    enum_field (*field)[field_width], int num, int field_y, int screen_y
)
{
    int field_x, screen_x;
    for(num--, field_y--, screen_y -= cell_height;
        num > 0;
        num--, field_y--, screen_y -= cell_height)
    {
        for(field_x = 0, screen_x = get_init_x();
            field_x < field_width;
            field_x++, screen_x += cell_width)
        {
            field[field_y][field_x] = 0;
            print_cell_(empty, screen_x, screen_y);
        }
    }
}

void delete_completed_lines(
    enum_field (*field)[field_width], int init_row_to_replace, int num
)
{
    int field_y, screen_y;
    shift_down_upper_not_empty_lines_for_num_positions(
        field, init_row_to_replace, num, &field_y, &screen_y
    );
    replace_upmost_not_empty_lines_with_empty_cells(
        field, num, field_y, screen_y
    );
}

bool curr_line_is_completed(int num)
{
    return (num) ? true : false;
}

void find_continuous_block_of_completed_lines(
    bool *sequence_of_completed_lines, int *i, int *num_of_deleted_lines
)
{
    while ((*i < max_num_of_completed_lines) && sequence_of_completed_lines[*i])
    {
        (*num_of_deleted_lines)++;
        (*i)++;
    }
}

void field_matrix_rearrangement(
    enum_field (*field)[field_width], int init_row_to_replace,
    bool *sequence_of_completed_lines
)
{
    int i = 0, num_of_deleted_lines = 0;
    for(;
        i < max_num_of_completed_lines;
        i++, num_of_deleted_lines = 0, init_row_to_replace--)
    {
        find_continuous_block_of_completed_lines(
            sequence_of_completed_lines, &i, &num_of_deleted_lines
        );
        if (curr_line_is_completed(num_of_deleted_lines))
            delete_completed_lines(
                field, init_row_to_replace, num_of_deleted_lines
            );
        else
            /* it's an uncompleted line in the block of completed lines */
            continue;
    }
}

int score_bonus(int level, int num_of_completed_lines)
{
    switch (num_of_completed_lines) {
        case 1:
            return level * one_line_score_bonus;
        case 2:
            return level * two_lines_score_bonus;
        case 3:
            return level * three_lines_score_bonus;
        case 4:
            return level * four_lines_score_bonus;
        default:
            fprintf(
                stderr, "%s:%d: incorrect number of completed lines: %d\n",
                __FILE__, __LINE__, num_of_completed_lines
            );
            exit(1);
    }
}

void score_increase(struct_game_info *game_info, int num_of_completed_lines)
{
    game_info->score += score_bonus(game_info->level, num_of_completed_lines);
}

void level_up_if_necessary(
    struct_game_info *game_info, int num_of_completed_lines
)
{
    static int total_num_of_completed_lines;
    total_num_of_completed_lines += num_of_completed_lines;
    if (total_num_of_completed_lines >= num_of_completed_lines_for_level_up) {
        game_info->level++;
        if (game_info->level > maximum_game_level)
            game_info->level = maximum_game_level;
        print_side_panel(game_info->level, level_row);
        total_num_of_completed_lines = 0;
    }
}

void dude_coordinates_adjustment(
    const enum_field (*field)[field_width], struct_dude *dude
)
{
    if (field[dude->y_decline][dude->x_shift] == dude_cell)
        return;
    int i;
    for (i=0, dude->y_decline++;
        i < max_num_of_completed_lines;
        i++, dude->y_decline++)
    {
        if (field[dude->y_decline][dude->x_shift] == dude_cell)
            return;
    }
    fprintf(
        stderr, "%s:%d Failed to find the new \"dude cell\" value.\n",
        __FILE__, __LINE__
    );
    exit(1);
}

void dude_crash_handling(
    enum_field (*field)[field_width], struct_dude *dude,
    struct_game_info *game_info
)
{
    usleep(dude_fall_speed_in_microseconds);
    dude_(hide_dude, dude);
    dude->posture = squat;
    dude->height = 1;
    dude_(print_dude, dude);
    minus_dude_life(field, dude, game_info);
    game_info->life_lost = true;
}

bool empty_cell_under_dude(
    const enum_field (*field)[field_width], const struct_dude *dude
)
{
    if (dude->y_decline == last_field_row_num)
        return false;
    if (field[dude->y_decline + 1][dude->x_shift] == empty)
        return true;
    return false;
}

void dude_falls_if_empty_cell_under_him(
    enum_field (*field)[field_width], struct_dude *dude,
    struct_game_info *game_info
)
{
    int num_of_cells_dude_flew_through = 0;
    if (!empty_cell_under_dude(field, dude))
        return;
    dude_(print_dude, dude);
    refresh();
    while (empty_cell_under_dude(field, dude)) {
        usleep(dude_fall_speed_in_microseconds);
        dude_(hide_dude, dude);
        field[dude->y_decline][dude->x_shift] = empty;
        dude->y_decline++;
        field[dude->y_decline][dude->x_shift] = dude_cell;
        dude_(print_dude, dude);
        refresh();
        num_of_cells_dude_flew_through++;
    }
    if (num_of_cells_dude_flew_through > save_fall_height_for_dude)
        dude_crash_handling(field, dude, game_info);
}

void clear_completed_lines_update_game_info(
    enum_field (*field)[field_width], struct_dude *dude,
    struct_game_info *game_info
)
{
    int num_of_completed_lines = 0, row_num_of_first_completed_line = 0;
    /* completed lines may not be continuous and may contain breaks */
    /* the following array records this sequence */
    bool sequence_of_completed_lines[max_num_of_completed_lines] = { 0 };
    if (there_are_completed_lines(
            field, &num_of_completed_lines, &row_num_of_first_completed_line,
            sequence_of_completed_lines
        )
    )
    {
        dude_(hide_dude, dude);
        field_matrix_rearrangement(
            field, row_num_of_first_completed_line, sequence_of_completed_lines
        );
        dude_coordinates_adjustment(field, dude);
        dude_falls_if_empty_cell_under_him(field, dude, game_info);
        if ((game_info->life_lost) || (!game_info->game_on))
            return;
        dude_(print_dude, dude);
        score_increase(game_info, num_of_completed_lines);
        print_side_panel(game_info->score, score_row);
        level_up_if_necessary(game_info, num_of_completed_lines);
    }
}

int min_screen_width()
{
    return
        (side_panel_gap + big_piece_size) * cell_width * 2 +
        field_width * cell_width - 1;
}

int min_screen_height()
{
    return 2*top_boundary_height + cell_height*field_height;
}

int init_resize_screen_y(int row)
{
    return (row - num_of_resize_msg_lines) / 2;
}

void print_resize_request_msg(int row, int col)
{
    int y;
    y = init_resize_screen_y(row);
    print_centered_format_msg(&y, col, RESIZE_WARNING_MSG, 0, 0);
    print_centered_format_msg(&y, col, CURR_TERMINAL_SIZE_MSG, col, row);
    print_centered_format_msg(
        &y, col, REQUIRED_TERMINAL_SIZE_MSG,
        min_screen_width(), min_screen_height()
    );
    print_centered_format_msg(&y, col, RESIZE_REQUEST_MSG_1, 0, 0);
    print_centered_format_msg(&y, col, RESIZE_REQUEST_MSG_2, 0, 0);
    print_centered_format_msg(&y, col, CLOSE_WINDOW_MSG, 0, 0);
    refresh();
    curs_set(0);
}

void screen_size_check()
{
    int row, col;
    getmaxyx(stdscr, row, col);
    if ((col < min_screen_width()) || (row < min_screen_height())) {
        print_resize_request_msg(row, col);
        wait_until_esc_is_pressed_then_exit();
    }
}

int main()
{
    /* ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, 1);
    ESCDELAY = 50;

    /* variables */
    struct_game_info game_info = { 1, 0, false, true };
    enum_field field[field_height][field_width] = { 0 };
    struct_piece set_of_pieces[num_of_pieces];
    init_set_of_pieces(set_of_pieces);
    struct_piece piece, next_piece;
    struct_dude dude = {
        0, last_field_row_num, dude_straight_height,
        straight, forward, init_num_of_dude_lives
    };

    /* MAIN */
    screen_size_check();
    srand(time(NULL));
    print_field(field);
    print_labels();
    print_side_panel(game_info.level, level_row);
    print_side_panel(game_info.score, score_row);
    dude_lives_(print_dude_lives, &dude);
    dude_(print_dude, &dude);
    next_piece = get_random_piece(set_of_pieces);
    while (game_info.game_on) {
        game_info.life_lost = false;
        piece = next_piece;
        next_piece = get_random_piece(set_of_pieces);
        show_next_piece_preview(&piece, &next_piece);
        piece_spawn(field, &piece, &dude);
        if (falling_piece_field_crossing_conflict(field, &piece)) break;
        piece_falls_and_dude_takes_step(field, &piece, &dude, &game_info);
        if (!game_info.game_on) break;
        if (game_info.life_lost) continue;
        clear_completed_lines_update_game_info(field, &dude, &game_info);
    }
    end_game(game_info.score);
}
