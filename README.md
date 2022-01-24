# cscroll

A small and efficient file manager written in C.

## Usage

If an argument is provided, cscroll will open the path supplied. Otherwise it will open in the current working directory. ([see 'Options'](#options))

Files will be highlighted and shown with an identifier in correspondence to the file type. If compiled with icons and the icons option is set, a nerd icons corresponding the file file type will be shows to its left.

Default configurations can be specified in the [config file](#config-file). These will be overwritten by any flags passed or through the `set` command but will persist in the file.

#### Colors & Identifiers:

* Red, `?`: Unknown File
* Yellow, `#`: Block or char device
* Yellow, `|`: FIFO (named pipe)
* Green, `*`: File is executable
* Blue, `/`: Directory
* Cyan, `@`: Symbolic link
* Magenta, `=`: Unix socket
* Magenta, No identifier: Media file
* Red, No identifier: Archive or compressed file
* White, No identifier: Regular file

Files that are executable but have another identifier will keep the identifier but be colored green. Files that are either media files or archives will be colored respectively but will keep an identifier if they have one.

Symbolic links that point to directories will be suffixed with `@ => /` and may be entered as a normal directory. Otherwise, deletion of a symbolic link will not delete whatever the link points to; only the link itself and opening one will open what the link points to.

### Options

* `-p`: Print the path cscroll ends in. Useful for commands like `cd $(cscroll -p)` to cd into the last directory
* `-nc`: Turn off coloring. cscroll will run in black and white mode
* `-ni`: (If compiled with icons) turn off icons on start
* `-l`: Display files in long mode (file mode, owner, group, size, modification time)
* `-h`, `--help`: Show the help screen

### Commands

* `j`, `Ctrl+n` or `down arrow key`: Move the cursor down
* `k`, `Ctrl+p` or `up arrow key`: Move the cursor up
* `h`, `Ctrl+b` or `left arrow key`: Enter the previous directory
* `l`, `Ctrl+f`, `right arrow key`, or `enter key`: If the file the cursor is on is a directory, enter that directory. Otherwise open the file with `xdg-open`
* `g`: Place cursor on first file
* `G`: Place cursor on last file
* `.`: Toggle whether or not to show dot files
* `d`: Delete the file the cursor is on (a [prompt](#options-prompt) will be shown first)
* `m`: Mark the file the cursor is on
* `r`: Rename the file the cursor is on ([see file renaming](#renaming))
* `c`: Cut the file the cursor is currently on or all the marked files. Pressing twice in the same directory will cancel the cut
* `p`: Paste all cut files into the current directory. Pasting in the same directory where the cut originated will cancel the cut
* `:`: Open a commands prompt ([see 'Command Prompt' section](#command-prompt))
* `/`: Search for a file in the current directory using a POSIX regex
* `!`: Run a shell command ([see shell commands](#shell-commands))
* `q`: Quit

#### Options Prompt

An options prompt will pop up in the center of the screen with text at the top of the pop-up and one or more options at the bottom. To move the cursor to the left, any of the left or up keys will work. To move it to the right, any of the down or right keys (except for enter) will work. To select the current option, press either the space bar or the enter key. The `q` key will quit the prompt without selecting an option.

#### Command Prompt

The command prompt will show up upon pressing `:` and the prompt itself is prefixed with a colon. Here you may enter multi-character commands. Available commands are:

* `ma`: **M**ark **A**ll files in the directory
* `mu`: **M**ark **U**nmark: Unmarks all files on the directory
* `ca`: **C**ut **A**ll files in the current directory

* `set`: Set a variable ([see Variables](#variables))
* `unset`: Unset a variable ([see Variables](#variables))

#### Renaming

The `r` command will show a prompt (similar to a command prompt but without a prefix) where the new file name is to be expected. A file may only be renamed within the same directory.

E.g. if the directory cscroll is in is `/home/user/downloads` and the file `image.png` is renamed to `/home/user/image.png`, it will fail. The file can only be renamed to something like `my_image.png`. (Attempting to move a file across directories is not possible with the rename function.) Mass renaming is not possible.

#### Shell Commands

Pressing `!` will open a prompt prefixed with `!` which will run the shell command entered into it. Entering `%f` will format the command entered with the name of the file the cursor is currently on. To escape this format (i.e. to not have it be replaced with the file name), enter `%%f`. The output will be literally `%f`.

E.g. `vim %f` will format to `vim FILE` where `FILE` is the name of the file the cursor is on. `echo %%f`, however, will format to `echo %f` and the output of the command will literally be `%f`.

#### Config File

The config file allows for the specification of default variables cscroll will always set. The default location of the file will be `$HOME/.config/cscroll/config`, but the default configuration directory can be changed with the `XDG_CONFIG_HOME` environment variable.

The general syntax for configurations is `variable = value`. The variables that may be specified are the same as the ones the `set` command may take and are specified [here](#variables).

The values that variables may be set to are limited to `true` and `false`

Example config file:

```
long = true
color = false
icons = false
```

This will, by default, turn on long mode but turn off colors and icons. However, variables can still be set and unset when in cscroll itself. To turn off long listing mode once again, for example, one could run this command in cscroll: `:unset long`.

Variables specified in the configuration file will also be overwritten by command line arguments to cscroll. If the configuration specifies to show icons, the `-ni` flag will turn them off regardless. Configuration file values are of the least significance in regards to all other ways to set variables, although they will last indefinitely and will work for each run.

#### Variables

Variables allow changing settings while in cscroll itself as opposed to having to stop and restart it with different flags.

* `color`: Turn on or off colors.
* `icons`: (If compiled with icons) If true, show icons. Otherwise don't.
* `long`: Turn on or off long mode

## Compilation

To run, cscroll requires libncurses and libterminfo. Compilation, however, also requires pkg-config.

On Debian and Ubuntu based system, libncurses may be titled `libncurses-dev` or simply `libncurses` and can be installed with `sudo apt install libncurses{,-dev}`.

To compile, simply run `make`. Nerd icons are enabled by default. To turn them off, compile with `ICONS=0`.

The program will then be accessible by running `./cscroll`.

## Installation

Run `make install` to install the binary to the directory pointed at by the set PREFIX and DESTDIR.

