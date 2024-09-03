/* rotation.h */

#ifndef ROTATION_DEMO_H_INCLUDED
#define ROTATION_DEMO_H_INCLUDED

void rotate(void *figure, int len);
/*
    Rotates the incoming matrix 90 degrees clockwise.
RECEIVES:
    - `figure` an untyped pointer to the incoming matrix;
    - `len` the matrix size;
ERROR HANDLING:
    - the `len` can't be different from `small_piece_size` and `big_piece_size`
values. If it does, an error message is printed with the current `len` value and
the program terminates. */

#endif
