#pragma once
// #define INCLUDE_DWM
#define ENABLE_ERRORLOGS /* warning: this module requires error_logging package */

/* cancel out error macros unless we want error_logging included */

#ifndef ENABLE_ERRORLOGS

#define err(expression) expression
#define errf(expression, retflag) retflag = retflag ? int(expression) : 0;
#define errl(expression, errlog) expression
#define errs(expression, str) expression
#define errsf(expression, str, retflag) retflag = retflag ? int(expression) : 0;

#else
#include "error\augs_error.h"
#endif