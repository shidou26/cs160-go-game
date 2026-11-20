# CS160 Project: Go game

C++ final project for CS160 - Introduction to Computer Science.

## Introduction
This project is the Go game written in C++ language, with usage of Raylib library.

## Features
The game currently supports two modes:
- Player vs. Player
- Player vs. Computer with three difficulty levels: Easy, Medium, and Hard

Notable features include:
- Adjustable board sizes: 9×9, 13×13, and 19×19
- Undoing and redoing moves
- Saving and loading games
- Background music selection
- Audio control
- Autosave current settings

## Dependencies

### Applications
- C++ 17 (GNU GCC 14.2.0)
- GNU Make
- [Raylib](https://github.com/raysan5/raylib)
- [JSON for Modern C++](https://github.com/nlohmann/json) 

## Building

1. Ensure GNU Make and raylib are installed on your platform before building. The Makefile is included in this repository, and the Windows raylib library is bundled.

2. Clone the repository to your machine
```sh
git clone https://github.com/shidou26/cs160-go-game.git
cd cs160-go-game
```

3. Build and run the game
```sh
make
mingw32-make //if GNU Make is installed as `mingw32-make` on Windows
```

The Makefile creates `Game.exe` on Windows or `Game` on macOS/Linux in the project root and launches it automatically. To run it again, execute `.\Game.exe` in Windows PowerShell or `./Game` on macOS/Linux from the project root.

## Demo

## Credit
- [Raylib](https://github.com/raysan5/raylib)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- Background music: 
    - Song 1: Mizuiro - Go Sakabe
    - Song 2: Atatakai Hi - Kei Wakakusa
    - Song 3: Hateshinaki Tabiji - Hideki Taniuchi

The music and imagery displayed in this game are not made or owned by me. All rights are reserved to their respective owners and no copyright infringement is intended.
