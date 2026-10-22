# AI-assisted

CXX ?= g++
SRC = src/main.cpp src/ui.cpp
CXXFLAGS = -Iinclude -std=c++17

ifeq ($(OS),Windows_NT)
default: windows
else
default: linux
endif

.PHONY: default windows linux

windows:
	$(CXX) $(SRC) -o Game.exe $(CXXFLAGS) -Llib -lraylib -lopengl32 -lgdi32 -lwinmm
	Game.exe

linux:
	$(CXX) $(SRC) -o Game $(CXXFLAGS) $$(pkg-config --cflags --libs raylib)
	./Game
