
                                  tetris

                                Description

    Yet another implementation of the famous Tetris game.

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

    Run `make` in a terminal window in the project directory.

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
