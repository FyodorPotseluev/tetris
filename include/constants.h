/* constants.h */

#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

#include <stdbool.h>

enum constants {
    /* the delay value - the time in milliseconds it takes for a piece to fall
    by one pixel */
    init_delay          = 100000,          /* 100 seconds */
    /* game field size in pixels */
    field_height        = 20,
    field_width         = 10,
    /* the number of pieces available in the game */
    num_of_pieces       = 7,
    /* possible piece sizes in pixels */
    small_piece_size    = 3,
    big_piece_size      = 4,
    /* initial piece shift to the left border of the field when it spawns */
    initial_piece_shift = 4
};

typedef enum tag_move_direction { left = 1, right } move_direction;
typedef enum tag_position {
    horizontal_1, vertical_1, horizontal_2, vertical_2, orientation_count
} position;

typedef struct tag_figure {
    /* piece size */
    unsigned char size;
    /* pixel order in the piece */
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
} figure;

#endif
