//	This file is part of the continued Journey MMORPG client
//	Copyright (C) 2015-2019  Daniel Allendorf, Ryan Payton
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Affero General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Affero General Public License for more details.
//
//	You should have received a copy of the GNU Affero General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
#include "Keyboard.h"
#include <android/input.h>

namespace ms {
    constexpr int32_t Keytable[90] = { 0,
                                       0,  // 1
                                       AKEYCODE_0,
                                       AKEYCODE_1,
                                       AKEYCODE_2,
                                       AKEYCODE_3,
                                       AKEYCODE_4,
                                       AKEYCODE_5,
                                       AKEYCODE_6,
                                       AKEYCODE_7,
                                       AKEYCODE_8,
                                       AKEYCODE_9,
                                       AKEYCODE_MINUS,
                                       AKEYCODE_EQUALS,
                                       0,
                                       0,  // 15
                                       AKEYCODE_Q,
                                       AKEYCODE_W,
                                       AKEYCODE_E,
                                       AKEYCODE_R,
                                       AKEYCODE_T,
                                       AKEYCODE_Y,
                                       AKEYCODE_U,
                                       AKEYCODE_I,
                                       AKEYCODE_O,
                                       AKEYCODE_P,
                                       AKEYCODE_LEFT_BRACKET,
                                       AKEYCODE_RIGHT_BRACKET,
                                       0,  // 28
                                       AKEYCODE_CTRL_LEFT,
                                       AKEYCODE_A,
                                       AKEYCODE_S,
                                       AKEYCODE_D,
                                       AKEYCODE_F,
                                       AKEYCODE_G,
                                       AKEYCODE_H,
                                       AKEYCODE_J,
                                       AKEYCODE_K,
                                       AKEYCODE_L,
                                       AKEYCODE_SEMICOLON,
                                       AKEYCODE_APOSTROPHE,
                                       AKEYCODE_GRAVE,
                                       AKEYCODE_SHIFT_LEFT,
                                       AKEYCODE_BACKSLASH,
                                       AKEYCODE_Z,
                                       AKEYCODE_X,
                                       AKEYCODE_C,
                                       AKEYCODE_V,
                                       AKEYCODE_B,
                                       AKEYCODE_N,
                                       AKEYCODE_M,
                                       AKEYCODE_COMMA,
                                       AKEYCODE_PERIOD,
                                       0,
                                       0,
                                       0,  // 55
                                       AKEYCODE_ALT_LEFT,
                                       AKEYCODE_SPACE,
                                       0,  // 58
                                       AKEYCODE_F1,
                                       AKEYCODE_F2,
                                       AKEYCODE_F3,
                                       AKEYCODE_F4,
                                       AKEYCODE_F5,
                                       AKEYCODE_F6,
                                       AKEYCODE_F7,
                                       AKEYCODE_F8,
                                       AKEYCODE_F9,
                                       AKEYCODE_F10,
                                       AKEYCODE_F11,
                                       AKEYCODE_F12,
                                       AKEYCODE_HOME,
                                       0,  // 72
                                       AKEYCODE_PAGE_UP,
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,  // 78
                                       AKEYCODE_MOVE_END,
                                       0,  // 80
                                       AKEYCODE_PAGE_DOWN,
                                       AKEYCODE_INSERT,
                                       AKEYCODE_DEL,
                                       AKEYCODE_ESCAPE,
                                       AKEYCODE_CTRL_RIGHT,
                                       AKEYCODE_SHIFT_RIGHT,
                                       AKEYCODE_ALT_RIGHT,
                                       AKEYCODE_SCROLL_LOCK };

constexpr int32_t Shifttable[126] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    //  10
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    //  20
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    //  30
    0,   0,   49,  39,  51,  52,  53,  55,  0,   57,   //  40
    48,  56,  61,  0,   0,   0,   0,   0,   0,   0,    //  50
    0,   0,   0,   0,   0,   0,   0,   59,  0,   44,   //  60
    0,   46,  47,  50,  97,  98,  99,  100, 101, 102,  //  70
    103, 104, 105, 106, 107, 108, 109, 110, 111, 112,  //  80
    113, 114, 115, 116, 117, 118, 119, 120, 121, 122,  //  90
    0,   0,   0,   54,  45,  0,   0,   0,   0,   0,    // 100
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    // 110
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    // 120
    0,   0,   91,  92,  93,  96                        // 126
};

