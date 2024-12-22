#ifndef __RTIMER_H__
#define __RTIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(RT_MALLOC) || !defined(RT_REALLOC) || !defined(TraceLog)
    #include <stdlib.h>
#endif

#ifndef RT_ERROR
    #if defined(TraceLog)
        #define RT_ERROR(x) TraceLog(LOG_ERROR, "%s: %s:%d", x, __FILE__, __LINE__);
    #else
        #define RT_ERROR(x) perror("%s: %s:%d", x, __FILE__, __LINE__);
    #endif
#endif

#ifndef RT_MALLOC
    #define RT_MALLOC malloc
#endif

#ifndef RT_REALLOC
    #define RT_REALLOC realloc
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
    TF_ASYNC    = 1 << 0, /* async                   */
    TF_TIMEBOMB = 1 << 1  /* destroyed after firing  */
};

struct Timer {
    enum TimerFlags tf;     /* timer flags (sync/async...)     */
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
        .timers = (Timer*)RT_MALLOC(sizeof(Timer) * (max_num_timers + 1)),
        .num_timers = 0,
        .max_num_timers = max_num_timers,
    };

    return res;
}


void AddTimer(TimerDispatch* td, float duration, TimerCallback callback, void* data, enum TimerFlags timer_flags) {
    if (td->num_timers > td->max_num_timers) {
#if defined (RT_NO_REALLOC)
        RT_ERROR("td->num_timers > td->max_num_timers!");
        return;
#else
        td->max_num_timers = (td->num_timers + TIMERDISPATCH_REALLOC_SAFENET);
        td->timers = (Timer*)RT_REALLOC(td->timers, sizeof(Timer) * td->max_num_timers);
#endif
    }

    td->timers[td->num_timers].tf = timer_flags;
    td->timers[td->num_timers].duration = duration;
    td->timers[td->num_timers].elapsed = 0.0f;
    td->timers[td->num_timers].callback = callback;
    td->timers[td->num_timers].data = data;
    td->num_timers++;
}


void RemoveTimer(TimerDispatch *td, int index) {
    if (index < 0 || index >= td->num_timers) return;
    for (int i = index; i < td->num_timers - 1; i++) {
        td->timers[i] = td->timers[i + 1];
    }
    td->num_timers--;
}


void UpdateTimers(TimerDispatch *td, float deltaTime) {
    for (int i = 0; i < td->num_timers; ++i) {
        td->timers[i].elapsed += deltaTime;
        if (td->timers[i].elapsed < td->timers[i].duration) continue;

        td->timers[i].callback(td->timers[i].data);
        td->timers[i].elapsed = 0.0f;
        if (td->timers[i].tf & TF_TIMEBOMB) RemoveTimer(td, i);
    }
}

#endif /* RT_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif
