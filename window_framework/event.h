#pragma once
#define UNICODE
#include <Windows.h>
#undef min
#undef max
#include "../math/rects.h"
#include "../math/vec2d.h"

namespace augmentations {
	namespace window {
		namespace event {
			typedef unsigned message;

			struct state {
				struct mouse_info {
					vec2<int> pos, rel, ldrag, rdrag;
					bool state[3];
					int scroll;
				} mouse;

				event::message msg;
				int key;
				bool repeated;
				wchar_t utf16;
				unsigned utf32;
				bool keys[256];
			};

			enum {
				close = SC_CLOSE,
				move = WM_MOVE,
				activate = WM_ACTIVATE,
				minimize = SC_MINIMIZE,
				maximize = SC_MAXIMIZE,
				restore = SC_RESTORE,
				clipboard_change = WM_CLIPBOARDUPDATE
			};

			namespace key {
				enum { 	
					down			= WM_KEYDOWN	  ,
					up				= WM_KEYUP		  ,
					character		= WM_CHAR		  ,
					unichar			= WM_UNICHAR	  
				};
			}

			namespace keys {
				extern bool is_numpad_key(int);
				enum {
					LMOUSE			= VK_LBUTTON      ,
					RMOUSE			= VK_RBUTTON      ,
					MMOUSE			= VK_MBUTTON      ,    
					CANCEL			= VK_CANCEL       ,
					BACKSPACE	    = VK_BACK         ,
					TAB				= VK_TAB          ,
					CLEAR		    = VK_CLEAR        ,
					ENTER		    = VK_RETURN       ,
					SHIFT		    = VK_SHIFT        ,
					CTRL			= VK_CONTROL      ,
					ALT				= VK_MENU         ,
					PAUSE		    = VK_PAUSE        ,
					CAPSLOCK	    = VK_CAPITAL      ,
					ESC   			= VK_ESCAPE       ,
					SPACE		    = VK_SPACE        ,
					PAGEUP			= VK_PRIOR        ,
					PAGEDOWN	    = VK_NEXT         ,
					END				= VK_END          ,
					HOME		    = VK_HOME         ,
					LEFT		    = VK_LEFT         ,
					UP				= VK_UP           ,
					RIGHT		    = VK_RIGHT        ,
					DOWN		    = VK_DOWN         ,
					SELECT			= VK_SELECT       ,
					PRINT		    = VK_PRINT        ,
					EXECUTE			= VK_EXECUTE      ,
					PRINTSCREEN		= VK_SNAPSHOT     ,
					INSERT			= VK_INSERT       ,
					DEL				= VK_DELETE       ,
					HELP		    = VK_HELP         ,
					LWIN		    = VK_LWIN         ,
					RWIN		    = VK_RWIN         ,
					APPS		    = VK_APPS         ,
					SLEEP		    = VK_SLEEP        ,
					NUMPAD0			= VK_NUMPAD0      ,
					NUMPAD1			= VK_NUMPAD1      ,
					NUMPAD2			= VK_NUMPAD2      ,
					NUMPAD3			= VK_NUMPAD3      ,
					NUMPAD4			= VK_NUMPAD4      ,
					NUMPAD5			= VK_NUMPAD5      ,
					NUMPAD6			= VK_NUMPAD6      ,
					NUMPAD7			= VK_NUMPAD7      ,
					NUMPAD8			= VK_NUMPAD8      ,
					NUMPAD9			= VK_NUMPAD9      ,
					MULTIPLY	    = VK_MULTIPLY     ,
					ADD				= VK_ADD          ,
					SEPARATOR	    = VK_SEPARATOR    ,
					SUBTRACT	    = VK_SUBTRACT     ,
					DECIMAL			= VK_DECIMAL      ,
					DIVIDE			= VK_DIVIDE       ,
					F1				= VK_F1           ,
					F2				= VK_F2           ,
					F3				= VK_F3           ,
					F4				= VK_F4           ,
					F5				= VK_F5           ,
					F6				= VK_F6           ,
					F7				= VK_F7           ,
					F8				= VK_F8           ,
					F9				= VK_F9           ,
					F10				= VK_F10          ,
					F11				= VK_F11          ,
					F12				= VK_F12          ,
					F13				= VK_F13          ,
					F14				= VK_F14          ,
					F15				= VK_F15          ,
					F16				= VK_F16          ,
					F17				= VK_F17          ,
					F18				= VK_F18          ,
					F19				= VK_F19          ,
					F20				= VK_F20          ,
					F21				= VK_F21          ,
					F22				= VK_F22          ,
					F23				= VK_F23          ,
					F24				= VK_F24          ,
					A			    = 'A'             ,
					B			    = 'B'             ,
					C			    = 'C'             ,
					D			    = 'D'             ,
					E			    = 'E'             ,
					F			    = 'F'             ,
					G			    = 'G'             ,
					H			    = 'H'             ,
					I			    = 'I'             ,
					J			    = 'J'             ,
					K			    = 'K'             ,
					L			    = 'L'             ,
					M			    = 'M'             ,
					N			    = 'N'             ,
					O			    = 'O'             ,
					P			    = 'P'             ,
					Q			    = 'Q'             ,
					R			    = 'R'             ,
					S			    = 'S'             ,
					T			    = 'T'             ,
					U			    = 'U'             ,
					V			    = 'V'             ,
					W			    = 'W'             ,
					X			    = 'X'             ,
					Y			    = 'Y'             ,
					Z			    = 'Z'             ,
					_0				= '0'             ,
					_1				= '1'             ,
					_2				= '2'             ,
					_3				= '3'             ,
					_4				= '4'             ,
					_5				= '5'             ,
					_6				= '6'             ,
					_7				= '7'             ,
					_8				= '8'             ,
					_9				= '9'             ,
					NUMLOCK			= VK_NUMLOCK      ,
					SCROLL			= VK_SCROLL       ,
					LSHIFT			= VK_LSHIFT       ,
					RSHIFT			= VK_RSHIFT       ,
					LCTRL			= VK_LCONTROL     ,
					RCTRL			= VK_RCONTROL     ,
					LALT		    = VK_LMENU        ,
					RALT		    = VK_RMENU        ,
					DASH			= VK_OEM_3		  	
				};
			};

			namespace mouse {
				enum {
					ltripleclick,
					motion =		WM_MOUSEMOVE,
					wheel =		    WM_MOUSEWHEEL,
					ldoubleclick =	WM_LBUTTONDBLCLK,
					mdoubleclick =	WM_MBUTTONDBLCLK,
					rdoubleclick =	WM_RBUTTONDBLCLK,
					ldown =			WM_LBUTTONDOWN,
					lup =			WM_LBUTTONUP,
					mdown =			WM_MBUTTONDOWN,
					mup =			WM_MBUTTONUP,
					rdown =			WM_RBUTTONDOWN,
					rup =			WM_RBUTTONUP
				};
			};
		};
	}
}