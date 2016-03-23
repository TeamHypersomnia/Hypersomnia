#include "ensure.h"
#include <Windows.h>

void cleanup_proc() {
	ClipCursor(NULL);
	__debugbreak();
}