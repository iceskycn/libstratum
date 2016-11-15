//
// Created by Gregory Istratov on 11/15/16.
//


#ifndef LIBSTRATUM_LOG_H
#define LIBSTRATUM_LOG_H

#define LOG_FORMAT_BUFFER_MAX_SIZE 2048

#define LOG(msg, lvl, ...) { _log(msg, __FILE__, __PRETTY_FUNCTION__, __LINE__, lvl, __VA_ARGS__); }
#define LOG_DEBUG(msg, ...) { _log(msg, __FILE__, __PRETTY_FUNCTION__, __LINE__, LOG_DEBUG, ##__VA_ARGS__); }
#define LOG_ERROR(msg, ...) { _log(msg, __FILE__, __PRETTY_FUNCTION__, __LINE__, LOG_ERROR, ##__VA_ARGS__); }
#define LOG_INFO(msg, ...) { _log(msg, __FILE__, __PRETTY_FUNCTION__, __LINE__, LOG_INFO, ##__VA_ARGS__); }

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
void _log(const char* msg, const char* file, const char* fun, int line, int lvl, ...);

#endif //LIBSTRATUM_LOG_H_H
