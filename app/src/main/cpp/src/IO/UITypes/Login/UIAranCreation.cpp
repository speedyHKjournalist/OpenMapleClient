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
#include "UIAranCreation.h"

#include <nlnx/nx.hpp>

#include "../../Audio/Audio.h"
#include "../../Configuration.h"
#include "../../Data/ItemData.h"
#include "../../Net/Packets/CharCreationPackets.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "UICharSelect.h"
#include "UILoginNotice.h"
#include "UIRaceSelect.h"

namespace ms {
namespace {
auto fn_create_char = []<typename... T>(T && ...args) {
    CreateCharPacket(std::forward<T>(args)...).dispatch();
};

auto fn_name_char = []<typename... T>(T && ...args) {
    NameCharPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIAranCreation::UIAranCreation() :
    UIElement(Point<int16_t>(0, 0), Point<int16_t>(800, 600)) {
    gender_ = false;
    char_set_ = false;
    named_ = false;

    std::string version_text = Configuration::get().get_version();
    version_ = Text(Text::Font::A11M,
                    Text::Alignment::LEFT,
                    Color::Name::LEMONGRASS,
                    "Ver. " + version_text);

    nl::node Login = nl::nx::ui["Login.img"];
    nl::node Common = Login["Common"];
    nl::node CustomizeChar = Login["CustomizeChar"]["2000"];
    nl::node back = nl::nx::map001["Back"]["login.img"]["back"];
    nl::node board = CustomizeChar["board"];
    nl::node genderSelect = CustomizeChar["genderSelect"];

    sky_ = back["2"];
    cloud_ = back["27"];

    sprites_.emplace_back(back["33"], Point<int16_t>(256, 299));
    sprites_.emplace_back(back["34"], Point<int16_t>(587, 157));
    sprites_gender_select_.emplace_back(board["genderTop"],
                                        Point<int16_t>(491, 168));
    sprites_gender_select_.emplace_back(board["boardMid"],
                                        Point<int16_t>(491, 220));
    sprites_gender_select_.emplace_back(board["boardBottom"],
                                        Point<int16_t>(491, 313));
    sprites_lookboard_.emplace_back(CustomizeChar["charSet"],
                                    Point<int16_t>(473, 103));

    for (size_t i = 0; i <= 6; i++) {
        sprites_lookboard_.emplace_back(CustomizeChar["avatarSel"][i]["normal"],
                                        Point<int16_t>(504, 187 + (i * 18)));
    }

    buttons_[Buttons::BT_CHARC_GENDER_M] =
        std::make_unique<MapleButton>(genderSelect["male"],
                                      Point<int16_t>(439, 106));
    buttons_[Buttons::BT_CHARC_GEMDER_F] =
        std::make_unique<MapleButton>(genderSelect["female"],
                                      Point<int16_t>(437, 106));
    buttons_[Buttons::BT_CHARC_SKINL] =
        std::make_unique<MapleButton>(CustomizeChar["BtLeft"],
                                      Point<int16_t>(562, 187 + (2 * 18)));
    buttons_[Buttons::BT_CHARC_SKINR] =
        std::make_unique<MapleButton>(CustomizeChar["BtRight"],
                                      Point<int16_t>(699, 187 + (2 * 18)));
    buttons_[Buttons::BT_CHARC_OK] =
        std::make_unique<MapleButton>(CustomizeChar["BtYes"],
                                      Point<int16_t>(520, 397));
    buttons_[Buttons::BT_CHARC_CANCEL] =
        std::make_unique<MapleButton>(CustomizeChar["BtNo"],
                                      Point<int16_t>(594, 397));

    buttons_[Buttons::BT_CHARC_SKINL]->set_active(false);
    buttons_[Buttons::BT_CHARC_SKINR]->set_active(false);

    nameboard_ = CustomizeChar["charName"];
    namechar_ = Textfield(
        Text::Font::A13M,
        Text::Alignment::LEFT,
        Color::Name::WHITE,
        Rectangle<int16_t>(Point<int16_t>(524, 196), Point<int16_t>(630, 253)),
        12);

    sprites_.emplace_back(Common["frame"], Point<int16_t>(400, 300));
    sprites_.emplace_back(Common["step"]["3"], Point<int16_t>(40, 0));

    buttons_[Buttons::BT_BACK] =
        std::make_unique<MapleButton>(Login["Common"]["BtStart"],
                                      Point<int16_t>(0, 515));

    namechar_.set_state(Textfield::DISABLED);

    namechar_.set_enter_callback(
        [&](const std::string &) { button_pressed(Buttons::BT_CHARC_OK); });

    namechar_.set_key_callback(KeyAction::Id::ESCAPE, [&]() {
        button_pressed(Buttons::BT_CHARC_CANCEL);
    });

    facename_ =
        Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::BLACK);
    hairname_ =
        Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::BLACK);
    bodyname_ =
        Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::BLACK);
    topname_ =
        Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::BLACK);
    botname_ =
        Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::BLACK);
    shoename_ =
        Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::BLACK);
    wepname_ =
        Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::BLACK);

    nl::node mkinfo = nl::nx::etc["MakeCharInfo.img"]["Info"];

    for (size_t i = 0; i < 2; i++) {
        bool f;
        nl::node CharGender;

        if (i == 0) {
            f = true;
            CharGender = mkinfo["CharFemale"];
        } else {
            f = false;
            CharGender = mkinfo["CharMale"];
        }

        for (const auto &node : CharGender) {
            int num = stoi(node.name());

            for (const auto &idnode : node) {
                int32_t value = idnode;

                switch (num) {
                    case 0: faces_[f].push_back(value); break;
                    case 1: hairs_[f].push_back(value); break;
                    case 2:
                        haircolors_[f].push_back(static_cast<uint8_t>(value));
                        break;
                    case 3:
                        skins_[f].push_back(static_cast<uint8_t>(value));
                        break;
                    case 4: tops_[f].push_back(value); break;
                    case 5: bots_[f].push_back(value); break;
                    case 6: shoes_[f].push_back(value); break;
                    case 7: weapons_[f].push_back(value); break;
                }
            }
        }
    }

    female_ = false;
    randomize_look();

    newchar_.set_direction(true);

    cloudfx_ = 200.0f;
}

