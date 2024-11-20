
                             tetris / tetriller

                                Description

    The project allows you to switch between the classic Tetris game and its Tetriller mode, where you try to place the blocks so that the man at the bottom of the play field can climb to one of the two exits.

                                Prerequisites

    Uses the `unistd` library, so it won't run natively on Windows or other non-Unix platforms.
    Uses the `ncurses` library. If you don't already have it, it can be installed the following way:
    Debian/Ubuntu Linux:
        ```
        sudo apt-get install libncurses5-dev libncursesw5-dev
        ```
    CentOS/RHEL/Scientific Linux 6.x/7.x+ and Fedora Linux 21 or older:
        ```
        sudo yum install ncurses-devel
        ```
    Fedora Linux 22.x+:
        ```
        sudo dnf install ncurses-devel
        ```
    Arch Linux:
        ```
        sudo pacman -S ncurses
        ```
    macOS (using Homebrew):
        ```
        brew install ncurses
        ```

                                Installation

    After cloning the repository, it will contain the Tetris game by default.

    To switch to the Tetriller game, run
    ```
    git checkout tetriller
    ```
    To switch back to the Tetris game, run
    ```
    git checkout master
    ```
    Run `make` in a terminal window in the project directory to compile the game.

                                Usage

    Run `make run` in a terminal window in the project directory to start the game.

    The control keys:

    Move left     - left arrow key;
    Move right    - right arrow key;
    Rotate        - up arrow key;
    Soft drop     - down arrow key;
    Hard drop     - space bar;
    Exit the game - the Esc key.

    Run `make help` to see the list of Makefile commands.

                                Contributing

    Discussions:   github.com/FyodorPotseluev/tetris/discussions
    Issue Tracker: github.com/FyodorPotseluev/tetris/issues

                                License

    MIT License

                                Contact

    kommunist90@gmail.com
