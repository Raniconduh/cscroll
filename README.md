# cscroll

A small file manager written in C.


## Usage

If an argument is provided, cscroll will open the path supplied. Otherwise it will open on the current working directory.

Files will be highlighted and shown with an identifier in correspondence to the file type.

#### Colors & Identifiers:

* Red, `?`: Unknown File
* Yellow, `#`: Block or char device
* Yellow, `|`: FIFO (named pipe)
* Green, `*`: File is executable
* Blue, `/`: Directory
* Cyan, `@`: Symbolic link
* Magenta, `=`: Unix socket
* White, No identifier: Regular file

Files that are are executable but have another identifier will keep the identifier but be colored green. Symbolic links that point to directories will be suffixed with `@ => /` and may be entered as a normal directory. Otherwise, deletion of a symbolic link will not delete whatever the link points to; only the link itself and opening one will open what the link points to.

### Options

* `-p`: Print the path cscroll ends in. Useful for commands like `cd $(cscroll -p)` to cd into the last directory.

### Commands

* `j`, `Ctrl+n` or `down arrow key`: Move the cursor down
* `k`, `Ctrl+p` or `up arrow key`: Move the cursor up
* `h`, `Ctrl+b` or `left arrow key`: Enter the previous directory
* `l`, `Ctrl+f`, `right arrow key`, or `enter key`: If the file the cursor is on is a directory, enter that directory. Otherwise open the file with `xdg-open`
* `g`: Place cursor on first file
* `G`: Place cursor on last file
* `.`: Toggle whether or not to show dot files
* `d`: Delete the file the cursor is on (a prompt will be shown first)
* `m`: Mark the file the cursor is on
* `:`: Open a commands prompt ([see 'Command Prompt' section](#command-prompt))
* `q`: Quit

#### Command Prompt

The command prompt will show up upon pressing `:` and the prompt itself is prefixed with a colon. Here you may enter multi-character commands. Available commands are:

* `ma`: **M**ark **A**ll files in the directory
* `mu`: **M**ark **U**nmark: Unmarks all files on the directory

## Installation

cscroll has one dependency: ncurses. To compile cscroll you must install `libncurses`.

On Debian and Ubuntu based system this should be titled `libncurses-dev` and may be installed with `sudo apt install libncurses-dev`.

Then run `make` to compile. The program will then be accessible by running `./cscroll`.