void UIAranCreation::draw(float inter) const {
    for (size_t i = 0; i < 2; i++) {
        for (size_t k = 0; k < 800; k += sky_.width()) {
            sky_.draw(Point<int16_t>(k, (400 * i) - 100));
        }
    }

    int16_t cloudx = static_cast<int16_t>(cloudfx_) % 800;
    cloud_.draw(Point<int16_t>(cloudx - cloud_.width(), 310));
    cloud_.draw(Point<int16_t>(cloudx, 310));
    cloud_.draw(Point<int16_t>(cloudx + cloud_.width(), 310));

    if (!gender_) {
        for (size_t i = 0; i < sprites_gender_select_.size(); i++) {
            if (i == 1) {
                for (size_t f = 0; f <= 4; f++) {
                    sprites_gender_select_[i].draw(
                        position_ + Point<int16_t>(0, 24 * f),
                        inter);
                }
            } else {
                sprites_gender_select_[i].draw(position_, inter);
            }
        }

        UIElement::draw(inter);

        newchar_.draw(Point<int16_t>(394, 339), inter);
    } else {
        if (!char_set_) {
            UIElement::draw_sprites(inter);

            for (const auto &sprite : sprites_lookboard_) {
                sprite.draw(position_, inter);
            }

            facename_.draw(Point<int16_t>(647, 183 + (0 * 18)));
            hairname_.draw(Point<int16_t>(647, 183 + (1 * 18)));
            bodyname_.draw(Point<int16_t>(647, 183 + (2 * 18)));
            topname_.draw(Point<int16_t>(647, 183 + (3 * 18)));
            botname_.draw(Point<int16_t>(647, 183 + (4 * 18)));
            shoename_.draw(Point<int16_t>(647, 183 + (5 * 18)));
            wepname_.draw(Point<int16_t>(647, 183 + (6 * 18)));

            newchar_.draw(Point<int16_t>(394, 339), inter);

            UIElement::draw_buttons(inter);
        } else {
            if (!named_) {
                UIElement::draw_sprites(inter);

                nameboard_.draw(Point<int16_t>(489, 106));

                namechar_.draw(position_);
                newchar_.draw(Point<int16_t>(394, 339), inter);

                UIElement::draw_buttons(inter);
            } else {
                UIElement::draw_sprites(inter);

                nameboard_.draw(Point<int16_t>(489, 106));

                UIElement::draw_buttons(inter);

                for (const auto &sprite : sprites_keytype_) {
                    sprite.draw(position_, inter);
                }
            }
        }
    }

    version_.draw(position_ + Point<int16_t>(707, 1));
}

