/* conflict_resolution.c */

#include "conflict_resolution.h"
#include <stdio.h>
#include <stdlib.h>

#define MAKE_FUNCTION_MATRIX_COPY(FUNNAME, SIZE) \
    void FUNNAME( \
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

MAKE_FUNCTION_MATRIX_COPY(small_matrix_copy, small)

MAKE_FUNCTION_MATRIX_COPY(big_matrix_copy, big)

void make_backup(void *dst, const figure *piece)
{
    bool (*backup_matrix)[piece->size] = dst;
    switch (piece->size) {
        case small_piece_size:
            small_matrix_copy(backup_matrix, piece->form.small);
            break;
        case big_piece_size:
            big_matrix_copy(backup_matrix, piece->form.big);
            break;
        default:
            fprintf(
                stderr, "%s:%d: incorrect piece size %d\n",
                __FILE__, __LINE__, piece->size
            );
            exit(1);
    }
}

static void prev_orientation(figure *piece)
{
    /* traversing a list of enumerated values cyclically in reverse order
    (after the 1st value, we get the last value) */
    piece->orientation = piece->orientation - 1;
    if (piece->orientation < 0)
        piece->orientation = piece->orientation + orientation_count;
}

static void apply_backup(figure *piece, const void *src, int dx, int dy)
{
    const bool (*backup_matrix)[piece->size] = src;
    switch (piece->size) {
        case small_piece_size:
            small_matrix_copy(piece->form.small, backup_matrix);
            break;
        case big_piece_size:
            big_matrix_copy(piece->form.big, backup_matrix);
            break;
        default:
            fprintf(
                stderr, "%s:%d: incorrect piece size %d\n",
                __FILE__, __LINE__, piece->size
            );
            exit(1);
    }
    piece->x_shift -= dx;
    piece->y_decline -= dy;
    if (piece->i_form)
        prev_orientation(piece);
}

static bool o_piece(const figure *piece)
{
    return ((piece->size == big_piece_size) && (!piece->i_form));
}

static bool i_piece_rotation_conflict(
    const bool (*field)[field_width], const figure *piece, int x, int y
)
{
    return (field[piece->y_decline + y][piece->x_shift + x] == 1);
}

static void handle_i_form_piece_rotation_conflict(
    const bool (*field)[field_width], const figure *piece,
    const int (*confl_coords)[2], const int *amendments, int *coordinate
)
{
    int confl_1_x = confl_coords[0][0], confl_1_y = confl_coords[0][1];
    int confl_2_x = confl_coords[1][0], confl_2_y = confl_coords[1][1];
    int confl_3_x = confl_coords[2][0], confl_3_y = confl_coords[2][1];
    int amendment = 0;
        /* short_side_conflict */
    if (i_piece_rotation_conflict(field, piece, confl_1_x, confl_1_y)) {
            /* long_side_border_conflict */
        if (i_piece_rotation_conflict(field, piece, confl_2_x, confl_2_y) ||
            /* long_side_center_conflict */
            i_piece_rotation_conflict(field, piece, confl_3_x, confl_3_y))
        {
            return;
        } else
            amendment = amendments[0];
    }
    else
        /* long_side_border_conflict */
    if (i_piece_rotation_conflict(field, piece, confl_2_x, confl_2_y))
        amendment = amendments[1];
    else
        /* long_side_center_conflict */
    if (i_piece_rotation_conflict(field, piece, confl_3_x, confl_3_y))
        amendment = amendments[2];
    *coordinate += amendment;
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
static void i_form_piece_rotation_conflicts_handling(
    const bool (*field)[field_width], figure *piece, int *dx, int *dy
)
{
    int tmpdx = 0, tmpdy = 0;
    switch (piece->orientation) {
        case (horizontal_1): {
            int conflict_coordinates[][2] = { { 0, 2 }, { 3, 2 }, { 2, 2 } };
            int amendments[] = { 1, -2, -1 };
            handle_i_form_piece_rotation_conflict(
                field, piece, conflict_coordinates, amendments, &tmpdx
            );
            break;
        }
        case (vertical_1): {
            int conflict_coordinates[][2] = { { 1, 3 }, { 1, 0 }, { 1, 1 } };
            int amendments[] = { -1, 2, 1 };
            handle_i_form_piece_rotation_conflict(
                field, piece, conflict_coordinates, amendments, &tmpdy
            );
            break;
        }
        case (horizontal_2): {
            int conflict_coordinates[][2] = { { 3, 1 }, { 0, 1 }, { 1, 1 } };
            int amendments[] = { -1, 2, 1 };
            handle_i_form_piece_rotation_conflict(
                field, piece, conflict_coordinates, amendments, &tmpdx
            );
            break;
        }
        case (vertical_2): {
            int conflict_coordinates[][2] = { { 2, 0 }, { 2, 3 }, { 2, 2 } };
            int amendments[] = { 1, -2, -1 };
            handle_i_form_piece_rotation_conflict(
                field, piece, conflict_coordinates, amendments, &tmpdy
            );
            break;
        }
        case (orientation_count):
            ;
            break;
        default:
            fprintf(stderr, "%s:%d: incorrect value", __FILE__, __LINE__);
            exit(1);
    }
    piece->x_shift += tmpdx;
    *dx += tmpdx;
    piece->y_decline += tmpdy;
    *dy += tmpdy;
}

static bool cell_occupied_by_(
    const bool (*field)[field_width], int x, int y, const figure *piece
)
{
    return (field[y+piece->y_decline][x+piece->x_shift] == 1);
}

bool piece_field_crossing_conflict(
    const bool (*field)[field_width], const figure *piece
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    const bool (*matrix)[piece->size] = piece->form.small;
    int x, y;
    for (y=0; y < piece->size; y++) {
        for (x=0; x < piece->size; x++) {
            if ((matrix[y][x] == 1) && (cell_occupied_by_(field, x, y, piece)))
                return true;
        }
    }
    return false;
}

static bool regular_piece_rotation_conflict(
    const bool (*field)[field_width], const figure *piece, int x, int y
)
{
    return ((field[piece->y_decline + y][piece->x_shift + x] == 1) &&
        (piece->form.small[y][x] == 1));
}

static void handle_regular_piece_rotation_conflict(
    const bool (*field)[field_width], const figure *piece,
    const int (*confl_coords)[2], int *amendment_1, int *amendment_2,
    int default_change
)
{
    int confl_1_x = confl_coords[0][0], confl_1_y = confl_coords[0][1];
    int confl_2_x = confl_coords[1][0], confl_2_y = confl_coords[1][1];
    int confl_3_x = confl_coords[2][0], confl_3_y = confl_coords[2][1];
    /*  - double center conflict: no solution, roll the rotation back */
    if (regular_piece_rotation_conflict(field, piece, confl_1_x, confl_1_y))
        return;

    /*  - center + corner conflict: if the conflicting cells are in the
    opposite rows/columns: move toward the only empty row/column */
    if (regular_piece_rotation_conflict(field, piece, confl_2_x, confl_2_y)) {
        *amendment_1 += 1;
        return;
    } else
    /* the same center + corner conflict as the previous one.
    The difference - now the conflicting cell is in the opposite far corner */
    if (regular_piece_rotation_conflict(field, piece, confl_3_x, confl_3_y)) {
        *amendment_1 -= 1;
        return;
    }
    /* no additional conflicting cells. Just and ordinary center conflict */
    *amendment_2 += default_change;
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
            -- if the conflicting cells are in the same row/column:

        1 2 2    . X X    |
        1 1 1 => . 1 .    V move as it were just a center conflict
        . . .    . 1 .

            -- if the conflicting cells are in the opposite rows/columns:

        1 . 2    . 1 X
        1 1 1 => . 1 .    <- move toward the only empty row/column
        . 2 .    . X .

*/
static void regular_piece_rotation_conflicts_handling(
    const bool (*field)[field_width], figure *piece, int *dx, int *dy
)
{
    int tmpdx = 0, tmpdy = 0;
        /* top center conflict */
    if (regular_piece_rotation_conflict(field, piece, 1, 0)) {
        int conflict_coordinates[][2] = { { 1, 2 }, { 0, 2 }, { 2, 2 } };
        handle_regular_piece_rotation_conflict(
            field, piece, conflict_coordinates, &tmpdx, &tmpdy, 1
        );
    } else
        /* right center conflict */
    if (regular_piece_rotation_conflict(field, piece, 2, 1)) {
        int conflict_coordinates[][2] = { { 0, 1 }, { 0, 0 }, { 0, 2 } };
        handle_regular_piece_rotation_conflict(
            field, piece, conflict_coordinates, &tmpdy, &tmpdx, -1
        );
    } else
        /* bottom center conflict */
    if (regular_piece_rotation_conflict(field, piece, 1, 2)) {
        int conflict_coordinates[][2] = { { 1, 0 }, { 0, 0 }, { 2, 0 } };
        handle_regular_piece_rotation_conflict(
            field, piece, conflict_coordinates, &tmpdx, &tmpdy, -1
        );
    } else
        /* left center conflict */
    if (regular_piece_rotation_conflict(field, piece, 0, 1)) {
        int conflict_coordinates[][2] = { { 2, 1 }, { 2, 0 }, { 2, 2 } };
        handle_regular_piece_rotation_conflict(
            field, piece, conflict_coordinates, &tmpdy, &tmpdx, 1
        );
    } else
        /* top left corner conflict */
    if (regular_piece_rotation_conflict(field, piece, 0, 0) ||
        /* bottom left corner conflict */
        regular_piece_rotation_conflict(field, piece, 0, 2))
    {
        tmpdx++;
    }
    else
        /* top right corner conflict */
    if (regular_piece_rotation_conflict(field, piece, 2, 0) ||
        /* bottom right corner conflict */
        regular_piece_rotation_conflict(field, piece, 2, 2))
    {
        tmpdx--;
    }
    piece->x_shift += tmpdx;
    *dx += tmpdx;
    piece->y_decline += tmpdy;
    *dy += tmpdy;
}

static bool set_x_cycle_cell_cond(
    move_direction direction, const figure *piece,
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
        default:
            fprintf(stderr, "%s:%d: incorrect value", __FILE__, __LINE__);
            exit(1);
    }
    return true;
}

void side_cells_crossing_prevention(
    move_direction direction, const bool (*field)[field_width], figure *piece
)
{
    /* `piece->form.small` and `piece->form.big` share the same address,
    so we handle both scenarios here */
    const bool (*matrix)[piece->size] = piece->form.small;
    int x, y;
    int start_x, end_x, incr_x;
    set_x_cycle_cell_cond(direction, piece, &start_x, &end_x, &incr_x);
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

static bool out_of_bottom_field_boundary(const figure *piece)
{
    return (piece->y_decline > field_height - piece->size);
}

static bool out_of_top_field_boundary(const figure *piece)
{
    return (piece->y_decline < 0);
}

static bool set_y_cycle_cond(
    const figure *piece, int *start_y, int *end_y, int *incr_y
)
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

static bool vertical_orientation(const figure *piece)
{
    return ((piece->orientation == vertical_1) ||
        (piece->orientation == vertical_2));
}

static bool special_i_piece_bottom_top_case(const figure *piece)
{
    return ((piece->i_form) && vertical_orientation(piece));
}

static bool bottom_top_boundaries_crossing_(
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

static bool out_of_right_boundary(const figure *piece)
{
    return (piece->x_shift > field_width - piece->size);
}

static bool out_of_left_boundary(const figure *piece)
{
    return (piece->x_shift < 0);
}

static bool set_x_cycle_bound_cond(
    const figure *piece, int *start_x, int *end_x, int *incr_x
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

static bool horizontal_orientation(const figure *piece)
{
    return ((piece->orientation == horizontal_1) ||
        (piece->orientation == horizontal_2));
}

static bool special_i_piece_side_case(const figure *piece)
{
    return ((piece->i_form) && horizontal_orientation(piece));
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

static void next_orientation(figure *piece)
{
    /* traversing a list of enumerated values cyclically (after the last
    value, we get the 1st value again) */
    piece->orientation = (piece->orientation + 1) % orientation_count;
}

static void handle_i_piece(figure *piece, int *dx)
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

void handle_rotation_conflicts(
    const bool (*field)[field_width], figure *piece, const void *backup
)
{
    const bool (*backup_matrix)[piece->size] = backup;
    int dx = 0, dy = 0;
    if (o_piece(piece))
        return;
    handle_i_piece(piece, &dx);
    /* prevent any boundaries crossing conflicts
    and then look whether we have a piece/field cell crossing conflict now */
    if (side_boundaries_crossing_(prevention, piece, &dx))
        goto piece_field_cell_crossing_check;
    if (bottom_top_boundaries_crossing_(prevention, piece, &dy))
        goto piece_field_cell_crossing_check;
    if (piece->i_form)
        i_form_piece_rotation_conflicts_handling(field, piece, &dx, &dy);
    else
        regular_piece_rotation_conflicts_handling(field, piece, &dx, &dy);
    /* if after all our efforts we still have conflicts - restore the initial
    piece space orientation and its `x_shift` and `y_decline` */
    if (side_boundaries_crossing_(signal, piece, NULL)) {
        apply_backup(piece, backup_matrix, dx, dy);
        return;
    }
    if (bottom_top_boundaries_crossing_(signal, piece, NULL)) {
        apply_backup(piece, backup_matrix, dx, dy);
        return;
    }
    piece_field_cell_crossing_check:
    if (piece_field_crossing_conflict(field, piece))
        apply_backup(piece, backup_matrix, dx, dy);
}
