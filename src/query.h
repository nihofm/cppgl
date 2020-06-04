#pragma once

#include <string>
#include <vector>
#include <cfloat>
#include <GL/glew.h>
#include <GL/gl.h>
#include <chrono>
#include "named_map.h"

// -------------------------------------------------------
// (CPU) Timer

class Timer {
public:
	inline Timer() { start(); }

	inline void start() {
        start_time = std::chrono::system_clock::now();
	}

	inline double look() { // milliseconds
        return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
                std::chrono::system_clock::now() - start_time).count();
	}

    // data
	std::chrono::time_point<std::chrono::system_clock> start_time;
};

// -------------------------------------------------------
// Ring buffer

template <typename T> struct RingBuffer {
    RingBuffer(size_t N) : N(N), curr(0), data(N, T(0)) {}

    void put(const T& val) {
        data[curr] = val;
        curr = (curr + 1) % N;
        const float f = 0.1f;
        exp_avg = f * val + (1 - f) * exp_avg;
    }

    T min() const {
        T t(FLT_MAX);
        for (const auto& val : data)
            t = std::min(t, val);
        return t;
    }

    T max() const {
        T t(FLT_MIN);
        for (const auto& val : data)
            t = std::max(t, val);
        return t;
    }

    T avg() const {
        T t(0);
        for (const auto& val : data)
            t += val;
        return t / N;
    }

    // data
    const size_t N;
    size_t curr;
    std::vector<T> data;
    T exp_avg;
};

// -------------------------------------------------------
// CPU timer query

class TimerQuery : public NamedMap<TimerQuery> {
public:
    TimerQuery(const std::string& name, size_t samples = 256);
    virtual ~TimerQuery();

    void start();
    void end();
    float get() const; // milliseconds

    Timer timer;
    RingBuffer<float> buf;
    std::chrono::time_point<std::chrono::system_clock> start_time;
};

// -------------------------------------------------------
// GPU timer query

class TimerQueryGL : public NamedMap<TimerQueryGL> {
public:
    TimerQueryGL(const std::string& name, size_t samples = 256);
    virtual ~TimerQueryGL();

    // prevent copies and moves, since GL buffers aren't reference counted
    TimerQueryGL(const TimerQueryGL&) = delete;
    TimerQueryGL& operator=(const TimerQueryGL&) = delete;
    TimerQueryGL& operator=(const TimerQueryGL&&) = delete;

    void start();
    void end();
    float get() const; // milliseconds

    RingBuffer<float> buf;
    GLuint query_ids[2][2];
    GLuint64 start_time, stop_time;
};
