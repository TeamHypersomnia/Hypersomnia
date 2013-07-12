#pragma once
// #define INCLUDE_DWM
#define ENABLE_ERRORLOGS /* warning: this module requires error_logging package */
#define ENABLE_EXCEPTIONS

#define MAXIMUM_COMPONENTS 64

/* cancel out error macros unless we want error_logging included */

#ifndef ENABLE_ERRORLOGS

#define err(expression) expression
#define errf(expression, retflag) retflag = retflag ? int(expression) : 0;
#define errl(expression, errlog) expression
#define errs(expression, str) expression
#define errsf(expression, str, retflag) retflag = retflag ? int(expression) : 0;

#else
#include "error\error.h"
#endif