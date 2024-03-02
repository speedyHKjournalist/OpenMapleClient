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
#include "glfm.h"
#include <android/input.h>

namespace ms {
    constexpr int32_t Keytable[90] = {0,
                                      0,  // 1
                                      GLFMKeyCode0,
                                      GLFMKeyCode1,
                                      GLFMKeyCode2,
                                      GLFMKeyCode3,
                                      GLFMKeyCode4,
                                      GLFMKeyCode5,
                                      GLFMKeyCode6,
                                      GLFMKeyCode7,
                                      GLFMKeyCode8,
                                      GLFMKeyCode9,
                                      GLFMKeyCodeMinus,
                                      GLFMKeyCodeEqual,
                                      0,
                                      0,  // 15
                                      GLFMKeyCodeQ,
                                      GLFMKeyCodeW,
                                      GLFMKeyCodeE,
                                      GLFMKeyCodeR,
                                      GLFMKeyCodeT,
                                      GLFMKeyCodeY,
                                      GLFMKeyCodeU,
                                      GLFMKeyCodeI,
                                      GLFMKeyCodeO,
                                      GLFMKeyCodeP,
                                      GLFMKeyCodeBracketLeft,
                                      GLFMKeyCodeBracketRight,
                                      0,  // 28
                                      GLFMKeyCodeControlLeft,
                                      GLFMKeyCodeA,
                                      GLFMKeyCodeS,
                                      GLFMKeyCodeD,
                                      GLFMKeyCodeF,
                                      GLFMKeyCodeG,
                                      GLFMKeyCodeH,
                                      GLFMKeyCodeJ,
                                      GLFMKeyCodeK,
                                      GLFMKeyCodeL,
                                      GLFMKeyCodeSemicolon,
                                      GLFMKeyCodeQuote,
                                      GLFMKeyCodeBackquote,
                                      GLFMKeyCodeShiftLeft,
                                      GLFMKeyCodeBackslash,
                                      GLFMKeyCodeZ,
                                      GLFMKeyCodeX,
                                      GLFMKeyCodeC,
                                      GLFMKeyCodeV,
                                      GLFMKeyCodeB,
                                      GLFMKeyCodeN,
                                      GLFMKeyCodeM,
                                      GLFMKeyCodeComma,
                                      GLFMKeyCodePeriod,
                                      0,
                                      0,
                                      0,  // 55
                                      GLFMKeyCodeAltLeft,
                                      GLFMKeyCodeSpace,
                                      0,  // 58
                                      GLFMKeyCodeF1,
                                      GLFMKeyCodeF2,
                                      GLFMKeyCodeF3,
                                      GLFMKeyCodeF4,
                                      GLFMKeyCodeF5,
                                      GLFMKeyCodeF6,
                                      GLFMKeyCodeF7,
                                      GLFMKeyCodeF8,
                                      GLFMKeyCodeF9,
                                      GLFMKeyCodeF10,
                                      GLFMKeyCodeF11,
                                      GLFMKeyCodeF12,
                                      GLFMKeyCodeHome,
                                      0,  // 72
                                      GLFMKeyCodePageUp,
                                      0,
                                      0,
                                      0,
                                      0,
                                      0,  // 78
                                      GLFMKeyCodeEnd,
                                      GLFMKeyCodeCapsLock,  // 80
                                      GLFMKeyCodePageDown,
                                      GLFMKeyCodeInsert,
                                      GLFMKeyCodeDelete,
                                      GLFMKeyCodeEscape,
                                      GLFMKeyCodeControlRight,
                                      GLFMKeyCodeShiftRight,
                                      GLFMKeyCodeAltRight,
                                      GLFMKeyCodeScrollLock};

