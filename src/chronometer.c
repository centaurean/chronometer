/*
 * Centaurean Chronometer
 *
 * Copyright (c) 2015, Guillaume Voirin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     1. Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *
 *     3. Neither the name of the copyright holder nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 3/05/15 17:07
 */

#include "chronometer.h"

#if defined(_WIN64) || defined(_WIN32)
void usage_to_timeval(FILETIME *ft, struct timeval *tv) {
    ULARGE_INTEGER time;
    time.LowPart = ft->dwLowDateTime;
    time.HighPart = ft->dwHighDateTime;

    tv->tv_sec = time.QuadPart / 10000000;
    tv->tv_usec = (time.QuadPart % 10000000) / 10;
}

int getrusage(int who, struct rusage *usage) {
    FILETIME creation_time, exit_time, kernel_time, user_time;
    memset(usage, 0, sizeof(struct rusage));

    if (who == RUSAGE_SELF) {
        if (!GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time))
            return -1;
        usage_to_timeval(&kernel_time, &usage->ru_stime);
        usage_to_timeval(&user_time, &usage->ru_utime);
        return 0;
    } else
        return -1;
}
#endif

double chronometer_timeval_to_time(struct timeval tv) {
    return tv.tv_sec * CHRONOMETER_MICROSECONDS + tv.tv_usec;
}

double chronometer_substract(struct timeval stop, struct timeval start) {
    return (chronometer_timeval_to_time(stop) - chronometer_timeval_to_time(start)) / CHRONOMETER_MICROSECONDS;
}

CHRONOMETER_WINDOWS_EXPORT struct timeval chronometer_get_current_time() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime;
}

CHRONOMETER_WINDOWS_EXPORT void chronometer_start(chronometer *chrono) {
    chrono->state = CHRONOMETER_STATE_RUNNING;
    chrono->start = chronometer_get_current_time();
    chrono->lap = chrono->start;
}

CHRONOMETER_WINDOWS_EXPORT double chronometer_lap(chronometer *chrono) {
    if (chrono->state == CHRONOMETER_STATE_RUNNING) {
        struct timeval new_lap = chronometer_get_current_time();
        double elapsed = chronometer_substract(new_lap, chrono->lap);
        chrono->lap = new_lap;
        return elapsed;
    } else
        return 0.0;
}

CHRONOMETER_WINDOWS_EXPORT double chronometer_stop(chronometer *chrono) {
    if (chrono->state == CHRONOMETER_STATE_RUNNING) {
        chrono->stop = chronometer_get_current_time();
        chrono->state = CHRONOMETER_STATE_IDLE;
        return chronometer_substract(chrono->stop, chrono->start);
    } else
        return 0.0;
}

CHRONOMETER_WINDOWS_EXPORT double chronometer_elapsed(chronometer *chrono) {
    if (chrono->state == CHRONOMETER_STATE_RUNNING)
        return chronometer_substract(chronometer_get_current_time(), chrono->start);
    else
        return 0.0;
}