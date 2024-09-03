/* conflict_resolution.c */

#include "conflict_resolution.h"
#include <stdlib.h>

#define MAKE_FUNCTION_MATRIX_COPY(SIZE) \
    void SIZE ## _matrix_copy( \
        bool (*dst)[SIZE ## _piece_size], \
        const bool (*src)[SIZE ## _piece_size] \
    ) \
    { \
        int x, y; \
        for (y=0; y < SIZE ## _piece_size; y++) { \
            for (x=0; x < SIZE ## _piece_size; x++) \
                dst[y][x] = src[y][x]; \
        } \
    }

MAKE_FUNCTION_MATRIX_COPY(small)

MAKE_FUNCTION_MATRIX_COPY(big)

void prev_orientation(figure *piece)
{
    /* traversing a list of enumerated values cyclically in reverse order
    (after the 1st value, we get the last value) */
    piece->orientation = piece->orientation - 1;
    if (piece->orientation < 0)
        piece->orientation = piece->orientation + orientation_count;
}

void apply_backup(void *src, figure *piece, int dx, int dy)
{
    bool (*backup_matrix)[piece->size] = src;
    switch (piece->size) {
        case small_piece_size:
            small_matrix_copy(piece->form.small, backup_matrix);
            break;
        case big_piece_size:
            big_matrix_copy(piece->form.big, backup_matrix);
    }
    piece->x_shift -= dx;
    piece->y_decline -= dy;
    if (piece->i_form)
        prev_orientation(piece);
}

void make_backup(void *dst, figure *piece)
{
    bool (*backup_matrix)[piece->size] = dst;
    switch (piece->size) {
        case small_piece_size:
            small_matrix_copy(backup_matrix, piece->form.small);
            break;
        case big_piece_size:
            big_matrix_copy(backup_matrix, piece->form.big);
    }
}

bool o_piece(figure *piece)
{
    return ((piece->size == big_piece_size) && (!piece->i_form)) ? true : false;
}

bool horizontal_1_long_side_center_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 2][piece->x_shift + 2] == 1)
        return true;
    else
        return false;
}

bool horizontal_1_long_side_border_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 2][piece->x_shift + 3] == 1)
        return true;
    else
        return false;
}

bool horizontal_1_short_side_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 2][piece->x_shift + 0] == 1)
        return true;
    else
        return false;
}

void i_piece_horizontal_1_rotation_conflict_handling(
    bool (*field)[field_width], figure *piece, int *dx
)
{
    int tmpdx = 0;
    if (horizontal_1_short_side_conflict(field, piece)) {
        if (
            horizontal_1_long_side_border_conflict(field, piece) ||
            horizontal_1_long_side_center_conflict(field, piece)
        )
        {
            return;
        } else {
            tmpdx = 1;
        }
    }
    else
    if (horizontal_1_long_side_center_conflict(field, piece))
        tmpdx = -2;
    else
    if (horizontal_1_long_side_border_conflict(field, piece))
        tmpdx = -1;
    piece->x_shift += tmpdx;
    *dx += tmpdx;
}

bool vertical_2_long_side_center_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 2][piece->x_shift + 2] == 1)
        return true;
    else
        return false;
}

bool vertical_2_long_side_border_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 3][piece->x_shift + 2] == 1)
        return true;
    else
        return false;
}

bool vertical_2_short_side_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 0][piece->x_shift + 2] == 1)
        return true;
    else
        return false;
}

void i_piece_vertical_2_rotation_conflict_handling(
    bool (*field)[field_width], figure *piece, int *dy
)
{
    int tmpdy = 0;
    if (vertical_2_short_side_conflict(field, piece)) {
        if (
            vertical_2_long_side_border_conflict(field, piece) ||
            vertical_2_long_side_center_conflict(field, piece)
        )
        {
            return;
        } else {
            tmpdy = 1;
        }
    }
    else
    if (vertical_2_long_side_center_conflict(field, piece))
        tmpdy = -2;
    else
    if (vertical_2_long_side_border_conflict(field, piece))
        tmpdy = -1;
    piece->y_decline += tmpdy;
    *dy += tmpdy;
}

bool horizontal_2_long_side_center_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 1][piece->x_shift + 1] == 1)
        return true;
    else
        return false;
}

bool horizontal_2_long_side_border_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 1][piece->x_shift + 0] == 1)
        return true;
    else
        return false;
}

bool horizontal_2_short_side_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 1][piece->x_shift + 3] == 1)
        return true;
    else
        return false;
}

void i_piece_horizontal_2_rotation_conflict_handling(
    bool (*field)[field_width], figure *piece, int *dx
)
{
    int tmpdx = 0;
    if (horizontal_2_short_side_conflict(field, piece)) {
        if (
            horizontal_2_long_side_border_conflict(field, piece) ||
            horizontal_2_long_side_center_conflict(field, piece)
        )
        {
            return;
        } else {
            tmpdx = -1;
        }
    }
    else
    if (horizontal_2_long_side_center_conflict(field, piece))
        tmpdx = 2;
    else
    if (horizontal_2_long_side_border_conflict(field, piece))
        tmpdx = 1;
    piece->x_shift += tmpdx;
    *dx += tmpdx;
}

bool vertical_1_long_side_center_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 1][piece->x_shift + 1] == 1)
        return true;
    else
        return false;
}

bool vertical_1_long_side_border_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 0][piece->x_shift + 1] == 1)
        return true;
    else
        return false;
}

bool vertical_1_short_side_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (field[piece->y_decline + 3][piece->x_shift + 1] == 1)
        return true;
    else
        return false;
}

void i_piece_vertical_1_rotation_conflict_handling(
    bool (*field)[field_width], figure *piece, int *dy
)
{
    int tmpdy = 0;
    if (vertical_1_short_side_conflict(field, piece)) {
        if (
            vertical_1_long_side_border_conflict(field, piece) ||
            vertical_1_long_side_center_conflict(field, piece)
        )
        {
            return;
        } else {
            tmpdy = -1;
        }
    }
    else
    if (vertical_1_long_side_center_conflict(field, piece))
        tmpdy = 2;
    else
    if (vertical_1_long_side_border_conflict(field, piece))
        tmpdy = 1;
    piece->y_decline += tmpdy;
    *dy += tmpdy;
}

/*
    Possible conflicts:

        - short side conflict:

        . . . .    . 1 . .
        . . . .    . 1 . .
        1 1 1 1 => . 1 . .
        . 1 . .    . X . .

        - long side border conflict:

        . 1 . .    . X . .
        . . . .    . 1 . .
        1 1 1 1 => . 1 . .
        . . . .    . 1 . .

        - long side center conflict:

        . . . .    . 1 . .
        . 1 . .    . X . .
        1 1 1 1 => . 1 . .
        . . . .    . 1 . .

*/
void i_form_piece_rotation_conflicts_handling(
    bool (*field)[field_width], figure *piece, int *dx, int *dy
)
{
    switch (piece->orientation) {
        case (horizontal_1):
            i_piece_horizontal_1_rotation_conflict_handling(field, piece, dx);
            break;
        case (vertical_1):
            i_piece_vertical_1_rotation_conflict_handling(field, piece, dy);
            break;
        case (horizontal_2):
            i_piece_horizontal_2_rotation_conflict_handling(field, piece, dx);
            break;
        case (vertical_2):
            i_piece_vertical_2_rotation_conflict_handling(field, piece, dy);
            break;
        case (orientation_count):
            ;
    }
}

bool cell_occupied_by_(bool (*field)[field_width], int x, int y, figure *piece)
{
    return (field[y+piece->y_decline][x+piece->x_shift] == 1) ? true : false;
}

bool piece_field_pixel_crossing_conflict(
    bool (*field)[field_width], figure *piece
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*matrix)[piece->size] = piece->form.small;
    int x, y;
    for (y=0; y < piece->size; y++) {
        for (x=0; x < piece->size; x++) {
            if ((matrix[y][x] == 1) && (cell_occupied_by_(field, x, y, piece)))
                return true;
        }
    }
    return false;
}

bool top_center_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (
        (field[piece->y_decline + 0][piece->x_shift + 1] == 1) &&
        (piece->form.small[0][1] == 1)
    )
    {
        return true;
    } else
        return false;
}

bool top_right_corner_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (
        (field[piece->y_decline + 0][piece->x_shift + 2] == 1) &&
        (piece->form.small[0][2] == 1)
    )
    {
        return true;
    } else
        return false;
}

bool right_center_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (
        (field[piece->y_decline + 1][piece->x_shift + 2] == 1) &&
        (piece->form.small[1][2] == 1)
    )
    {
        return true;
    } else
        return false;
}

bool bottom_right_corner_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (
        (field[piece->y_decline + 2][piece->x_shift + 2] == 1) &&
        (piece->form.small[2][2] == 1)
    )
    {
        return true;
    } else
        return false;
}

bool bottom_center_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (
        (field[piece->y_decline + 2][piece->x_shift + 1] == 1) &&
        (piece->form.small[2][1] == 1)
    )
    {
        return true;
    } else
        return false;
}

