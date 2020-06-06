#include "query.h"

// -------------------------------------------------------
// CPU timer query

TimerQueryImpl::TimerQueryImpl(size_t samples) : buf(samples) {}

TimerQueryImpl::~TimerQueryImpl() {}

void TimerQueryImpl::start() {
    timer.start();
}

void TimerQueryImpl::end() {
    buf.put(timer.look());
}

float TimerQueryImpl::get() const {
    return buf.avg();
}

// -------------------------------------------------------
// GPU timer query

TimerQueryGLImpl::TimerQueryGLImpl(size_t samples) : buf(samples) {
    glGenQueries(2, query_ids[0]);
    glGenQueries(2, query_ids[1]);
    glQueryCounter(query_ids[1][0], GL_TIMESTAMP);
    glQueryCounter(query_ids[1][1], GL_TIMESTAMP);
}

TimerQueryGLImpl::~TimerQueryGLImpl() {
    glDeleteQueries(2, query_ids[0]);
    glDeleteQueries(2, query_ids[1]);
}

void TimerQueryGLImpl::start() {
    glQueryCounter(query_ids[0][0], GL_TIMESTAMP);
}

void TimerQueryGLImpl::end() {
    glQueryCounter(query_ids[0][1], GL_TIMESTAMP);
    std::swap(query_ids[0], query_ids[1]); // switch front/back buffer
    glGetQueryObjectui64v(query_ids[0][0], GL_QUERY_RESULT, &start_time);
    glGetQueryObjectui64v(query_ids[0][1], GL_QUERY_RESULT, &stop_time);
    buf.put((stop_time - start_time) / 1000000.0);
}

float TimerQueryGLImpl::get() const {
    return buf.avg();
}