    constexpr int32_t Shifttable[126] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  10
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  20
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  30
            0, 0, 49, 39, 51, 52, 53, 55, 0, 57,   //  40
            48, 56, 61, 0, 0, 0, 0, 0, 0, 0,    //  50
            0, 0, 0, 0, 0, 0, 0, 59, 0, 44,   //  60
            0, 46, 47, 50, 97, 98, 99, 100, 101, 102,  //  70
            103, 104, 105, 106, 107, 108, 109, 110, 111, 112,  //  80
            113, 114, 115, 116, 117, 118, 119, 120, 121, 122,  //  90
            0, 0, 0, 54, 45, 0, 0, 0, 0, 0,    // 100
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    // 110
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    // 120
            0, 0, 91, 92, 93, 96                        // 126
    };

    constexpr int32_t Specialtable[96] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 10
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 20
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 30
            0, 0, 0, 0, 0, 0, 0, 0, 34, 0,   // 40
            0, 0, 0, 60, 95, 62, 63, 41, 33, 64,  // 50
            35, 36, 37, 94, 38, 42, 40, 0, 58, 0,   // 60
            43, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 70
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 80
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 90
            123, 124, 125, 0, 0, 126                  // 96
    };

    Keyboard::Keyboard() {
        keymap_[GLFMKeyCodeArrowLeft] = Mapping(KeyType::Id::ACTION, KeyAction::Id::LEFT);
        keymap_[GLFMKeyCodeArrowRight] = Mapping(KeyType::Id::ACTION, KeyAction::Id::RIGHT);
        keymap_[GLFMKeyCodeArrowUp] = Mapping(KeyType::Id::ACTION, KeyAction::Id::UP);
        keymap_[GLFMKeyCodeArrowDown] = Mapping(KeyType::Id::ACTION, KeyAction::Id::DOWN);
        keymap_[GLFMKeyCodeEnter] = Mapping(KeyType::Id::ACTION, KeyAction::Id::RETURN);
        keymap_[GLFMKeyCodeNumpadEnter] = Mapping(KeyType::Id::ACTION, KeyAction::Id::RETURN);
        keymap_[GLFMKeyCodeTab] = Mapping(KeyType::Id::ACTION, KeyAction::Id::TAB);

        text_actions_[GLFMKeyCodeDelete] = KeyAction::Id::BACK;
        text_actions_[GLFMKeyCodeEnter] = KeyAction::Id::RETURN;
        text_actions_[GLFMKeyCodeNumpadEnter] = KeyAction::Id::RETURN;
        text_actions_[GLFMKeyCodeSpace] = KeyAction::Id::SPACE;
        text_actions_[GLFMKeyCodeTab] = KeyAction::Id::TAB;
        text_actions_[GLFMKeyCodeEscape] = KeyAction::Id::ESCAPE;
        text_actions_[GLFMKeyCodeHome] = KeyAction::Id::HOME;
        text_actions_[GLFMKeyCodeEnd] = KeyAction::Id::END;
        text_actions_[GLFMKeyCodeDelete] = KeyAction::Id::DELETE;
    }

    int32_t Keyboard::left_shift_code() const {
        return GLFMKeyCodeShiftLeft;
    }

    int32_t Keyboard::right_shift_code() const {
        return GLFMKeyCodeShiftRight;
    }

    int32_t Keyboard::capslock_code() const {
        return GLFMKeyCodeCapsLock;
    }

    int32_t Keyboard::left_ctrl_code() const {
        return GLFMKeyCodeControlLeft;
    }

    int32_t Keyboard::right_ctrl_code() const {
        return GLFMKeyCodeControlRight;
    }

    std::map<int32_t, Keyboard::Mapping> Keyboard::get_maplekeys() const {
        return maple_keys_;
    }

    KeyAction::Id Keyboard::get_ctrl_action(int32_t keycode) const {
        switch (keycode) {
            case GLFMKeyCodeC:
                return KeyAction::Id::COPY;
            case GLFMKeyCodeV:
                return KeyAction::Id::PASTE;
                /*case GLFW_KEY_A:
                    return KeyAction::Id::SELECTALL;*/
            default:
                return KeyAction::Id::LENGTH;
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
            case GLFMKeyCodeArrowLeft:
            case GLFMKeyCodeArrowRight:
            case GLFMKeyCodeArrowUp:
            case GLFMKeyCodeArrowDown:
                return keymap_.at(keycode);
            default:
                return Mapping(KeyType::Id::NONE, 0);
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
}  // namespace ms