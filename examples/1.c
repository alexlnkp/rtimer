#include <raylib.h>

#define RT_IMPLEMENTATION
#include "rtimer.h"

typedef struct ColorHandler {
    Color col;
    int type;
} ColorHandler;

void TimerCallback1(void* data) {
    ColorHandler *handler = data;
    TraceLog(LOG_INFO, "Fired timer1! type: %d, color: {%u, %u, %u, %u}",
        handler->type, handler->col.r, handler->col.g, handler->col.b, handler->col.a);
    if (handler->type == 1) {
        handler->col = (Color) { 0, 0, 255, 255 };
        handler->type = 0;
    } else {
        handler->col = (Color) { 255, 0, 0, 255 };
        handler->type = 1;
    }
}

int main(void) {
    InitWindow(800, 600, "Timer Example");
    SetTargetFPS(60);

    TimerDispatch td = InitTimerDispatch(1);

    ColorHandler ch = {
        .col = BLACK,
        .type = 0
    };

    AddTimer(&td, 1.0f, TimerCallback1, &ch, TF_DEFAULT);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateTimers(&td, deltaTime);

        BeginDrawing();
        ClearBackground(ch.col);
        DrawText("Check the console for timer messages!", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
