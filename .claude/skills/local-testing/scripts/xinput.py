# Minimal XTest driver for $DISPLAY (no xdotool needed):
#   python3 xinput.py click X Y      move and left-click
#   python3 xinput.py type TEXT      type letters, digits and spaces
#   python3 xinput.py scroll X Y N   scroll down N notches at X Y (up if N < 0)
#   python3 xinput.py blur | focus     the game loses / gets back the keyboard focus
#   python3 xinput.py key NAME       press one key by its X keysym name, e.g. Return, Escape, BackSpace,
#                                    or a combination like Control_L+v
import ctypes
import sys
import time

x11 = ctypes.CDLL("libX11.so.6")
xtst = ctypes.CDLL("libXtst.so.6")

x11.XOpenDisplay.restype = ctypes.c_void_p
x11.XOpenDisplay.argtypes = [ctypes.c_char_p]
x11.XFlush.argtypes = [ctypes.c_void_p]
x11.XCloseDisplay.argtypes = [ctypes.c_void_p]
x11.XStringToKeysym.restype = ctypes.c_ulong
x11.XStringToKeysym.argtypes = [ctypes.c_char_p]
x11.XKeysymToKeycode.restype = ctypes.c_ubyte
x11.XKeysymToKeycode.argtypes = [ctypes.c_void_p, ctypes.c_ulong]
xtst.XTestFakeMotionEvent.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_ulong]
xtst.XTestFakeButtonEvent.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_int, ctypes.c_ulong]
xtst.XTestFakeKeyEvent.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_int, ctypes.c_ulong]

display = x11.XOpenDisplay(None)

if not display:
	sys.exit("no display")


def keycode_of(keysym_name):
	keycode = x11.XKeysymToKeycode(display, x11.XStringToKeysym(keysym_name.encode()))

	if keycode == 0:
		sys.exit("unknown key: " + keysym_name)

	return keycode


def press(keysym_name, shift=False):
	keycode = keycode_of(keysym_name)
	shift_keycode = keycode_of("Shift_L")

	if shift:
		xtst.XTestFakeKeyEvent(display, shift_keycode, 1, 0)

	xtst.XTestFakeKeyEvent(display, keycode, 1, 0)
	x11.XFlush(display)
	time.sleep(0.05)
	xtst.XTestFakeKeyEvent(display, keycode, 0, 0)

	if shift:
		xtst.XTestFakeKeyEvent(display, shift_keycode, 0, 0)

	x11.XFlush(display)
	time.sleep(0.05)


mode = sys.argv[1]

if mode == "click":
	x, y = int(sys.argv[2]), int(sys.argv[3])
	xtst.XTestFakeMotionEvent(display, -1, x, y, 0)
	x11.XFlush(display)
	time.sleep(0.3)
	xtst.XTestFakeButtonEvent(display, 1, 1, 0)
	x11.XFlush(display)
	time.sleep(0.1)
	xtst.XTestFakeButtonEvent(display, 1, 0, 0)
	x11.XFlush(display)
	time.sleep(0.2)
elif mode == "drag":
	# drag x1 y1 x2 y2 - e.g. a window's edge, in steps so ImGui sees the motion
	x1, y1, x2, y2 = (int(v) for v in sys.argv[2:6])
	xtst.XTestFakeMotionEvent(display, -1, x1, y1, 0)
	x11.XFlush(display)
	time.sleep(0.3)
	xtst.XTestFakeButtonEvent(display, 1, 1, 0)
	x11.XFlush(display)
	time.sleep(0.2)

	for i in range(1, 21):
		xtst.XTestFakeMotionEvent(display, -1, x1 + (x2 - x1) * i // 20, y1 + (y2 - y1) * i // 20, 0)
		x11.XFlush(display)
		time.sleep(0.03)

	time.sleep(0.2)
	xtst.XTestFakeButtonEvent(display, 1, 0, 0)
	x11.XFlush(display)
	time.sleep(0.2)
elif mode == "scroll":
	x, y, notches = int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])
	button = 5 if notches > 0 else 4
	xtst.XTestFakeMotionEvent(display, -1, x, y, 0)
	x11.XFlush(display)
	time.sleep(0.2)

	for _ in range(abs(notches)):
		xtst.XTestFakeButtonEvent(display, button, 1, 0)
		xtst.XTestFakeButtonEvent(display, button, 0, 0)
		x11.XFlush(display)
		time.sleep(0.1)
elif mode == "type":
	# Punctuation by its keysym on a US layout: (name, needs shift)
	punctuation = {
		" ": ("space", False), ".": ("period", False), ",": ("comma", False), ":": ("semicolon", True), ";": ("semicolon", False),
		"-": ("minus", False), "_": ("minus", True), "/": ("slash", False), "[": ("bracketleft", False), "]": ("bracketright", False),
		"@": ("2", True), "#": ("3", True), "=": ("equal", False)
	}

	for c in sys.argv[2]:
		if c in punctuation:
			press(*punctuation[c])
		else:
			press(c.lower(), c.isupper())
elif mode == "blur":
	# The game loses the keyboard focus (to no window), as when the player switches to another program
	x11.XSetInputFocus.argtypes = [ctypes.c_void_p, ctypes.c_ulong, ctypes.c_int, ctypes.c_ulong]
	x11.XSetInputFocus(display, 0, 1, 0)
	x11.XFlush(display)
	time.sleep(0.2)
elif mode == "focus":
	# Focus back to the window under the pointer (PointerRoot) - the game in a test display
	x11.XSetInputFocus.argtypes = [ctypes.c_void_p, ctypes.c_ulong, ctypes.c_int, ctypes.c_ulong]
	x11.XSetInputFocus(display, 1, 1, 0)
	x11.XFlush(display)
	time.sleep(0.2)
elif mode == "key":
	*modifiers, key = sys.argv[2].split("+")

	for m in modifiers:
		xtst.XTestFakeKeyEvent(display, keycode_of(m), 1, 0)

	press(key)

	for m in reversed(modifiers):
		xtst.XTestFakeKeyEvent(display, keycode_of(m), 0, 0)

	x11.XFlush(display)

x11.XCloseDisplay(display)
