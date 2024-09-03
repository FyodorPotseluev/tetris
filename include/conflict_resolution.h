/* conflict_resolution.h */

#ifndef CONFLICT_RESOLUTION_H_INCLUDED
#define CONFLICT_RESOLUTION_H_INCLUDED

#include "constants.h"

typedef enum tag_crossing_action { prevention, signal } crossing_action;

bool side_boundaries_crossing_(crossing_action action, figure *piece, int *dx);
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

void side_pixels_crossing_prevention(
    move_direction direction, bool (*field)[field_width], figure *piece
);
/*
    Prevents a piece from crossing already occupied side field pixels by
changing the `piece->x_shift` value.
RECEIVES:
    - `direction` the direction in which the piece was just moved;
    - `field` the pointer to the matrix describing the current field state;
    - `piece` the pointer to the structure containing the current piece
    properties.
RETURNES:
    --- */

void handle_rotation_conflicts(bool (*field)[field_width], figure *piece);
/*
    Prevents conflicts (crossing the field borders or already occupied field pixels) after a rotation of a piece. In some cases, if a conflict is too deep, it rolls the rotation back.
RECEIVES:
    - `field` the pointer to the matrix describing the current field state;
    - `piece` the pointer to the structure containing the current piece properties.
RETURNES:
    --- */

#endif
