/**
 * @file rbf_dbg_log.c
 * @author Jio (hedajun@hzdusun.com)
 * @brief 
 * @version 0.1
 * @date 2025-01-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "rbf_time.h"

char *dbg_log_get_time_str()
{
    static char acBuf[64];
    rbf_time_t time;

    rbf_time_get_ms(&time);
    snprintf(acBuf, sizeof(acBuf),
         "%04d:",
          time);
          
    return acBuf;
}