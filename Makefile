default:
	g++ src/main.cpp -o Game -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm
	Game