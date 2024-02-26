using namespace augs::event;
using namespace augs::event::keys;

augs::event::keys::key translate_sdl2_mouse_key(const int button) {
    switch (button) {
        case SDL_BUTTON_LEFT: return key::LMOUSE;
        case SDL_BUTTON_RIGHT: return key::RMOUSE;
        case SDL_BUTTON_MIDDLE: return key::MMOUSE;
        case SDL_BUTTON_X1: return key::MOUSE4;
        case SDL_BUTTON_X2: return key::MOUSE5;
        default: return key::INVALID;
    }
}

key translate_sdl2_key(const int sym) {
    switch (sym) {
        case SDLK_CAPSLOCK: return key::CAPSLOCK;

        case SDLK_LCTRL: return key::LCTRL;
        case SDLK_RCTRL: return key::RCTRL;

        case SDLK_LALT: return key::LALT;
        case SDLK_RALT: return key::RALT;

        case SDLK_LSHIFT: return key::LSHIFT;
        case SDLK_RSHIFT: return key::RSHIFT;

        case SDLK_LGUI: return key::LWIN;
        case SDLK_RGUI: return key::RWIN;

        case SDLK_BACKSPACE: return key::BACKSPACE;
        case SDLK_TAB: return key::TAB;
        case SDLK_RETURN: return key::ENTER;
        case SDLK_PAUSE: return key::PAUSE;
        case SDLK_ESCAPE: return key::ESC;
        case SDLK_SPACE: return key::SPACE;
        case SDLK_PAGEUP: return key::PAGEUP;
        case SDLK_PAGEDOWN: return key::PAGEDOWN;
        case SDLK_END: return key::END;
        case SDLK_HOME: return key::HOME;
        case SDLK_LEFT: return key::LEFT;
        case SDLK_UP: return key::UP;
        case SDLK_RIGHT: return key::RIGHT;
        case SDLK_DOWN: return key::DOWN;
        case SDLK_PRINTSCREEN: return key::PRINTSCREEN;
        case SDLK_INSERT: return key::INSERT;
        case SDLK_DELETE: return key::DEL;

        case SDLK_KP_MULTIPLY: return key::MULTIPLY;
        case SDLK_KP_PLUS: return key::ADD;
        case SDLK_KP_MINUS: return key::SUBTRACT;
        case SDLK_KP_DECIMAL: return key::DECIMAL;
        case SDLK_KP_DIVIDE: return key::DIVIDE;

        case SDLK_F1: return key::F1;
        case SDLK_F2: return key::F2;
        case SDLK_F3: return key::F3;
        case SDLK_F4: return key::F4;
        case SDLK_F5: return key::F5;
        case SDLK_F6: return key::F6;
        case SDLK_F7: return key::F7;
        case SDLK_F8: return key::F8;
        case SDLK_F9: return key::F9;
        case SDLK_F10: return key::F10;
        case SDLK_F11: return key::F11;
        case SDLK_F12: return key::F12;
        // Add more F-keys if your application uses them

        case SDLK_a: return key::A;
        case SDLK_b: return key::B;
        case SDLK_c: return key::C;
        case SDLK_d: return key::D;
        case SDLK_e: return key::E;
        case SDLK_f: return key::F;
        case SDLK_g: return key::G;
        case SDLK_h: return key::H;
        case SDLK_i: return key::I;
        case SDLK_j: return key::J;
        case SDLK_k: return key::K;
        case SDLK_l: return key::L;
        case SDLK_m: return key::M;
        case SDLK_n: return key::N;
        case SDLK_o: return key::O;
        case SDLK_p: return key::P;
        case SDLK_q: return key::Q;
        case SDLK_r: return key::R;
        case SDLK_s: return key::S;
        case SDLK_t: return key::T;
        case SDLK_u: return key::U;
        case SDLK_v: return key::V;
        case SDLK_w: return key::W;
        case SDLK_x: return key::X;
        case SDLK_y: return key::Y;
        case SDLK_z: return key::Z;

        case SDLK_0: return key::_0;
        case SDLK_1: return key::_1;
        case SDLK_2: return key::_2;
        case SDLK_3: return key::_3;
        case SDLK_4: return key::_4;
        case SDLK_5: return key::_5;
        case SDLK_6: return key::_6;
        case SDLK_7: return key::_7;
        case SDLK_8: return key::_8;
        case SDLK_9: return key::_9;

        // Add any other keys you need

        default: return key::INVALID;
    }
}