bool bottom_left_corner_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (
        (field[piece->y_decline + 2][piece->x_shift + 0] == 1) &&
        (piece->form.small[2][0] == 1)
    )
    {
        return true;
    } else
        return false;
}

bool left_center_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (
        (field[piece->y_decline + 1][piece->x_shift + 0] == 1) &&
        (piece->form.small[1][0] == 1)
    )
    {
        return true;
    } else
        return false;
}

bool top_left_corner_conflict(
    bool (*field)[field_width], figure *piece
)
{
    if (
        (field[piece->y_decline + 0][piece->x_shift + 0] == 1) &&
        (piece->form.small[0][0] == 1)
    )
    {
        return true;
    } else
        return false;
}

void handle_left_center_conflict(
    bool (*field)[field_width], figure *piece, int *dx, int *dy
)
{
    if (right_center_conflict(field, piece))
        return;
    if (top_right_corner_conflict(field, piece)) {
        piece->y_decline++;
        *dy += 1;
        return;
    } else
    if (bottom_right_corner_conflict(field, piece)) {
        piece->y_decline--;
        *dy -= 1;
        return;
    }
    piece->x_shift++;
    *dx += 1;
}

void handle_bottom_center_conflict(
    bool (*field)[field_width], figure *piece, int *dx, int *dy
)
{
    if (top_center_conflict(field, piece))
        return;
    if (top_left_corner_conflict(field, piece)) {
        piece->x_shift++;
        *dx += 1;
        return;
    } else
    if (top_right_corner_conflict(field, piece)) {
        piece->x_shift--;
        *dx -= 1;
        return;
    }
    piece->y_decline--;
    *dy -= 1;
}

void handle_right_center_conflict(
    bool (*field)[field_width], figure *piece, int *dx, int *dy
)
{
    if (left_center_conflict(field, piece))
        return;
    if (top_left_corner_conflict(field, piece)) {
        piece->y_decline++;
        *dy += 1;
        return;
    } else
    if (bottom_left_corner_conflict(field, piece)) {
        piece->y_decline--;
        *dy -= 1;
        return;
    }
    piece->x_shift--;
    *dx -= 1;
}

void handle_top_center_conflict(
    bool (*field)[field_width], figure *piece, int *dx, int *dy
)
{
    if (bottom_center_conflict(field, piece))
        return;
    if (bottom_left_corner_conflict(field, piece)) {
        piece->x_shift++;
        *dx += 1;
        return;
    } else
    if (bottom_right_corner_conflict(field, piece)) {
        piece->x_shift--;
        *dx -= 1;
        return;
    }
    piece->y_decline++;
    *dy += 1;
}

/*
    Possible conflicts:

        - center conflict:

        . 1 1    . . .
        2 1 . => X 1 1    -> move in the opposite direction
        . 1 .    . . 1

        - double center conflict:

        . 1 1    . . .
        2 1 2 => X 1 X    no solution, roll the rotation back
        . 1 .    . . 1

        - corner conflict:

        1 . 2    . 1 X
        1 1 1 => . 1 .    <- move as it were left or right center conflict
        . . .    . 1 .

        - center + corner conflict:
            -- if the conflicting pixels are in the same row/column:

        1 2 2    . X X    |
        1 1 1 => . 1 .    V move as it were just a center conflict
        . . .    . 1 .

            -- if the conflicting pixels are in the opposite rows/columns:

        1 . 2    . 1 X
        1 1 1 => . 1 .    <- move toward the only empty row/column
        . 2 .    . X .

*/
void regular_piece_rotation_conflicts_handling(
    bool (*field)[field_width], figure *piece, int *dx, int *dy
)
{
    if (top_center_conflict(field, piece)) {
        handle_top_center_conflict(field, piece, dx, dy);
    } else
    if (right_center_conflict(field, piece)) {
        handle_right_center_conflict(field, piece, dx, dy);
    } else
    if (bottom_center_conflict(field, piece)) {
        handle_bottom_center_conflict(field, piece, dx, dy);
    } else
    if (left_center_conflict(field, piece)) {
        handle_left_center_conflict(field, piece, dx, dy);
    } else
    if (
        top_left_corner_conflict(field, piece) ||
        bottom_left_corner_conflict(field, piece)
    )
    {
        piece->x_shift++;
        *dx += 1;
    }
    else
    if (
        top_right_corner_conflict(field, piece) ||
        bottom_right_corner_conflict(field, piece)
    )
    {
        piece->x_shift--;
        *dx -= 1;
    }
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
    move_direction direction, bool (*field)[field_width], figure *piece
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*matrix)[piece->size] = piece->form.small;
    int x, y;
    int start_x, end_x, incr_x;
    set_x_cycle_pixel_cond(direction, piece, &start_x, &end_x, &incr_x);
    for (y=0; y < piece->size; y++) {
        for (x=start_x; x != end_x; x+=incr_x) {
            if ((matrix[y][x] == 1) && (cell_occupied_by_(field, x, y, piece)))
            {
                piece->x_shift += incr_x;
                return;
            }
        }
    }
}

