# cscroll

A small and efficient TUI file manager.

For usage, see [Usage](#usage). For building, see [Building](#building).

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
* `m`: Mark a file
* `c`: Cut marked files
* `p`: [Paste](#file-pasting) all cut files into the current directory
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

### File Pasting

When cut files are pasted, first a confirmation prompt is shown. Then, the cut
files will be moved into the current directory. Pasting may fail for various
reasons:

1. The paste directory cannot be opened for some reason. This is rare and should
   only happen if the current directory is removed in between opening it in
   cscroll and pasting the files.
2. One of the origin (cut) directories cannot be opened.
3. One of the cut files no longer exists.
4. Pasting would otherwise overwrite an already existing file.

In the case of 2 through 4, cscroll will skip the error causing file/directory
and continue trying to paste the others. An error message will be shown after
pasting finishes, notifying the user that some files were skipped.

cscroll will never attempt to overwrite a file by pasting. If this is behavior
you want, you need to manually remove the conflicting file first. If two marked
files have the same name, only one of the files will be pasted assuming the name
does not conflict with any existing files in the paste directory. The paste
order is arbitrary. I.e. if there are multiple marks with the same name, the one
which is pasted can not be determined beforehand.

## Building

cscroll requires libncurses. cscroll can be compiled using any C99 or more
recent C compiler and requires at least POSIX.1-2008. This means that
compilation with a modern C compiler and libc will work with at least
`-D_POSIX_C_SOURCE=200809L` and `-std=c99`. Note, however, that POSIX.1-2008
does not include the "sticky" bit in file modes. This mode bit is platform
dependent and is rarely used, but can be included in cscroll using the compiler
flag `-D_XOPEN_SOURCE=700`. (Note that `_XOPEN_SOURCE = 700` implies
`_POSIX_C_SOURCE = 200809L`.)

cscroll can be built using `make`. On some systems, linking against libncurses
may fail. If your system has `libncursesw.so` but not `libncurses.so`, modify
the makefile to link against libncursesw instead. If you do not know what this
means, run `sed -i 's/-lncurses/-lncursesw/g' Makefile` and try `make` again.
