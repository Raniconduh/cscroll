# cscroll

A small file manager written in C.


## Usage

If an argument is provided, cscroll will open the path supplied. Otherwise it will open on the current working directory.

### Options

* `-p`: Print the path cscroll ends in. Useful for commands like `cd $(cscroll -p)` to cd into the last directory.

### Commands

* `j` or `down arrow key`: Move the cursor down
* `k` or `up arrow key`: Move the cursor up
* `h` or `left arrow key`: Enter the previous directory
* `l` or `right arrow key`: Enter the directory the cursor is currently on
* `g`: Place cursor on first file
* `G`: Place cursor on last file
* `.`: Toggle whether or not to show dot files
* `d`: Delete the file the cursor is on (a prompt will be shown first)
* `o`: Open the file the cursor is on with `xdg-open`
* `q`: Quit


## Installation

cscroll has one dependency: ncurses. To compile cscroll you must install `libncurses`.

On Debian and Ubuntu based system this should be titled `libncurses-dev` and may be installed with `sudo apt install libncurses-dev`.

Then run `make` to compile. The program will then be accessible by running `./cscroll`.

