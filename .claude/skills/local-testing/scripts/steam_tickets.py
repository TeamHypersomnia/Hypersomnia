# Steam Web API tickets from the user's running Steam client, without the game:
#   cd <dir with steam_appid.txt containing 2660970> && python3 steam_tickets.py IDENTITY...
# prints "identity result TICKET_HEX" per line (result 1 = OK). Verify where the Web API key lives, never print the key:
#   curl -G --data-urlencode key=... --data-urlencode appid=2660970 --data-urlencode ticket=... --data-urlencode identity=...
#        https://api.steampowered.com/ISteamUserAuth/AuthenticateUserTicket/v1/
# The user's Steam shows them "playing Hypersomnia" for a moment.
import ctypes, os, sys, time
repo = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../.."))
lib = ctypes.CDLL(os.path.join(repo, "cmake/steam_integration/bin/linux/libsteam_api.so"))
err = ctypes.create_string_buffer(1024)
lib.SteamAPI_InitFlat.restype = ctypes.c_int
r = lib.SteamAPI_InitFlat(err)
assert r == 0, (r, err.value)
lib.SteamAPI_ManualDispatch_Init()
lib.SteamAPI_GetHSteamPipe.restype = ctypes.c_int32
pipe = lib.SteamAPI_GetHSteamPipe()
lib.SteamAPI_SteamUser_v023.restype = ctypes.c_void_p
user = lib.SteamAPI_SteamUser_v023()
lib.SteamAPI_ISteamUser_GetAuthTicketForWebApi.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
lib.SteamAPI_ISteamUser_GetAuthTicketForWebApi.restype = ctypes.c_uint32

class Msg(ctypes.Structure):
	_fields_ = [("user", ctypes.c_int32), ("cb", ctypes.c_int), ("param", ctypes.c_void_p), ("size", ctypes.c_int)]

class Resp(ctypes.Structure):
	_fields_ = [("h", ctypes.c_uint32), ("res", ctypes.c_int), ("n", ctypes.c_int), ("t", ctypes.c_ubyte * 2560)]

for ident in sys.argv[1:]:
	h = lib.SteamAPI_ISteamUser_GetAuthTicketForWebApi(user, ident.encode())
	got = None
	for _ in range(200):
		lib.SteamAPI_ManualDispatch_RunFrame(pipe)
		m = Msg()
		while lib.SteamAPI_ManualDispatch_GetNextCallback(pipe, ctypes.byref(m)):
			if m.cb == 168:
				resp = ctypes.cast(m.param, ctypes.POINTER(Resp)).contents
				if resp.h == h:
					got = (resp.res, bytes(resp.t[:resp.n]).hex().upper())
			lib.SteamAPI_ManualDispatch_FreeLastCallback(pipe)
		if got: break
		time.sleep(0.05)
	print(ident, got[0], got[1])
lib.SteamAPI_Shutdown()
