# Terminal JSON Editor (In-Progress)

A terminal-based JSON editor for Linux systems (including headless environments), designed to allow easy navigation, viewing, and modification of JSON data in a tree-like structure.keywords : json editor tui, ncurses json editor, menuconfig style json editor,terminal ui json tool, linux json editor, cjson editor, config editor tui

---

## Features

- ncurses-based JSON editor
- Menuconfig-style UI
- Edit nested objects/arrays
- Boolean, integer, string editing
- cJSON backend
- Search functionality (coming soon)
- Large JSON file support (planned)

---

## Supported Platforms

- Linux (terminal)
- Linux headless environments
- Potential future support for macOS terminal (TBD)

---

## Usage:

To edit a JSON file from scratch,

```shell
./JSONEditorTUI
```

To edit a already present JSON file(intended usecase),

```shell
./JSONEditorTUI -j <filename>
```

A TUI will be rendered where the directions of usage are given.



You may also utilise the library as a JSON front end editor in your project for ease of use in user end in case they are required to edit json strings to use a particular tool.

---

## Build from source steps:

- pre-requisites - ncurses.Refer the official site of ncurses for your system's support.

- Clone the project into desired path.

- change directory to the JSONEditor folder and then follow the below steps,
  
  ```shell
  $ mkdir build
  $ cd build
  $ cmake ..
  $ make
  ```

- Now you should obtain the binary file "./JSONEditorTUI" specicfic to your system architecture.
