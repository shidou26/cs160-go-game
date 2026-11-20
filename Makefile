# AI-assisted: see AI01, AI30

CXX ?= g++
SRC = src/main.cpp src/ui.cpp src/board.cpp src/zobrist_hash.cpp src/rule.cpp src/score.cpp src/cpu.cpp src/audio.cpp
CXXFLAGS = -Iinclude -std=c++17

ifeq ($(OS),Windows_NT)
default: windows
else
UNAME_S ?= $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
ifeq ($(origin CXX),default)
CXX = clang++
endif
default: macos
else
default: linux
endif
endif

.PHONY: default windows linux macos

windows:
	$(CXX) $(SRC) -o Game.exe $(CXXFLAGS) -Llib -lraylib -lopengl32 -lgdi32 -lwinmm
	Game.exe

linux:
	$(CXX) $(SRC) -o Game $(CXXFLAGS) $$(pkg-config --cflags --libs raylib)
	./Game

# macOS requires the Xcode Command Line Tools and raylib discoverable by pkg-config.
macos:
	$(CXX) $(SRC) -o Game $(CXXFLAGS) $$(pkg-config --cflags --libs raylib)
	./Game
