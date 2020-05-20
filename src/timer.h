#pragma once

#include <chrono>

typedef std::chrono::time_point<std::chrono::system_clock> timepoint;

class Timer {
public:
	inline Timer() { start(); }

	inline void start() {
        start_time = std::chrono::system_clock::now();
	}

	inline void restart() { start(); }

	inline double look() {
        return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
                std::chrono::system_clock::now() - start_time).count();
	}

	static double msec(int ms) { return (double)ms; }
	static double sec(int s) { return s*1000.0; }

    // data
	timepoint start_time;
};
