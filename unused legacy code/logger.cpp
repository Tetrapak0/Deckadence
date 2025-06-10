#ifndef _DEBUG // TODO: Check for _PROD instead
#include "../../include/Header.hpp"

#include <ctime>
#include <chrono>

const char* log_categories[] = {"NET.INIT", "NET.SOCK", "NET.COMM",
                                "GUI.INIT", "GUI.DRAW",
                                "CFG.READ", "CFG.SAVE",};

const char* log_levels[] = {"INFO", "NOTE", "WARN", "ERROR", "CRITICAL"};

void __log(FILE* stream, char lvl, char category, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    const auto now = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(now);

    fprintf(stream, "[%s] [%s] [%s]: \n", std::ctime(&t_c),
                                          log_levels[lvl],
                                          log_categories[category]);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");

    va_end(args);
}


#endif
