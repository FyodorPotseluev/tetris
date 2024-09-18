/* frontend.h */

#ifndef FRONTEND_H_INCLUDED
#define FRONTEND_H_INCLUDED

enum frontend_consts {
    cell_height         = 2,
    cell_width          = 3,
    side_boundary_width = 1,
    top_boundary_height = 1,
    score_label_row     = 2,
    score_row           = 3,
    next_label_row      = 6,
    next_row            = 7,
    game_info_gap       = 2
};

#define EMPTY_CELL_ROW      "   "

#define OCCUPIED_CELL_ROW   "###"

#define GHOST_CELL_ROW      ":::"

#define BOTTOM_TOP_BOUNDARY "-"

#define SIDE_BOUNDARY       "|"

#endif
