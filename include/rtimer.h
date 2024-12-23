#ifndef __RTIMER_H__
#define __RTIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(RT_MALLOC) || !defined(RT_REALLOC) || !defined(RAYLIB_H)
    #include <stdlib.h>
#endif

#ifndef RT_ERROR
    #if defined(RAYLIB_H)
        #define RT_ERROR(fmt, ...) TraceLog(LOG_ERROR, "%s:%d - " fmt, __FILE__, __LINE__, __VA_ARGS__);
    #else
        #include <stdio.h>
        #define RT_ERROR(fmt, ...) fprintf(stderr, "%s:%d; " fmt, __FILE__, __LINE__, __VA_ARGS__);
    #endif
#endif

#ifndef RT_MALLOC
    #define RT_MALLOC malloc
#endif

#ifndef RT_REALLOC
    #define RT_REALLOC realloc
#endif

#if !defined(bool)
    #include <stdbool.h>
#endif

#ifndef TIMERDISPATCH_REALLOC_SAFENET
    /* how much more Timers to allocate per realloc */
    #define TIMERDISPATCH_REALLOC_SAFENET 1
#endif

/* ----------- typedefs ----------- */

typedef void (*TimerCallback)(void*);

#if defined (RT_NO_REALLOC)
    typedef const int MaxTimerCount;
#else
    typedef int MaxTimerCount;
#endif

typedef struct Timer Timer;
typedef struct TimerDispatch TimerDispatch;

/* -------------------------------- */

/* ------------ structs ------------ */
enum TimerFlags {
    TF_DEFAULT  = 0,      /* synchronous, continuous */
    TF_TIMEBOMB = 1 << 0, /* destroyed after firing  */
 /* TF_ASYNC    = 1 << 1, */ /* async (WIP)             */
};

struct Timer {
    enum TimerFlags tf;     /* bitwise timer flags             */
    bool paused;            /* whether this timer is paused    */
    bool active;            /* whether this timer is active    */
    float duration;         /* timer duration                  */
    float elapsed;          /* elapsed time                    */
    TimerCallback callback; /* callback function               */
    void* data;             /* passed to the callback function */
};

struct TimerDispatch {
    Timer* timers;                /* timers array                        */
    int num_timers;               /* num of elements in timers array     */
    MaxTimerCount max_num_timers; /* max num of elements in timers array */
};

/* --------------------------------- */

#ifdef    RT_IMPLEMENTATION


TimerDispatch InitTimerDispatch(MaxTimerCount max_num_timers) {
    TimerDispatch res = {
        .num_timers = 0,
        .max_num_timers = max_num_timers + 1,
    };

    res.timers = (Timer*)RT_MALLOC(sizeof(Timer) * res.max_num_timers);

    return res;
}


int AddTimer(TimerDispatch* td, float duration, TimerCallback callback, void* data, enum TimerFlags timer_flags, bool paused_by_default) {
    if (td->num_timers >= td->max_num_timers) {
#if defined (RT_NO_REALLOC)
        RT_ERROR("couldn't add timer (%f; %d; %d), td->num_timers >= td->max_num_timers", duration, timer_flags, paused_by_default);
        return -1;
#else
        td->max_num_timers = (td->num_timers + TIMERDISPATCH_REALLOC_SAFENET);
        td->timers = (Timer*)RT_REALLOC(td->timers, sizeof(Timer) * td->max_num_timers);
#endif
    }

    td->timers[td->num_timers].paused = paused_by_default;
    td->timers[td->num_timers].active = true;
    td->timers[td->num_timers].tf = timer_flags;
    td->timers[td->num_timers].duration = duration;
    td->timers[td->num_timers].elapsed = 0.0f;
    td->timers[td->num_timers].callback = callback;
    td->timers[td->num_timers].data = data;
    return td->num_timers++;
}


void RemoveTimer(TimerDispatch *td, int index) {
    if (index < 0 || index >= td->num_timers) return;
    td->timers[index].active = false;
}


/* !! WARNING: shifts timer IDs in the td->timers array !! */
void RippleCutTimer(TimerDispatch *td, int index) {
    if (index < 0 || index >= td->num_timers) return;
    for (int i = index; i < td->num_timers - 1; i++) {
        td->timers[i] = td->timers[i + 1];
    }
    td->num_timers--;
}


/* returns the current timer_id adjusted to ripplecut timer array */
int GetTimerIDAfterRippleCut(int current_timer_id, int cut_id) {
    if (current_timer_id < cut_id) return current_timer_id;
    else return current_timer_id - 1;
}


void UpdateTimers(TimerDispatch *td, float deltaTime) {
    for (int i = 0; i < td->num_timers; ++i) {
        if (td->timers[i].paused || !td->timers[i].active) continue;

        td->timers[i].elapsed += deltaTime;
        if (td->timers[i].elapsed < td->timers[i].duration) continue;

        td->timers[i].callback(td->timers[i].data);
        td->timers[i].elapsed = 0.0f;
        if (td->timers[i].tf & TF_TIMEBOMB) RemoveTimer(td, i);
    }
}


bool IsTimerPaused(TimerDispatch *td, int timer_id) {
    return td->timers[timer_id].paused;
}


void PauseTimer(TimerDispatch *td, int timer_id) {
    td->timers[timer_id].paused = true;
}


void ResumeTimer(TimerDispatch *td, int timer_id) {
    td->timers[timer_id].paused = false;
}


void ResetTimer(TimerDispatch *td, int timer_id, float duration, enum TimerFlags timer_flags, bool paused_by_default) {
    if (timer_id < 0 || timer_id >= td->num_timers) return;

    td->timers[timer_id].paused = paused_by_default;
    td->timers[timer_id].active = true;
    td->timers[timer_id].tf = timer_flags;
    td->timers[timer_id].duration = duration;
    td->timers[timer_id].elapsed = 0.0f;
}


#endif /* RT_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif
