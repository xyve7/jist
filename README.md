# jist
Gets the "jist" of the file.

This is basically the `file` command on steroids.

It supports plugins, allowing you to create your own parser for custom data types.

# Todo
- [x] Auto create directories in $HOME/.config and $HOME/.local/share for config and plugins 
- [x] Search said directories for plugins and load them accordingly
- [x] Identify the file automatically (either an `identify` function or libmagic)
- [x] Print the data in order
- [ ] Write an installer (install libjist)
- [ ] Add a "add plugin" command to the base 'jist' binary (installs the plugins)

# Structure 
# jist 
- `bin` is where the compiled binary is located 
- `obj` is where the object files are before being linked into the final binary in `bin`
- `src` is where the source code is 
- `tests` is where some basic tests are (txt, elf, etc.)
- `lib` is where `libjist` livs 
- `plugins` some default plugins
