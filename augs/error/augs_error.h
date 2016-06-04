#pragma once
// legacy macros kept for conformance with old code. Do not use anymore.
#include "build_settings.h"

#define err(expression) expression
#define errf(expression, retflag) retflag = retflag ? int(expression) : 0;
#define errl(expression, errlog) expression
#define errs(expression, str) expression
#define errsf(expression, str, retflag) retflag = retflag ? int(expression) : 0;