# CS160 Project: Go game
--- 

Repository: https://github.com/shidou26/cs160-go-game

Go Game Final Project for CS160 Course

## Introduction
By Đoàn Nhật Anh and Lê Hồng Anh

## Features

### Game Modes & AI
- **Player vs. Player (PvP):** Local 2-player mode (*Play against Friend*) alternating turns between Black and White.
- **Player vs. AI (PvE):** Single-player mode (*Play against Seol*) with three difficulty levels:
  - **Easy:** Plays randomized legal moves while avoiding filling its own eyes.
  - **Medium:** Uses shallow Minimax search with slight evaluation noise.
  - **Hard:** Uses Minimax with Alpha-Beta pruning, move ordering, and liberty/atari heuristics.
  - Automatically passes when no beneficial or legal moves are available.

### Go Rules & Scoring
- **Multiple Board Sizes:** Switchable between **9×9**, **13×13**, and **19×19** grids.
- **Capture & Liberty Detection:** Automatic group capture detection using Breadth-First Search (BFS), with live captured stone counters for both Black and White.
- **Suicide Prevention:** Illegal suicide moves (placing a stone with zero liberties that does not capture opponent stones) are blocked.
- **Ko Rule Enforcement:** Prevents immediate board repetition using Zobrist Hashing and marks the restricted Ko intersection with a visual red square indicator.
- **Pass & End-Game Scoring:** Players can pass their turn; two consecutive passes end the match and trigger territory + capture + Komi (`6.5`) scoring.

### Match Controls & Persistence
- **Move History:** Full **Undo** and **Redo** support, plus a **Reset** button to restart the board.
- **Save & Load System:** Save and load matches across **3 JSON slots** (`saves/slot_1.json` – `saves/slot_3.json`), complete with overwrite confirmation prompts.

### Graphics, UI & Display
- **Raylib GUI:** Clean menu flow including Start Menu (`NEW GAME`, `LOAD GAME`, `SETTINGS`), Mode/Difficulty selection, In-Game toolbar, and Game Over summary screen.
- **High-Definition Visuals:**
  - Procedural **512×512** 3D glossy Black & White stone textures with Mipmaps and Trilinear filtering.
  - Procedural **1024×1024** natural wood-grain board texture, engraved grid lines, Star Points (*Hoshi*), and **MSAA 4x** anti-aliasing.
  - Crisp custom TrueType font rendering.
- **Settings & Responsive Scaling:**
  - Dedicated **Settings** screen accessible from both the Start Menu and in-game toolbar.
  - Live switching between **Windowed** and **Fullscreen** display modes.
  - Virtual **`1280×960`** render target scaling that keeps the board and UI centered and proportional across aspect ratios.
  - Sound volume slider UI placeholder (audio playback not yet implemented).

## Build & Run

Built with **C++** and **Raylib** (`Makefile` supports both Windows and Linux):

```bash
# Build
make

# Run on Windows
Game.exe

# Run on Linux
./Game
```
# Technology Stack

# Credit