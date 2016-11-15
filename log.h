//
// Created by Gregory Istratov on 11/15/16.
//


#ifndef LIBSTRATUM_LOG_H
#define LIBSTRATUM_LOG_H

#define LOG(msg, lvl) { _log(msg, __FILE__, __PRETTY_FUNCTION__, __LINE__, lvl); }
#define LOG_DEBUG(msg) LOG(msg, LOG_DEBUG)
#define LOG_ERROR(msg) LOG(msg, LOG_ERROR)
#define LOG_INFO(msg) LOG(msg, LOG_INFO);

enum {
    LOGLEVEL_NONE = 0,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG
};

enum {
    LOG_ERROR = 0,
    LOG_INFO,
    LOG_DEBUG

};

void init_log(int loglvl);
void _log(const char* msg, const char* file, const char* fun, int line, int lvl);

#endif //LIBSTRATUM_LOG_H_H