constexpr int32_t Specialtable[96] = {
    0,   0,   0,   0,  0,  0,  0,  0,  0,  0,   // 10
    0,   0,   0,   0,  0,  0,  0,  0,  0,  0,   // 20
    0,   0,   0,   0,  0,  0,  0,  0,  0,  0,   // 30
    0,   0,   0,   0,  0,  0,  0,  0,  34, 0,   // 40
    0,   0,   0,   60, 95, 62, 63, 41, 33, 64,  // 50
    35,  36,  37,  94, 38, 42, 40, 0,  58, 0,   // 60
    43,  0,   0,   0,  0,  0,  0,  0,  0,  0,   // 70
    0,   0,   0,   0,  0,  0,  0,  0,  0,  0,   // 80
    0,   0,   0,   0,  0,  0,  0,  0,  0,  0,   // 90
    123, 124, 125, 0,  0,  126                  // 96
};

Keyboard::Keyboard() {
    keymap_[AKEYCODE_DPAD_LEFT] = Mapping(KeyType::Id::ACTION, KeyAction::Id::LEFT);
    keymap_[AKEYCODE_DPAD_RIGHT] =
        Mapping(KeyType::Id::ACTION, KeyAction::Id::RIGHT);
    keymap_[AKEYCODE_DPAD_UP] = Mapping(KeyType::Id::ACTION, KeyAction::Id::UP);
    keymap_[AKEYCODE_DPAD_DOWN] = Mapping(KeyType::Id::ACTION, KeyAction::Id::DOWN);
    keymap_[AKEYCODE_ENTER] =
        Mapping(KeyType::Id::ACTION, KeyAction::Id::RETURN);
    keymap_[AKEYCODE_NUMPAD_ENTER] =
            Mapping(KeyType::Id::ACTION, KeyAction::Id::RETURN);
    keymap_[AKEYCODE_TAB] = Mapping(KeyType::Id::ACTION, KeyAction::Id::TAB);

    text_actions_[AKEYCODE_DEL] = KeyAction::Id::BACK;
    text_actions_[AKEYCODE_ENTER] = KeyAction::Id::RETURN;
    text_actions_[AKEYCODE_NUMPAD_ENTER] = KeyAction::Id::RETURN;
    text_actions_[AKEYCODE_SPACE] = KeyAction::Id::SPACE;
    text_actions_[AKEYCODE_TAB] = KeyAction::Id::TAB;
    text_actions_[AKEYCODE_ESCAPE] = KeyAction::Id::ESCAPE;
    text_actions_[AKEYCODE_MOVE_HOME] = KeyAction::Id::HOME;
    text_actions_[AKEYCODE_MOVE_END] = KeyAction::Id::END;
    text_actions_[AKEYCODE_FORWARD_DEL] = KeyAction::Id::DELETE;
    specialCharacters = false;
}

int32_t Keyboard::left_shift_code() const {
    return AKEYCODE_SHIFT_LEFT;
}

int32_t Keyboard::right_shift_code() const {
    return AKEYCODE_SHIFT_RIGHT;
}

int32_t Keyboard::capslock_code() const {
    // Android doesn't have a direct key code for caps lock
    return AKEYCODE_CAPS_LOCK;
}

int32_t Keyboard::left_ctrl_code() const {
    return AKEYCODE_CTRL_LEFT;
}

int32_t Keyboard::right_ctrl_code() const {
    return AKEYCODE_CTRL_RIGHT;
}

bool Keyboard::IsSpecialCharacter() {
        return specialCharacters;
}

std::map<int32_t, Keyboard::Mapping> Keyboard::get_maplekeys() const {
    return maple_keys_;
}

KeyAction::Id Keyboard::get_ctrl_action(int32_t keycode) const {
    switch (keycode) {
        case AKEYCODE_C: return KeyAction::Id::COPY;
        case AKEYCODE_V:
            return KeyAction::Id::PASTE;
            /*case GLFW_KEY_A:
                return KeyAction::Id::SELECTALL;*/
        default: return KeyAction::Id::LENGTH;
    }
}

void Keyboard::assign(uint8_t key, uint8_t tid, int32_t action) {
    if (KeyType::Id type = KeyType::get_type_by_id(tid)) {
        Mapping mapping = Mapping(type, action);

        keymap_[Keytable[key]] = mapping;
        maple_keys_[key] = mapping;
    }
}

