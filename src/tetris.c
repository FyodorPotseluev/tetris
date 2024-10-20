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

typedef enum tag_piece_action {
    hide_piece, print_piece, hide_ghost, print_ghost
} piece_action;

typedef enum tag_type_of_cell {
    empty, occupied, ghost
} type_of_cell;

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

void print_cell_(type_of_cell type, int x, int y)
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

void print_field(const bool (*field)[field_width])
{
    int field_x, field_y, screen_x, screen_y;
    print_field_boundary(top, NULL, NULL);
    for (
        field_y = 0, screen_x = init_screen_x(), screen_y = get_init_y();
        field_y < field_height;
        field_y++, screen_x = init_screen_x(), screen_y += cell_height
    )
    {
        print_field_boundary(left_side, &screen_x, &screen_y);
        for (field_x=0; field_x < field_width; field_x++) {
            if (field[field_y][field_x] == 0)
                print_cell_(empty, screen_x, screen_y);
            else
                print_cell_(occupied, screen_x, screen_y);
            screen_x += cell_width;
            move(screen_y, screen_x);
        }
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

void piece_(piece_action action, const struct_piece *piece)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    const bool (*matrix)[piece->size] = piece->form.small;
    int x, y;
    for (y=0; y < piece->size; y++) {
        for (x=0; x < piece->size; x++) {
            if (matrix[y][x] == 1) {
                if (action == hide_ghost)
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
    const bool (*field)[field_width], const struct_piece *piece, int x, int y
)
{
    return (field_has_ended(piece, y) ||
        field[y + piece->y_decline + 1][x + piece->x_shift] == 1);
}

bool piece_has_fallen(const bool (*field)[field_width], const struct_piece *piece)
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

void cast_ghost(
    const bool (*field)[field_width], struct_piece piece,
    signed char *ghost_decline
)
{
    while (!piece_has_fallen(field, &piece))
        piece.y_decline++;
    piece_(print_ghost, &piece);
    *ghost_decline = piece.y_decline;
}

void piece_spawn(const bool (*field)[field_width], struct_piece *piece)
{
    truncate_piece(piece);
    cast_ghost(field, *piece, &piece->ghost_decline);
    piece_(print_piece, piece);
    curs_set(0);
    refresh();
}

void field_absorbes_piece(bool (*field)[field_width], const struct_piece *piece)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    const bool (*matrix)[piece->size] = piece->form.small;
    int x, y;
    /* `piece` cells become `field` cells */
    for (y=0; y < piece->size; y++) {
        for (x=0; x < piece->size; x++) {
            if (matrix[y][x] == 1)
                field[y + piece->y_decline][x + piece->x_shift] = 1;
        }
    }
}

void move_(
    move_direction direction, const bool (*field)[field_width],
    struct_piece *piece
)
{
    int x_shift_backup = piece->x_shift;
    piece_(hide_ghost, piece);
    piece_(hide_piece, piece);
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
    cast_ghost(field, *piece, &piece->ghost_decline);
    piece_(print_piece, piece);
}

void piece_fall_step(struct_piece *piece)
{
    piece_(hide_piece, piece);
    piece->y_decline++;
    piece_(print_piece, piece);
    curs_set(0);
    refresh();
}

void handle_rotation(const bool (*field)[field_width], struct_piece *piece)
{
    bool backup_matrix[piece->size][piece->size];
    make_backup(backup_matrix, piece);
    piece_(hide_ghost, piece);
    piece_(hide_piece, piece);
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    rotate(piece->form.small, piece->size);
    handle_rotation_conflicts(field, piece, backup_matrix);
    cast_ghost(field, *piece, &piece->ghost_decline);
    piece_(print_piece, piece);
}

void process_key(
    int key_pressed, const bool (*field)[field_width],
    struct_piece *piece, bool *hard_drop, bool *game_on
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
            break;
        /* exit the game (Esc) */
        case key_esc:
            *game_on = false;
            break;
        case KEY_DOWN:
        case ERR:
            ;
    }
}

void process_input(
    const bool (*field)[field_width], struct_piece *piece,
    int level, bool *game_on
)
{
    struct timeval tv1, tv2;
    struct timezone tz;
    speed_list speed[] = {
        zero,     first,       second,      third,      fourth,    fifth,
        sixth,    seventh,     eighth,      ninth,      tenth,     eleventh,
        twelfth,  thirteenth,  fourteenth,  fifteenth
    };
    int key_pressed, delay = speed[level];
    for (;;) {
        bool hard_drop = false;
        timeout(delay);
        time_start(&tv1, &tz);
        key_pressed = getch();
        process_key(key_pressed, field, piece, &hard_drop, game_on);
        if (
            (key_pressed == ERR) || (key_pressed == KEY_DOWN) ||
            (hard_drop) || (!*game_on)
        )
        {
            break;
        }
        /* new delay value calculation */
        delay -= time_stop(&tv1, &tv2, &tz);
        if (delay < 0)
            break;
        else
            continue;
    }
}

void piece_falls(
    bool (*field)[field_width], struct_piece *piece, int level, bool *game_on
)
{
    while ((*game_on)) {
        process_input(field, piece, level, game_on);
        if (piece_has_fallen(field, piece)) {
            field_absorbes_piece(field, piece);
            break;
        }
        piece_fall_step(piece);
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

struct_piece get_random_piece(const struct_piece *set_of_pieces)
{
    int i = (int)(((double)num_of_pieces) * rand() / (RAND_MAX+1.0));
    return set_of_pieces[i];
}

int game_info_y(int y)
{
    return get_init_y() + y * cell_height;
}

int game_info_x()
{
    return get_init_x() + (field_width + game_info_gap) * cell_width;
}

void print_labels()
{
    mvprintw(game_info_y(level_label_row), game_info_x(), "LEVEL");
    mvprintw(game_info_y(score_label_row), game_info_x(), "SCORE");
    mvprintw(game_info_y(next_label_row)+cell_height-1, game_info_x(), "NEXT");
    curs_set(0);
    refresh();
}

void print_game_info(int info, int position)
{
    mvprintw(game_info_y(position), game_info_x(), "%d", info);
    curs_set(0);
    refresh();
}

void show_next_piece_preview(struct_piece piece, struct_piece next_piece)
{
    piece.x_shift = field_width + game_info_gap;
    next_piece.x_shift = field_width + game_info_gap;
    piece.y_decline = next_row;
    next_piece.y_decline = next_row;
    piece_(hide_piece, &piece);
    piece_(print_piece, &next_piece);
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
    const bool (*field)[field_width], int *num_of_completed_lines,
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
            /* since we met 1st completed line we looked through the maximum
            number of possibly completed lines and can break now */
            (*row_num_of_first_completed_line - y ==
            max_num_of_completed_lines - 1))
        {
            break;
        }
    }
    return (*num_of_completed_lines) ? true : false;
}

void shift_down_upper_not_empty_lines_for_num_positions(
    bool (*field)[field_width], int init_row_to_replace,
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
    bool (*field)[field_width], int num, int field_y, int screen_y
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
    bool (*field)[field_width], int init_row_to_replace, int num
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
    bool (*field)[field_width], int init_row_to_replace,
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

void score_increase(int *score, int level, int num_of_completed_lines)
{
    *score += score_bonus(level, num_of_completed_lines);
}

void level_up_if_necessary(int *level, int num_of_completed_lines)
{
    static int total_num_of_completed_lines;
    total_num_of_completed_lines += num_of_completed_lines;
    if (total_num_of_completed_lines >= num_of_completed_lines_for_level_up) {
        (*level)++;
        if ((*level) > maximum_game_level) (*level) = maximum_game_level;
        print_game_info(*level, level_row);
        total_num_of_completed_lines = 0;
    }
}

void clear_completed_lines_update_score_and_level_up(
    bool (*field)[field_width], int *level, int *score
)
{
    (void)score;
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
        field_matrix_rearrangement(
            field, row_num_of_first_completed_line, sequence_of_completed_lines
        );
        score_increase(score, *level, num_of_completed_lines);
        print_game_info(*score, score_row);
        level_up_if_necessary(level, num_of_completed_lines);
    }
}

int min_screen_width()
{
    return
        (game_info_gap + big_piece_size) * cell_width * 2 +
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
    int level = 1, score = 0;
    bool field[field_height][field_width] = { 0 };
    struct_piece set_of_pieces[num_of_pieces];
    init_set_of_pieces(set_of_pieces);
    struct_piece piece, next_piece;

    /* MAIN */
    screen_size_check();
    srand(time(NULL));
    print_field(field);
    print_labels();
    print_game_info(level, level_row);
    print_game_info(score, score_row);
    /* print_dude */
    next_piece = get_random_piece(set_of_pieces);
    bool game_on = true;
    while (game_on) {
        piece = next_piece;
        next_piece = get_random_piece(set_of_pieces);
        show_next_piece_preview(piece, next_piece);
        piece_spawn(field, &piece);
        if (piece_field_crossing_conflict(field, &piece))
            game_on = false;
        piece_falls(field, &piece, level, &game_on);
        clear_completed_lines_update_score_and_level_up(field, &level, &score);
    }
    end_game(score);
}
