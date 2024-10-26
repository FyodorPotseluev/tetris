/* frontend.h */

#ifndef FRONTEND_H_INCLUDED
#define FRONTEND_H_INCLUDED

enum frontend_consts {
    /* cell dimensions */
    cell_height                         = 2,
    cell_width                          = 3,
    /* playing field boundaries dimensions */
    side_boundary_width                 = 1,
    top_boundary_height                 = 1,
    /* which field row matches the corresponding label or its value
    (the game info's y coordinate) */
    level_label_row                     = 0,
    level_row                           = 1,
    score_label_row                     = 3,
    score_row                           = 4,
    next_label_row                      = 6,
    next_row                            = 7,
    /* how far the game info is from the playing field
    (the game info's x coordinate) */
    game_info_gap                       = 2
};

/* how the dude's characters cells look like: */

const char *const straight_dude_goes_forth[] = {
    " o ",
    " -<",
    " | ",
    "/ >"
};

const char *const straight_dude_goes_back[] = {
    " o ",
    ">- ",
    " | ",
    "< \\"
};

const char *const squat_dude_goes_forth[] = {
    " -o",
    ">^<"
};

const char *const squat_dude_goes_back[] = {
    "o- ",
    ">^<"
};

/* how one row of a playing field cell looks like: */

#define EMPTY_CELL_ROW      "   "

#define OCCUPIED_CELL_ROW   "###"

#define GHOST_CELL_ROW      ":::"

/* how one character cell of the playing field boundary looks like: */

#define BOTTOM_TOP_BOUNDARY "-"

#define SIDE_BOUNDARY       "|"

#endif