void UIAranCreation::update() {
    if (!gender_) {
        for (auto &sprite : sprites_gender_select_) {
            sprite.update();
        }

        newchar_.update(Constants::TIMESTEP);
    } else {
        if (!char_set_) {
            for (auto &sprite : sprites_lookboard_) {
                sprite.update();
            }

            newchar_.update(Constants::TIMESTEP);
        } else {
            if (!named_) {
                namechar_.update(position_);
                newchar_.update(Constants::TIMESTEP);
            } else {
                for (auto &sprite : sprites_keytype_) {
                    sprite.update();
                }

                namechar_.set_state(Textfield::State::DISABLED);
            }
        }
    }

    UIElement::update();

    cloudfx_ += 0.25f;
}

Cursor::State UIAranCreation::send_cursor(bool clicked,
                                          Point<int16_t> cursorpos) {
    if (namechar_.get_state() == Textfield::State::NORMAL) {
        if (namechar_.get_bounds().contains(cursorpos)) {
            if (clicked) {
                namechar_.set_state(Textfield::State::FOCUSED);

                return Cursor::State::CLICKING;
            }

            return Cursor::State::IDLE;
        }
    }

    return UIElement::send_cursor(clicked, cursorpos);
}

void UIAranCreation::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            button_pressed(Buttons::BT_CHARC_CANCEL);
        } else if (keycode == KeyAction::Id::RETURN) {
            button_pressed(Buttons::BT_CHARC_OK);
        }
    }
}

UIElement::Type UIAranCreation::get_type() const {
    return TYPE;
}

void UIAranCreation::send_naming_result(bool nameused) {
    if (!named_) {
        if (!nameused) {
            named_ = true;

            std::string cname = namechar_.get_text();
            int32_t cface = faces_[female_][face];
            int32_t chair = hairs_[female_][hair_];
            uint8_t chairc = haircolors_[female_][haircolor_];
            uint8_t cskin = skins_[female_][skin_];
            int32_t ctop = tops_[female_][top_];
            int32_t cbot = bots_[female_][bot_];
            int32_t cshoe = shoes_[female_][shoe_];
            int32_t cwep = weapons_[female_][weapon_];

            fn_create_char(cname,
                           2,
                           cface,
                           chair,
                           chairc,
                           cskin,
                           ctop,
                           cbot,
                           cshoe,
                           cwep,
                           female_);

            auto onok = [&](bool alternate) {
                Sound(Sound::Name::SCROLL_UP).play();

                UI::get().remove(UIElement::Type::LOGIN_NOTICE_CONFIRM);
                UI::get().remove(UIElement::Type::LOGIN_NOTICE);
                UI::get().remove(UIElement::Type::CLASS_CREATION);
                UI::get().remove(UIElement::Type::RACE_SELECT);

                if (auto charselect = UI::get().get_element<UICharSelect>()) {
                    charselect->get().post_add_character();
                }
            };

            UI::get().emplace<UIKeySelect>(onok, true);
        } else {
            auto onok = [&]() {
                namechar_.set_state(Textfield::State::FOCUSED);

                buttons_[Buttons::BT_CHARC_OK]->set_state(
                    Button::State::NORMAL);
                buttons_[Buttons::BT_CHARC_CANCEL]->set_state(
                    Button::State::NORMAL);
            };

            UI::get().emplace<UILoginNotice>(
                UILoginNotice::Message::NAME_IN_USE,
                onok);
        }
    }
}

