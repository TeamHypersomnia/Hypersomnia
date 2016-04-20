#pragma once
#define ENABLE_ERRORLOGS 1

#if !ENABLE_ERRORLOGS

#define err(expression) expression
#define errf(expression, retflag) retflag = retflag ? int(expression) : 0;
#define errl(expression, errlog) expression
#define errs(expression, str) expression
#define errsf(expression, str, retflag) retflag = retflag ? int(expression) : 0;

#else
#include "error\augs_error.h"
#endif

extern bool INTERPOLATE_DEBUG_LINES;
extern bool DEBUG_LOG_CREATED_AND_DELETED_ENTITIES;