bool out_of_bottom_field_boundary(figure *piece)
{
    return (piece->y_decline > field_height - piece->size) ? true : false;
}

bool out_of_top_field_boundary(figure *piece)
{
    return (piece->y_decline < 0) ? true : false;
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

bool bottom_top_boundaries_crossing_(
    crossing_action action, figure *piece, int *dy
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*matrix)[piece->size] = piece->form.small;
    bool res = false;
    int x, y;
    int start_y, end_y, incr_y;
    if (!set_y_cycle_cond(piece, &start_y, &end_y, &incr_y))
        return res;
    for (y=start_y; y != end_y; y+=incr_y) {
        for (x=0; x < piece->size; x++) {
            if (matrix[y][x] == 1) {
                res = true;
                if (action == signal)
                    return res;
                piece->y_decline += incr_y;
                *dy += incr_y;
                if (special_i_piece_bottom_top_case(piece))
                    continue;
                else
                    return res;
            }
        }
    }
    return res;
}

bool out_of_right_boundary(figure *piece)
{
    return (piece->x_shift > field_width - piece->size) ? true : false;
}

bool out_of_left_boundary(figure *piece)
{
    return (piece->x_shift < 0) ? true : false;
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

bool special_i_piece_side_case(figure *piece)
{
    return ((piece->i_form) && horizontal_orientation(piece)) ? true : false;
}

bool side_boundaries_crossing_(crossing_action action, figure *piece, int *dx)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    bool (*matrix)[piece->size] = piece->form.small;
    bool res = false;
    int x, y;
    int start_x, end_x, incr_x;
    if (!set_x_cycle_bound_cond(piece, &start_x, &end_x, &incr_x))
        return res;
    for (y=0; y < piece->size; y++) {
        for (x=start_x; x != end_x; x+=incr_x) {
            if (matrix[y][x] == 1) {
                res = true;
                if (action == signal)
                    return res;
                piece->x_shift += incr_x;
                if (dx)
                    *dx += incr_x;
                if (special_i_piece_side_case(piece))
                    continue;
                else
                    return res;
            }
        }
    }
    return res;
}

void next_orientation(figure *piece)
{
    /* traversing a list of enumerated values cyclically (after the last
    value, we get the 1st value again) */
    piece->orientation = (piece->orientation + 1) % orientation_count;
}

void handle_i_piece(figure *piece, int *dx)
{
    /* after the rotation of the I piece, we need that both of it vertical
    incarnations were at the same column */
    if (piece->i_form) {
        if (piece->orientation == vertical_1) {
            piece->x_shift--;
            (*dx)--;
        } else
        if (piece->orientation == vertical_2) {
            piece->x_shift++;
            (*dx)++;
        }
        next_orientation(piece);
    }
}

void handle_rotation_conflicts(bool (*field)[field_width], figure *piece)
{
    int dx = 0, dy = 0;
    bool backup_matrix[piece->size][piece->size];
    if (o_piece(piece))
        return;
    make_backup(backup_matrix, piece);
    handle_i_piece(piece, &dx);
    /* prevent any boundaries crossing conflicts
    and then look whether we have a piece/field pixel crossing conflict now */
    if (side_boundaries_crossing_(prevention, piece, &dx))
        goto piece_field_pixel_crossing_check;
    if (bottom_top_boundaries_crossing_(prevention, piece, &dy))
        goto piece_field_pixel_crossing_check;
    if (piece->i_form)
        i_form_piece_rotation_conflicts_handling(field, piece, &dx, &dy);
    else
        regular_piece_rotation_conflicts_handling(field, piece, &dx, &dy);
    /* if after all our efforts we still have conflicts - restore the initial
    piece space orientation and its `x_shift` and `y_decline` */
    if (side_boundaries_crossing_(signal, piece, NULL)) {
        apply_backup(backup_matrix, piece, dx, dy);
        return;
    }
    if (bottom_top_boundaries_crossing_(signal, piece, NULL)) {
        apply_backup(backup_matrix, piece, dx, dy);
        return;
    }
    piece_field_pixel_crossing_check:
    if (piece_field_pixel_crossing_conflict(field, piece))
        apply_backup(backup_matrix, piece, dx, dy);
}