Button::State UIAranCreation::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::BT_CHARC_OK:
            if (!gender_) {
                gender_ = true;

                buttons_[Buttons::BT_CHARC_GENDER_M]->set_active(false);
                buttons_[Buttons::BT_CHARC_GEMDER_F]->set_active(false);

                buttons_[Buttons::BT_CHARC_SKINL]->set_active(true);
                buttons_[Buttons::BT_CHARC_SKINR]->set_active(true);

                buttons_[Buttons::BT_CHARC_OK]->set_position(
                    Point<int16_t>(533, 368));
                buttons_[Buttons::BT_CHARC_CANCEL]->set_position(
                    Point<int16_t>(607, 368));

                return Button::State::NORMAL;
            } else {
                if (!char_set_) {
                    char_set_ = true;

                    buttons_[Buttons::BT_CHARC_SKINL]->set_active(false);
                    buttons_[Buttons::BT_CHARC_SKINR]->set_active(false);

                    buttons_[Buttons::BT_CHARC_OK]->set_position(
                        Point<int16_t>(523, 243));
                    buttons_[Buttons::BT_CHARC_CANCEL]->set_position(
                        Point<int16_t>(597, 243));

                    namechar_.set_state(Textfield::State::FOCUSED);

                    return Button::State::NORMAL;
                }

                if (!named_) {
                    std::string name = namechar_.get_text();

                    if (name.empty()) {
                        return Button::State::NORMAL;
                    }

                    if (name.size() >= 4) {
                        namechar_.set_state(Textfield::State::DISABLED);

                        buttons_[Buttons::BT_CHARC_OK]->set_state(
                            Button::State::DISABLED);
                        buttons_[Buttons::BT_CHARC_CANCEL]->set_state(
                            Button::State::DISABLED);

                        if (auto raceselect =
                                UI::get().get_element<UIRaceSelect>()) {
                            if (raceselect->get().check_name(name)) {
                                fn_name_char(name);

                                return Button::State::IDENTITY;
                            }
                        }

                        std::function<void()> okhandler = [&]() {
                            namechar_.set_state(Textfield::State::FOCUSED);

                            buttons_[Buttons::BT_CHARC_OK]->set_state(
                                Button::State::NORMAL);
                            buttons_[Buttons::BT_CHARC_CANCEL]->set_state(
                                Button::State::NORMAL);
                        };

                        UI::get().emplace<UILoginNotice>(
                            UILoginNotice::Message::ILLEGAL_NAME,
                            okhandler);

                        return Button::State::NORMAL;
                    }

                    namechar_.set_state(Textfield::State::DISABLED);

                    buttons_[Buttons::BT_CHARC_OK]->set_state(
                        Button::State::DISABLED);
                    buttons_[Buttons::BT_CHARC_CANCEL]->set_state(
                        Button::State::DISABLED);

                    std::function<void()> okhandler = [&]() {
                        namechar_.set_state(Textfield::State::FOCUSED);

                        buttons_[Buttons::BT_CHARC_OK]->set_state(
                            Button::State::NORMAL);
                        buttons_[Buttons::BT_CHARC_CANCEL]->set_state(
                            Button::State::NORMAL);
                    };

                    UI::get().emplace<UILoginNotice>(
                        UILoginNotice::Message::ILLEGAL_NAME,
                        okhandler);

                    return Button::State::IDENTITY;
                }

                return Button::State::NORMAL;
            }
        case BT_BACK:
            Sound(Sound::Name::SCROLL_UP).play();

            UI::get().remove(UIElement::Type::CLASS_CREATION);
            UI::get().emplace<UIRaceSelect>();

            return Button::State::NORMAL;
        case Buttons::BT_CHARC_CANCEL:
            if (char_set_) {
                char_set_ = false;

                buttons_[Buttons::BT_CHARC_SKINL]->set_active(true);
                buttons_[Buttons::BT_CHARC_SKINR]->set_active(true);

                buttons_[Buttons::BT_CHARC_OK]->set_position(
                    Point<int16_t>(533, 368));
                buttons_[Buttons::BT_CHARC_CANCEL]->set_position(
                    Point<int16_t>(607, 368));

                namechar_.set_state(Textfield::State::DISABLED);

                return Button::State::NORMAL;
            } else {
                if (gender_) {
                    gender_ = false;

                    buttons_[Buttons::BT_CHARC_GENDER_M]->set_active(true);
                    buttons_[Buttons::BT_CHARC_GEMDER_F]->set_active(true);

                    buttons_[Buttons::BT_CHARC_SKINL]->set_active(false);
                    buttons_[Buttons::BT_CHARC_SKINR]->set_active(false);

                    buttons_[Buttons::BT_CHARC_OK]->set_position(
                        Point<int16_t>(520, 397));
                    buttons_[Buttons::BT_CHARC_CANCEL]->set_position(
                        Point<int16_t>(594, 397));

                    return Button::State::NORMAL;
                }

                button_pressed(Buttons::BT_BACK);

                return Button::State::NORMAL;
            }
        case Buttons::BT_CHARC_SKINL:
            skin_ = (skin_ > 0) ? skin_ - 1 : skins_[female_].size() - 1;
            newchar_.set_body(skins_[female_][skin_]);
            bodyname_.change_text(newchar_.get_body()->get_name());

            return Button::State::NORMAL;
        case Buttons::BT_CHARC_SKINR:
            skin_ = (skin_ < skins_[female_].size() - 1) ? skin_ + 1 : 0;
            newchar_.set_body(skins_[female_][skin_]);
            bodyname_.change_text(newchar_.get_body()->get_name());

            return Button::State::NORMAL;
        case Buttons::BT_CHARC_GENDER_M:
            if (female_) {
                female_ = false;
                randomize_look();
            }

            return Button::State::NORMAL;
        case Buttons::BT_CHARC_GEMDER_F:
            if (!female_) {
                female_ = true;
                randomize_look();
            }

            return Button::State::NORMAL;
    }

    return Button::State::PRESSED;
}

