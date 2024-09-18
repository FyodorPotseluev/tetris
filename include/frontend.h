/* frontend.h */

#ifndef FRONTEND_H_INCLUDED
#define FRONTEND_H_INCLUDED

enum frontend_consts {
    cell_height     = 1,
    cell_width      = 2,
    score_label_row = 1,
    score_row       = 3,
    next_label_row  = 6,
    next_row        = 7,
    game_info_gap   = 2 
};

#define EMPTY_CELL_ROW    "0 "

#define OCCUPIED_CELL_ROW "1 "

#define GHOST_CELL_ROW    ". "

#endif
