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
RETURNES:
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
RETURNES:
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
RETURNES:
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
RETURNES:
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
RETURNES:
    --- */

#endif
