/* conflict_resolution.h */

#ifndef CONFLICT_RESOLUTION_H_INCLUDED
#define CONFLICT_RESOLUTION_H_INCLUDED

#include "constants.h"

typedef enum tag_crossing_action { prevention, signal } crossing_action;

bool side_boundaries_crossing_(
    crossing_action action, struct_piece *piece, int *dx
);
/*
    Indicates whether the piece has crossed the field side boundary. If the
`crossing_action` value is `signal`, it does nothing else. If the
`crossing_action` value is `prevention` - prevents the crossing by changing
the `piece->x_shift` value and changes the `*dx` value if it can.
RECEIVES:
    - `action` indicates whether the function prevents the detected crossing;
    - `piece` the pointer to the structure containing the current pice
    properties;
    - `dx` characterizes the piece shift. Its main purpose is to keep track of
    changes so you can undo them later. Can be passed as NULL;
RETURNS:
    - the boolean value indicating whether a side boundary crossing took place. */

bool field_or_side_boundaries_conflict(
    const enum_field (*field)[field_width], const struct_piece *piece
);
/*
    Signals if a piece now has a conflict with occupied field cells, or if it
crosses the side field boundaries.
RECEIVES:
    - `field` the pointer to the matrix describing the current field state;
    - `piece` the pointer to the structure containing the current piece
    properties.
RETURNS:
    - the boolean value indicating whether the conflict occurred.
*/

bool falling_piece_field_crossing_conflict(
    const enum_field (*field)[field_width], const struct_piece *piece
);
/*
    Signals if a piece cell is crossing an occupied field cell.
RECEIVES:
    - `field` the pointer to the matrix describing the current field state;
    - `piece` the pointer to the structure containing the current piece
    properties.
RETURNS:
    - the boolean value indicating whether a piece/field cell crossing occurred.
*/

void make_backup(void *dst, const struct_piece *piece);
/*
    Copies to the `dst` argument the current state of the `piece->form` matrix.
It can be used later to restore the initial `piece->form` state.
RECEIVES:
    - `dst` untyped pointer to the matrix to store the current state of the
    `piece->form` matrix;
    - `piece` the pointer to the structure containing the current piece
    properties.
RETURNS:
    ---
ERROR HANDLING:
    - the `len` can't be different from `small_piece_size` and `big_piece_size`
    values. If it does, an error message is printed with the current `len` value
    and the program terminates. */


void handle_rotation_conflicts(
    const enum_field (*field)[field_width], struct_piece *piece,
    const void *backup
);
/*
    Prevents conflicts (crossing the field borders or already occupied field
cells) after a rotation of a piece. In some cases, if a conflict is too deep, it
rolls the rotation back.
RECEIVES:
    - `field` the pointer to the matrix describing the current field state;
    - `piece` the pointer to the structure containing the current piece
    properties;
    - `backup` untyped pointer to the matrix containing the state of the piece
    before the rotation. It's used for rolling the rotation back in case we
    couldn't resolve the conflict.
RETURNS:
    --- */

bool dude_check(
    const dude_check_func *arr_of_check_funcs,
    const enum_field (*field)[field_width],
    const struct_dude *dude, enum_posture posture
);
/*
    The function is called when we need to check the "dude" posture and
surrounding "dude" cells (e.g. if, for example, "the dude ran into a wall",
"the dude stepped off a cliff", "the dude stepped up one step", "the dude has
been crushed and died" e.t.c.). It returns "true" if the "dude's" surroundings
and his posture match our expectations, and "false" if they don't.
RECEIVES:
    - `arr_of_check_funcs` the expected content of the cells surrounding the
    dude. It's an array of 5 elements. Each element corresponds to the relevant
    cell:
        -- 1st - the cell above the "dude's" head;
        -- 2nd - the cell with the "dude's" head;
        -- 3rd - the cell with the "dude's" legs";
        -- 4th - the cell under the "dude's" legs;
        -- 5th - the cell under the previous one.
    The array consists of function addresses of `dude_check_func` type. If we
    don't need to check a cell - we pass the respective element as a `NULL`
    address.
    We can pass the following function addresses:
        -- `is_falling`(see its description below);
        -- `is_empty`;
        -- `is_occupied`;
        -- `is_falling_or_occupied`;
        -- `is_out_of_field`;
        -- `is_in_field_and_empty`;
        -- `is_in_field_and_falling`;
        -- `is_out_of_field_or_occupied`;
        -- `is_out_of_field_or_occupied_or_falling`.
    - `field` the pointer to the matrix describing the current field state;
    - `dude` the pointer to the structure containing the current "dude"
    properties;
    - `posture` the expected "dude" posture. If it doesn't matter can be passed
    as zero;
RETURNS:
    - returns "true" if the "dude's" surroundings and his posture match our
    expectations, and "false" if they don't.
ERROR HANDLING:
    --- */

bool is_falling(
    const enum_field (*field)[field_width], const struct_dude *dude, int num
);
/*
    Checks the state of a cell around the "dude" (the exact location is set with
the `num` argument) and returns `true` if it is a falling cell and `false` if it
is not.
    A `dude_check_func` type of function. Its address is used in the array
passed as the first parameter of the `dude_check` function (see above).
RECEIVES:
    - `field` the pointer to the matrix describing the current field state;
    - `dude` the pointer to the structure containing the current "dude"
    properties;
    - `num` - the number of the cell we check:
        -- 0 - the cell above the "dude's" head;
        -- 1 - the cell with the "dude's" head;
        -- 2 - the cell with the "dude's" legs;
        -- 3 - the cell under the "dude's" legs;
        -- 4 - the cell under the previous one.
RETURNS:
    - `true` if the current cell is falling and `false` if it is not.
ERROR HANDLING:
    --- */

void death_handling(bool *game_on);
/*
    What happens after the "dude" dies. We pause the current game screen for a second and end the game.
RECEIVES:
    - `game_on` the pointer to the boolean value indicating whether the game continues or it is over;
RETURNS:
    ---
ERROR HANDLING:
    --- */

void dude_conflict_resolution_after_piece_move(
    const enum_field (*field)[field_width], struct_dude *dude,
    bool *game_on, bool *reprint
);
/*
    Must be called when the game pice has been moved by the player. If it caused
a conflict, resolves it (the "dude" squats or straightens) or ends the game if
the main character was smashed.
RECEIVES:
    - `field` the pointer to the matrix describing the current field state;
    - `dude` the pointer to the structure containing the current "dude"
    properties;
    - `game_on` the pointer to the boolean value indicating whether the game
    continues or it is over;
    - `reprint` the pointer to the boolean value indicating whether we need to
    reprint the "dude" after the function has finished.
RETURNS:
    ---
ERROR HANDLING:
    --- */

void conflict_resolution_after_dude_took_a_step(
    const enum_field (*field)[field_width], struct_dude *dude
);
/*
    If the "dude" has taken a step and there is a conflict with his surrounding,
resolve this conflict (the "dude" turns around, squats, straightens, steps up
and down).
RECEIVES:
    - `field` the pointer to the matrix describing the current field state;
    - `dude` the pointer to the structure containing the current "dude"
    properties;
RETURNS:
    ---
ERROR HANDLING:
    --- */

#endif
