#ifndef LOCALTIME_H__
#define LOCALTIME_H__

#include <api/libc/time.h>

struct tm *localtime(time_t time);

#endif