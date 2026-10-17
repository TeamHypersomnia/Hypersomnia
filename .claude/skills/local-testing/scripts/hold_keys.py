# Holds keys and moves the mouse on $DISPLAY through XTest, for driving gameplay:
#   python3 hold_keys.py KEY SECS [KEY SECS ...]   e.g. d 0.3 e 0.1 - hold each key (X keysym name) for SECS
#   ... move X Y ... sleep SECS ...                mixed in between the keys
# The game reads mouse motion relatively, so "move" turns the crosshair - keep the mouse still to keep the camera on the character.
import ctypes
import sys
import time

x11 = ctypes.CDLL("libX11.so.6")
xtst = ctypes.CDLL("libXtst.so.6")

x11.XOpenDisplay.restype = ctypes.c_void_p
x11.XOpenDisplay.argtypes = [ctypes.c_char_p]
x11.XFlush.argtypes = [ctypes.c_void_p]
x11.XStringToKeysym.restype = ctypes.c_ulong
x11.XStringToKeysym.argtypes = [ctypes.c_char_p]
x11.XKeysymToKeycode.restype = ctypes.c_ubyte
x11.XKeysymToKeycode.argtypes = [ctypes.c_void_p, ctypes.c_ulong]
xtst.XTestFakeKeyEvent.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_int, ctypes.c_ulong]
xtst.XTestFakeMotionEvent.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_ulong]

display = x11.XOpenDisplay(None)

if not display:
	sys.exit("no display")

args = sys.argv[1:]
i = 0

while i < len(args):
	if args[i] == "move":
		xtst.XTestFakeMotionEvent(display, -1, int(args[i + 1]), int(args[i + 2]), 0)
		x11.XFlush(display)
		time.sleep(0.2)
		i += 3
		continue

	if args[i] == "sleep":
		time.sleep(float(args[i + 1]))
		i += 2
		continue

	keycode = x11.XKeysymToKeycode(display, x11.XStringToKeysym(args[i].encode()))

	if keycode == 0:
		sys.exit("unknown key: " + args[i])

	xtst.XTestFakeKeyEvent(display, keycode, 1, 0)
	x11.XFlush(display)
	time.sleep(float(args[i + 1]))
	xtst.XTestFakeKeyEvent(display, keycode, 0, 0)
	x11.XFlush(display)
	time.sleep(0.15)
	i += 2
