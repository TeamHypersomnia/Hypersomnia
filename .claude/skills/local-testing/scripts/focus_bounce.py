#!/usr/bin/env python3
# Takes the X input focus away from a window and gives it back - as alt-tabbing to another program and back does
# (Xvfb has no window manager, so nothing else ever moves the focus). The game sees FocusOut, then FocusIn.
# Usage: DISPLAY=:77 focus_bounce.py <window name substring> [seconds away]
import ctypes, subprocess, sys, time

name = sys.argv[1]
away = float(sys.argv[2]) if len(sys.argv) > 2 else 1.0

tree = subprocess.run(["xwininfo", "-root", "-tree"], capture_output=True, text=True).stdout
ids = [line.split()[0] for line in tree.splitlines() if name in line and line.strip().startswith("0x")]

if not ids:
    sys.exit(f"no window with \"{name}\" in its name")

window = int(ids[0], 16)
x = ctypes.cdll.LoadLibrary("libX11.so.6")
x.XOpenDisplay.restype = ctypes.c_void_p
display = ctypes.c_void_p(x.XOpenDisplay(None))

REVERT_TO_PARENT = 2
CURRENT_TIME = 0
NONE = 0

x.XSetInputFocus(display, ctypes.c_ulong(NONE), REVERT_TO_PARENT, CURRENT_TIME)
x.XFlush(display)
time.sleep(away)
x.XSetInputFocus(display, ctypes.c_ulong(window), REVERT_TO_PARENT, CURRENT_TIME)
x.XFlush(display)
x.XCloseDisplay(display)
print(f"focus bounced on 0x{window:x}")
