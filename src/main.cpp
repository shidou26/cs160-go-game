#include <raylib.h>

int main() {
    // init app
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1280, 700, "The Game of Go");
    SetTargetFPS(60);
    
    // run app
    while(!WindowShouldClose()) {
        BeginDrawing();
        


        EndDrawing();
    }

    // close app
    CloseWindow();

    return 0;
}