void Keyboard::remove(uint8_t key) {
    Mapping mapping = Mapping(KeyType::Id::NONE, 0);

    keymap_[Keytable[key]] = mapping;
    maple_keys_[key] = mapping;
}

Keyboard::Mapping Keyboard::get_text_mapping(int32_t keycode,
                                             bool shift) const {
    if (text_actions_.count(keycode) != 0u) {
        return Mapping(KeyType::Id::ACTION, text_actions_.at(keycode));
    }

    if (keycode == 39 || (keycode >= 44 && keycode <= 57) || keycode == 59
        || keycode == 61 || (keycode >= 91 && keycode <= 93) || keycode == 96) {
        if (!shift) {
            return Mapping(KeyType::Id::TEXT, keycode);
        }

        return Mapping(KeyType::Id::TEXT, Specialtable[keycode - 1]);
    }

    if (keycode >= 33 && keycode <= 126) {
        if (shift) {
            return Mapping(KeyType::Id::TEXT, keycode);
        }

        return Mapping(KeyType::Id::TEXT, Shifttable[keycode - 1]);
    }

    switch (keycode) {
        case AKEYCODE_DPAD_LEFT:
        case AKEYCODE_DPAD_RIGHT:
        case AKEYCODE_DPAD_UP:
        case AKEYCODE_DPAD_DOWN: return keymap_.at(keycode);
        default: return Mapping(KeyType::Id::NONE, 0);
    }
}

Keyboard::Mapping Keyboard::get_mapping(int32_t keycode) const {
    auto iter = keymap_.find(keycode);

    if (iter == keymap_.end()) {
        return Mapping(KeyType::Id::NONE, 0);
    }

    return iter->second;
}

Keyboard::Mapping Keyboard::get_maple_mapping(int32_t keycode) const {
    auto iter = maple_keys_.find(keycode);

    if (iter == maple_keys_.end()) {
        return Mapping(KeyType::Id::NONE, 0);
    }

    return iter->second;
}

int Keyboard::Key2Character(int scancode, bool shiftPressed) {
    int ascii = 0;
    bool specialCharacters = false;
    if (scancode >= AKEYCODE_A && scancode <= AKEYCODE_Z) {
        ascii = scancode + 'a' - AKEYCODE_A;
        if (shiftPressed) {
            ascii = std::toupper(ascii);
        }
    } else if (scancode >= AKEYCODE_0 && scancode <= AKEYCODE_9) {
        ascii = scancode + '0' - AKEYCODE_0;

    } else {
        switch (scancode) {
            case AKEYCODE_SPACE:
                ascii = ' ';
                break;
            case AKEYCODE_ENTER:
                ascii = '\n';
                break;
            case AKEYCODE_DEL:
                ascii = '\b';
                specialCharacters = true;
                break;
            case AKEYCODE_TAB:
                ascii = '\t';
                specialCharacters = true;
                break;
            case AKEYCODE_ESCAPE:
                ascii = 27;
                break;
            case AKEYCODE_GRAVE:
                ascii = shiftPressed ? '~' : '`';
                break;
            case AKEYCODE_MINUS:
                ascii = shiftPressed ? '_' : '-';
                break;
            case AKEYCODE_EQUALS:
                ascii = shiftPressed ? '+' : '=';
                break;
            case AKEYCODE_LEFT_BRACKET:
                ascii = shiftPressed ? '{' : '[';
                break;
            case AKEYCODE_RIGHT_BRACKET:
                ascii = shiftPressed ? '}' : ']';
                break;
            case AKEYCODE_BACKSLASH:
                ascii = shiftPressed ? '|' : '\\';
                break;
            case AKEYCODE_SEMICOLON:
                ascii = shiftPressed ? ':' : ';';
                break;
            case AKEYCODE_APOSTROPHE:
                ascii = shiftPressed ? '"' : '\'';
                break;
            case AKEYCODE_COMMA:
                ascii = shiftPressed ? '<' : ',';
                break;
            case AKEYCODE_PERIOD:
                ascii = shiftPressed ? '>' : '.';
                break;
            case AKEYCODE_SLASH:
                ascii = shiftPressed ? '?' : '/';
                break;
        }
    }
    this->specialCharacters = specialCharacters;
    return ascii;
}
}  // namespace ms