void UIAranCreation::randomize_look() {
    hair_ = 0;
    face = 0;
    skin_ = randomizer_.next_int(skins_[female_].size());
    haircolor_ = 0;
    top_ = 0;
    bot_ = 0;
    shoe_ = 0;
    weapon_ = 0;

    newchar_.set_body(skins_[female_][skin_]);
    newchar_.set_face(faces_[female_][face]);
    newchar_.set_hair(hairs_[female_][hair_]
                      + haircolors_[female_][haircolor_]);
    newchar_.add_equip(tops_[female_][top_]);
    newchar_.add_equip(bots_[female_][bot_]);
    newchar_.add_equip(shoes_[female_][shoe_]);
    newchar_.add_equip(weapons_[female_][weapon_]);

    bodyname_.change_text(newchar_.get_body()->get_name());
    facename_.change_text(newchar_.get_face()->get_name());
    hairname_.change_text(newchar_.get_hair()->get_name());
    topname_.change_text(get_equipname(EquipSlot::Id::TOP));
    botname_.change_text(get_equipname(EquipSlot::Id::BOTTOM));
    shoename_.change_text(get_equipname(EquipSlot::Id::SHOES));
    wepname_.change_text(get_equipname(EquipSlot::Id::WEAPON));
}

const std::string &UIAranCreation::get_equipname(EquipSlot::Id slot) const {
    if (int32_t item_id = newchar_.get_equips().get_equip(slot)) {
        return ItemData::get(item_id).get_name();
    }

    static const std::string &nullstr = "Missing name.";

    return nullstr;
}
}  // namespace ms