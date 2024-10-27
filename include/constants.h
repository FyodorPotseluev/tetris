/* constants.h */

#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

#include <stdbool.h>

enum constants {
    /* game field size */
    field_height                        = 20,
    last_field_row_num                  = 19,
    field_width                         = 10,
    /* dude size */
    dude_straight_height                = 2,
    dude_squat_height                   = 1,
    dude_width                          = 1,
    /* the number of pieces available in the game */
    num_of_pieces                       = 7,
    /* piece sizes */
    small_piece_size                    = 3,
    big_piece_size                      = 4,
    /* initial piece shift to the left border of the field when it spawns */
    initial_piece_shift                 = 4,
    /* the maximum number of lines that can be completed in one game move */
    max_num_of_completed_lines          = 4,
    key_esc                             = 27,
    /* the number of scores you get for completing the specified number of
    lines at a time (multiplied by the current level) */
    one_line_score_bonus                = 100,
    two_lines_score_bonus               = 300,
    three_lines_score_bonus             = 500,
    four_lines_score_bonus              = 800,
    /* the number of lines you need to get level up */
    num_of_completed_lines_for_level_up = 10,
    /* the maximum game level after which you cannot level up any more */
    maximum_game_level                  = 15,
    /* the number of lines the resize request message consists of */
    num_of_resize_msg_lines             = 6,
    /* the maximum length of a game message (for example, the score message) */
    max_msg_str_size                    = 80
};

/* the list of piece fall step delays (the time in milliseconds it takes for a
piece to fall by one cell) for the corresponding level */
typedef enum tag_speed_list {
        zero,           first      = 1000000, second     = 793,  third     = 618,
        fourth  = 473,  fifth      = 355,  sixth      = 262,  seventh   = 190,
        eighth  = 135,  ninth      = 94,   tenth      = 64,   eleventh  = 43,
        twelfth = 28,   thirteenth = 18,   fourteenth = 11,   fifteenth = 7
} speed_list;

/* the messages used when the player's terminal window is too small */

#define RESIZE_WARNING_MSG         "YOUR TERMINAL WINDOW IS TOO SMALL!"

#define CURR_TERMINAL_SIZE_MSG     "IT IS %dx%d CHARACTER CELLS NOW."

#define REQUIRED_TERMINAL_SIZE_MSG "IT NEEDS TO BE AT LEAST %dx%d."

#define RESIZE_REQUEST_MSG_1       "PLEASE RESIZE YOUR TERMINAL"

#define RESIZE_REQUEST_MSG_2       "AND TRY AGAIN."

#define CLOSE_WINDOW_MSG           "PRESS `ESC` TO CLOSE THIS WINDOW."

/* the message at the end of the game */

#define FINAL_SCORE_MSG            "YOUR SCORE IS %d"

typedef enum tag_move_direction { left = 1, right } move_direction;

typedef enum tag_position {
    horizontal_1, vertical_1, horizontal_2, vertical_2, orientation_count
} position;

typedef enum tag_enum_direction {
    forward, backward
} enum_direction;

typedef enum tag_enum_posture {
    straight, squat
} enum_posture;

typedef enum tag_enum_field {
    empty = 0, occupied = 1, ghost, falling
} enum_field;

typedef struct tag_struct_piece {
    /* piece size */
    unsigned char size;
    /* cell order in the piece */
    union tag_form {
        bool small[small_piece_size][small_piece_size];
        bool big[big_piece_size][big_piece_size];
    } form;
    /* current piece coordinates to the top left field corner.
    `ghost_decline` - the current ghost piece decline to the top border of the
    field */
    signed char x_shift, y_decline, ghost_decline;
    /* is it I-form piece? */
    bool i_form;
    /* the current piece orientation in space */
    position orientation;
} struct_piece;

typedef struct tag_struct_dude {
    /* current dude coordinates to the top left field corner. */
    signed char x_shift, y_decline;
    /* current dude's height */
    unsigned char height;
    /* posture - either straight or squat */
    enum_posture posture;
    /* moving direction - either forward or backward */
    enum_direction direction;
} struct_dude;

#endif
