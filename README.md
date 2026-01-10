# cscroll

A small and efficient TUI file manager

## Usage

cscroll has the following key binds:

* Up/`j`/ctrl+p: Move the cursor up
* Down/`k`/ctrl+n: Move the cursor down
* Left/`h`/ctrl+p: Enter the previous directory
* Right/`l`/ctrl+f: Enter the next directory, or open the current file
* Home/`g`: Go to the first file in the directory
* End/`G`: Go to the last file in the directory
* `.`: Toggle showing/hiding dot files
* `o`: Cycle through long mode viewing options
* `/`: Search for a file using a POSIX extended regex
* `d`: [Delete this file](#file-deletion)
* `q`: Quit cscroll

### File Deletion

When deleting a file, a prompt will always be shown. Cycle through the prompt
options using the left and right key binds (same as above). The prompt can be
exited (without selecting an option) by pressing `q` or ESC.

If the file is either a regular file or an empty directory, cscroll will delete
the file if the prompt is answered "Yes". Otherwise, it will not delete the
file. For non empty directories, the first prompt will show the number of files
inside the directory and, if confirmed, a second prompt will be shown. cscroll
will only delete the directory if the second prompt is also answered "Yes".
