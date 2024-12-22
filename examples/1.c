#include <raylib.h>

#define RT_IMPLEMENTATION
#include "rtimer.h"

void TimerCallback1(void* data) {
    int* intData = (int*)data;
    TraceLog(LOG_INFO, "Timer 1 expired! Data: %d", *intData);
}

void TimerCallback2(void* data) {
    const char* strData = (const char*)data;
    TraceLog(LOG_INFO, "Timer 2 expired! Message: %s", strData);
}

int main(void) {
    InitWindow(800, 600, "Timer Example");
    SetTargetFPS(60);

    TimerDispatch td = InitTimerDispatch(0);

    int timer1Data = 42;
    const char* timer2Data = "This timer fires only once!";
    const char* timer3Data = "This timer fires only once as well!";

    AddTimer(&td, 1.0f, TimerCallback1, &timer1Data, TF_DEFAULT);
    AddTimer(&td, 1.5f, TimerCallback2, (void*)timer2Data, TF_DEFAULT | TF_TIMEBOMB);
    AddTimer(&td, 2.5f, TimerCallback2, (void*)timer3Data, TF_DEFAULT | TF_TIMEBOMB);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateTimers(&td, deltaTime);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Check the console for timer messages!", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
