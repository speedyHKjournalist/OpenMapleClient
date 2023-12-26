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
#include "UICharSelect.h"

#include "../../Audio/Audio.h"
#include "../../Character/Job.h"
#include "../../Configuration.h"
#include "../../Net/Packets/SelectCharPackets.h"
#include "../Components/AreaButton.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "UILoginNotice.h"
#include "UIRaceSelect.h"
#include "UISoftKey.h"
#include "UIWorldSelect.h"

#define NOMINMAX
#ifdef WIN32
#include <windows.h>
#endif

#include <nlnx/nx.hpp>
#include <utility>

namespace ms {
namespace {
auto fn_select_char_pic = []<typename... T>(T && ...args) {
    SelectCharPicPacket(std::forward<T>(args)...).dispatch();
};

auto fn_select_char = []<typename... T>(T && ...args) {
    SelectCharPacket(std::forward<T>(args)...).dispatch();
};

auto fn_delete_char_pic = []<typename... T>(T && ...args) {
    DeleteCharPicPacket(std::forward<T>(args)...).dispatch();
};

auto fn_delete_char = []<typename... T>(T && ...args) {
    DeleteCharPacket(std::forward<T>(args)...).dispatch();
};

auto fn_register_pic = []<typename... T>(T && ...args) {
    RegisterPicPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UICharSelect::UICharSelect(std::vector<CharEntry> c,
                           int8_t char_count,
                           int32_t s,
                           int8_t rp) :
    UIElement(Point<int16_t>(0, 0), Point<int16_t>(800, 600)),
    characters_(std::move(c)),
    characters_count_(char_count),
    slots_(s),
    require_pic_(rp) {
    burning_character_ = true;

    std::string version_text = Configuration::get().get_version();
    version_ = Text(Text::Font::A11M,
                    Text::Alignment::LEFT,
                    Color::Name::LEMONGRASS,
                    "Ver. " + version_text);

    page_pos_ = Point<int16_t>(247, 462);
    world_pos_ = Point<int16_t>(586, 46);
    charinfo_pos_ = Point<int16_t>(671, 339);

    Point<int16_t> character_sel_pos = Point<int16_t>(601, 393);
    Point<int16_t> character_new_pos = Point<int16_t>(200, 495);
    Point<int16_t> character_del_pos = Point<int16_t>(316, 495);

    selected_character_ = Setting<DefaultCharacter>::get().load();
    selected_page = selected_character_ / PAGESIZE_;
    page_count_ = std::ceil((double)slots_ / (double)PAGESIZE_);

    tab_ = nl::nx::ui["Basic.img"]["Cursor"]["18"]["0"];

    tab_index_ = 0;
    tab_active_ = false;
    tab_move_ = false;

    Point<int16_t> tab_adj = Point<int16_t>(86, 5);

    tab_pos_[0] = character_sel_pos + tab_adj + Point<int16_t>(47, 3);
    tab_pos_[1] = character_new_pos + tab_adj;
    tab_pos_[2] = character_del_pos + tab_adj;

    tab_move_pos_ = 0;

    tab_map_[0] = Buttons::CHARACTER_SELECT;
    tab_map_[1] = Buttons::CHARACTER_NEW;
    tab_map_[2] = Buttons::CHARACTER_DELETE;

    nl::node Login = nl::nx::ui["Login.img"];
    nl::node Common = Login["Common"];
    nl::node CharSelect = Login["CharSelect"];
    nl::node selectWorld = Common["selectWorld"];
    nl::node selectedWorld = CharSelect["selectedWorld"];
    nl::node pageNew = CharSelect["pageNew"];

    world_dimensions_ = Texture(selectWorld).get_dimensions();

    uint16_t world = 0;
    uint8_t world_id = Configuration::get().get_worldid();
    uint8_t channel_id = Configuration::get().get_channelid();

    if (auto worldselect = UI::get().get_element<UIWorldSelect>()) {
        world = worldselect->get().get_worldbyid(world_id);
    }

    world_sprites_.emplace_back(selectWorld, world_pos_);
    world_sprites_.emplace_back(selectedWorld["icon"][world],
                                world_pos_ - Point<int16_t>(12, -1));
    world_sprites_.emplace_back(selectedWorld["name"][world],
                                world_pos_ - Point<int16_t>(8, 1));
    world_sprites_.emplace_back(selectedWorld["ch"][channel_id],
                                world_pos_ - Point<int16_t>(0, 1));

    nl::node map = nl::nx::map001["Back"]["login.img"];
    nl::node ani = map["ani"];

    sprites_.emplace_back(map["back"]["13"], Point<int16_t>(392, 297));
    sprites_.emplace_back(ani["17"], Point<int16_t>(151, 283));
    sprites_.emplace_back(ani["18"], Point<int16_t>(365, 252));
    sprites_.emplace_back(ani["19"], Point<int16_t>(191, 208));
    sprites_.emplace_back(Common["frame"], Point<int16_t>(400, 300));
    sprites_.emplace_back(Common["step"]["2"], Point<int16_t>(40, 0));

    burning_notice_ = Common["Burning"]["BurningNotice"];
    burning_count_ =
        Text(Text::Font::A12B, Text::Alignment::LEFT, Color::Name::WHITE, "1");

    charinfo_ = CharSelect["charInfo"];
    charslot_ = CharSelect["charSlot"]["0"];
    pagebase_ = pageNew["base"]["0"];
    pagenumber_ = Charset(pageNew["number"], Charset::Alignment::LEFT);
    pagenumber_pos_ = pageNew["numberpos"];

    sign_post_[0] = CharSelect["adventure"]["0"];
    sign_post_[1] = CharSelect["knight"]["0"];
    sign_post_[2] = CharSelect["aran"]["0"];

    nametag_ = CharSelect["nameTag"];

    buttons_[Buttons::CHARACTER_SELECT] =
        std::make_unique<MapleButton>(CharSelect["BtSelect"],
                                      character_sel_pos);
    buttons_[Buttons::CHARACTER_NEW] =
        std::make_unique<MapleButton>(CharSelect["BtNew"], character_new_pos);
    buttons_[Buttons::CHARACTER_DELETE] =
        std::make_unique<MapleButton>(CharSelect["BtDelete"],
                                      character_del_pos);
    buttons_[Buttons::PAGE_LEFT] =
        std::make_unique<MapleButton>(CharSelect["pageL"],
                                      Point<int16_t>(98, 491));
    buttons_[Buttons::PAGE_RIGHT] =
        std::make_unique<MapleButton>(CharSelect["pageR"],
                                      Point<int16_t>(485, 491));
    buttons_[Buttons::CHANGE_PIC] =
        std::make_unique<MapleButton>(Common["BtChangePIC"],
                                      Point<int16_t>(0, 45));
    buttons_[Buttons::RESET_PIC] =
        std::make_unique<MapleButton>(Login["WorldSelect"]["BtResetPIC"],
                                      Point<int16_t>(0, 85));
    buttons_[Buttons::EDIT_CHAR_LIST] =
        std::make_unique<MapleButton>(CharSelect["EditCharList"]["BtCharacter"],
                                      Point<int16_t>(-1, 118));
    buttons_[Buttons::BACK] =
        std::make_unique<MapleButton>(Common["BtStart"],
                                      Point<int16_t>(0, 515));

    for (size_t i = 0; i < PAGESIZE_; i++) {
        buttons_[Buttons::CHARACTER_SLOT0 + i] =
            std::make_unique<AreaButton>(get_character_slot_pos(i, 105, 144),
                                         Point<int16_t>(50, 90));
    }

    if (require_pic_ == 0) {
        buttons_[Buttons::CHANGE_PIC]->set_active(false);
        buttons_[Buttons::RESET_PIC]->set_active(false);
    }

    level_set_ = Charset(CharSelect["lv"], Charset::Alignment::CENTER);
    name_label_ = OutlinedText(Text::Font::A15B,
                               Text::Alignment::CENTER,
                               Color::Name::WHITE,
                               Color::Name::IRISHCOFFEE);

    for (size_t i = 0; i < InfoLabel::NUM_LABELS; i++) {
        info_labels_[i] = OutlinedText(Text::Font::A11M,
                                       Text::Alignment::RIGHT,
                                       Color::Name::WHITE,
                                       Color::Name::TOBACCOBROWN);
    }

    for (auto &entry : characters_) {
        char_looks_.emplace_back(entry.look);
        nametags_.emplace_back(nametag_, Text::Font::A13M, entry.stats.name);
    }

    empty_slot_effect_ = CharSelect["character"]["0"];
    empty_slot_ = CharSelect["character"]["1"]["0"];

    selected_slot_effect_[0] = CharSelect["effect"][0];
    selected_slot_effect_[1] = CharSelect["effect"][1];

    chat_slot_label_ = OutlinedText(Text::Font::A12M,
                                    Text::Alignment::LEFT,
                                    Color::Name::PORCELAIN,
                                    Color::Name::BROWNDERBY);
    chat_slot_label_.change_text(get_slot_text());

    update_buttons();

    if (characters_count_ > 0) {
        if (selected_character_ < characters_count_) {
            update_selected_character();
        } else {
            select_last_slot();
        }
    }

    if (Configuration::get().get_auto_login()) {
        fn_select_char_pic(Configuration::get().get_auto_pic(),
                           Configuration::get().get_auto_cid());
    }

    dimension_ = Point<int16_t>(800, 600);
}

void UICharSelect::draw(float inter) const {
    UIElement::draw_sprites(inter);

    version_.draw(position_ + Point<int16_t>(707, 1));
    charslot_.draw(position_ + Point<int16_t>(589, 106 - charslot_y_));
    chat_slot_label_.draw(position_ + Point<int16_t>(700, 110 - charslot_y_));

    for (const Sprite &sprite : world_sprites_) {
        sprite.draw(position_, inter);
    }

    std::string total = pad_number_with_leading_zero(page_count_);
    std::string current = pad_number_with_leading_zero(selected_page + 1);

    std::list<uint8_t> fliplist = { 2, 3, 6, 7 };

    for (int i = 0; i < PAGESIZE_; i++) {
        auto index = i + selected_page * PAGESIZE_;
        bool flip_character =
            std::find(fliplist.begin(), fliplist.end(), i) != fliplist.end();
        bool selectedslot = index == selected_character_;

        if (index < characters_count_) {
            Point<int16_t> charpos = get_character_slot_pos(i, 135, 234);
            DrawArgument chararg = DrawArgument(charpos, flip_character);

            nametags_[index].draw(charpos);

            const StatsEntry &character_stats = characters_[index].stats;

            if (selectedslot) {
                selected_slot_effect_[1].draw(charpos + Point<int16_t>(-5, 16),
                                              inter);

                int8_t lvy = -115;
                Point<int16_t> pos_adj = Point<int16_t>(668, 365);

                charinfo_.draw(position_ + charinfo_pos_);

                std::string levelstr =
                    std::to_string(character_stats.stats[MapleStat::Id::LEVEL]);
                int16_t lvx =
                    level_set_.draw(levelstr,
                                    pos_adj + Point<int16_t>(12, lvy));
                level_set_.draw('l',
                                pos_adj + Point<int16_t>(1 - lvx / 2, lvy));

                name_label_.draw(pos_adj + Point<int16_t>(8, -106));

                for (size_t i = 0; i < InfoLabel::NUM_LABELS; i++) {
                    Point<int16_t> labelpos = pos_adj + get_infolabel_pos(i);
                    info_labels_[i].draw(labelpos);
                }
            }

            uint8_t j = 0;
            uint16_t job = character_stats.stats[MapleStat::Id::JOB];

            if (job >= 0 && job < 1000) {
                j = 0;
            } else if (job >= 1000 && job < 2000) {
                j = 1;
            } else if (job >= 2000 && job < 2200) {
                j = 2;
            } else {
                j = 0;
            }

            sign_post_[j].draw(chararg);
            char_looks_[index].draw(chararg, inter);

            if (selectedslot) {
                selected_slot_effect_[0].draw(
                    charpos + Point<int16_t>(-5, -298),
                    inter);
            }
        } else if (i < slots_) {
            Point<int16_t> emptyslotpos = get_character_slot_pos(i, 130, 234);

            empty_slot_effect_.draw(emptyslotpos, inter);
            empty_slot_.draw(DrawArgument(emptyslotpos, flip_character));
        }
    }

    UIElement::draw_buttons(inter);

    if (tab_active_) {
        tab_.draw(position_ + tab_pos_[tab_index_]
                  + Point<int16_t>(0, tab_move_pos_));
    }

    if (burning_character_) {
        burning_notice_.draw(position_ + Point<int16_t>(190, 502), inter);
        burning_count_.draw(position_ + Point<int16_t>(149, 464));
    }

    pagebase_.draw(position_ + page_pos_);
    pagenumber_.draw(
        current.substr(0, 1),
        position_ + page_pos_ + Point<int16_t>(pagenumber_pos_[0]));
    pagenumber_.draw(
        current.substr(1, 1),
        position_ + page_pos_ + Point<int16_t>(pagenumber_pos_[1]));
    pagenumber_.draw(
        total.substr(0, 1),
        position_ + page_pos_ + Point<int16_t>(pagenumber_pos_[2]));
    pagenumber_.draw(
        total.substr(1, 1),
        position_ + page_pos_ + Point<int16_t>(pagenumber_pos_[3]));
}

void UICharSelect::update() {
    UIElement::update();

    if (show_timestamp_) {
        if (timestamp_ > 0) {
            timestamp_ -= Constants::TIMESTEP;

            if (timestamp_ <= 176) {
                charslot_y_ += 1;
            }
        }
    } else {
        if (timestamp_ <= 176) {
            timestamp_ += Constants::TIMESTEP;

            if (charslot_y_ >= 0) {
                charslot_y_ -= 1;
            }
        }
    }

    if (tab_move_ && tab_move_pos_ < 4) {
        tab_move_pos_ += 1;
    }

    if (tab_move_ && tab_move_pos_ == 4) {
        tab_move_ = false;
    }

    if (!tab_move_ && tab_move_pos_ > 0) {
        tab_move_pos_ -= 1;
    }

    for (auto &charlook : char_looks_) {
        charlook.update(Constants::TIMESTEP);
    }

    for (auto &effect : selected_slot_effect_) {
        effect.update();
    }

    empty_slot_effect_.update();

    if (burning_character_) {
        burning_notice_.update();
    }
}

void UICharSelect::doubleclick(Point<int16_t> cursorpos) {
    uint16_t button_index = selected_character_ + Buttons::CHARACTER_SLOT0;
    auto &btit = buttons_[button_index];

    if (btit->is_active() && btit->bounds(position_).contains(cursorpos)
        && btit->get_state() == Button::State::NORMAL
        && button_index >= Buttons::CHARACTER_SLOT0) {
        button_pressed(Buttons::CHARACTER_SELECT);
    }
}

Cursor::State UICharSelect::send_cursor(bool clicked,
                                        Point<int16_t> cursorpos) {
    Rectangle<int16_t> charslot_bounds =
        Rectangle<int16_t>(world_pos_, world_pos_ + world_dimensions_);

    if (charslot_bounds.contains(cursorpos)) {
        if (clicked) {
            show_timestamp_ = !show_timestamp_;

            return Cursor::State::CLICKING;
        }
    }

    Cursor::State ret = clicked ? Cursor::State::CLICKING : Cursor::State::IDLE;

    for (auto &[btnid, button] : buttons_) {
        if (button->is_active()
            && button->bounds(position_).contains(cursorpos)) {
            if (button->get_state() == Button::State::NORMAL) {
                Sound(Sound::Name::BUTTON_OVER).play();

                button->set_state(Button::State::MOUSEOVER);
                ret = Cursor::State::CAN_CLICK;
            } else if (button->get_state() == Button::State::MOUSEOVER) {
                if (clicked) {
                    Sound(Sound::Name::BUTTON_CLICK).play();

                    button->set_state(button_pressed(btnid));

                    if (tab_active_ && btnid == tab_map_[tab_index_]) {
                        button->set_state(Button::State::MOUSEOVER);
                    }

                    ret = Cursor::State::IDLE;
                } else {
                    if (!tab_active_ || btnid != tab_map_[tab_index_]) {
                        ret = Cursor::State::CAN_CLICK;
                    }
                }
            }
        } else if (button->get_state() == Button::State::MOUSEOVER) {
            if (!tab_active_ || btnid != tab_map_[tab_index_]) {
                button->set_state(Button::State::NORMAL);
            }
        }
    }

    return ret;
}

void UICharSelect::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            button_pressed(Buttons::BACK);
        } else if (keycode == KeyAction::Id::RETURN) {
            if (tab_active_) {
                uint16_t btn_index = tab_map_[tab_index_];

                auto &btn = buttons_[btn_index];
                auto state = btn->get_state();

                if (state != Button::State::DISABLED) {
                    button_pressed(btn_index);
                }
            } else {
                button_pressed(Buttons::CHARACTER_SELECT);
            }
        } else {
            uint8_t selected_index = selected_character_;
            uint8_t index_total =
                std::min(characters_count_,
                         static_cast<int8_t>((selected_page + 1) * PAGESIZE_));

            uint8_t COLUMNS = 4;
            uint8_t columns = std::min(index_total, COLUMNS);

            uint8_t rows = std::floor((index_total - 1) / COLUMNS) + 1;

            div_t div = std::div(selected_index, columns);
            int32_t current_col = div.rem;
            // int32_t current_row = div.quot;

            if (keycode == KeyAction::Id::UP) {
                auto next_index =
                    (selected_index - COLUMNS < 0
                         ? (selected_index - COLUMNS) + rows * COLUMNS
                         : selected_index - COLUMNS);

                if (next_index == selected_character_) {
                    return;
                }

                if (next_index >= index_total) {
                    button_pressed(next_index - COLUMNS
                                   + Buttons::CHARACTER_SLOT0);
                } else {
                    button_pressed(next_index + Buttons::CHARACTER_SLOT0);
                }
            } else if (keycode == KeyAction::Id::DOWN) {
                auto next_index = (selected_index + COLUMNS >= index_total
                                       ? current_col
                                       : selected_index + COLUMNS);

                if (next_index == selected_character_) {
                    return;
                }

                if (next_index > index_total) {
                    button_pressed(next_index + COLUMNS
                                   + Buttons::CHARACTER_SLOT0);
                } else {
                    button_pressed(next_index + Buttons::CHARACTER_SLOT0);
                }
            } else if (keycode == KeyAction::Id::LEFT) {
                if (selected_index != 0) {
                    selected_index--;

                    if (selected_index
                        >= (selected_page + 1) * PAGESIZE_ - PAGESIZE_) {
                        button_pressed(selected_index
                                       + Buttons::CHARACTER_SLOT0);
                    } else {
                        button_pressed(Buttons::PAGE_LEFT);
                    }
                }
            } else if (keycode == KeyAction::Id::RIGHT) {
                if (selected_index != characters_count_ - 1) {
                    selected_index++;

                    if (selected_index < index_total) {
                        button_pressed(selected_index
                                       + Buttons::CHARACTER_SLOT0);
                    } else {
                        button_pressed(Buttons::PAGE_RIGHT);
                    }
                }
            } else if (keycode == KeyAction::Id::TAB) {
                uint8_t prev_tab = tab_index_;

                if (!tab_active_) {
                    tab_active_ = true;

                    if (!buttons_[Buttons::CHARACTER_SELECT]->is_active()) {
                        tab_index_++;
                    }
                } else {
                    tab_index_++;

                    if (tab_index_ > 2) {
                        tab_active_ = false;
                        tab_index_ = 0;
                    }
                }

                tab_move_ = true;
                tab_move_pos_ = 0;

                auto &prev_btn = buttons_[tab_map_[prev_tab]];
                auto prev_state = prev_btn->get_state();

                if (prev_state != Button::State::DISABLED) {
                    prev_btn->set_state(Button::State::NORMAL);
                }

                if (tab_active_) {
                    auto &btn = buttons_[tab_map_[tab_index_]];
                    auto state = btn->get_state();

                    if (state != Button::State::DISABLED) {
                        btn->set_state(Button::State::MOUSEOVER);
                    }
                }
            }
        }
    }
}

UIElement::Type UICharSelect::get_type() const {
    return TYPE;
}

void UICharSelect::add_character(CharEntry &&character) {
    char_looks_.emplace_back(character.look);
    nametags_.emplace_back(nametag_, Text::Font::A13M, character.stats.name);
    characters_.emplace_back(std::forward<CharEntry>(character));

    characters_count_++;
}

void UICharSelect::post_add_character() {
    bool page_matches = (characters_count_ - 1) / PAGESIZE_ == selected_page;

    if (!page_matches) {
        button_pressed(Buttons::PAGE_RIGHT);
    }

    update_buttons();

    if (characters_count_ > 1) {
        select_last_slot();
    } else {
        update_selected_character();
    }

    makeactive();

    chat_slot_label_.change_text(get_slot_text());
}

void UICharSelect::remove_character(int32_t id) {
    for (size_t i = 0; i < characters_.size(); i++) {
        if (characters_[i].id == id) {
            char_looks_.erase(char_looks_.begin() + i);
            nametags_.erase(nametags_.begin() + i);
            characters_.erase(characters_.begin() + i);

            characters_count_--;

            if (selected_page > 0) {
                bool page_matches =
                    (characters_count_ - 1) / PAGESIZE_ == selected_page;

                if (!page_matches) {
                    button_pressed(Buttons::PAGE_LEFT);
                }
            }

            update_buttons();

            if (selected_character_ < characters_count_) {
                update_selected_character();
            } else {
                select_last_slot();
            }

            return;
        }
    }
}

const CharEntry &UICharSelect::get_character(int32_t id) {
    for (auto &character : characters_) {
        if (character.id == id) {
            return character;
        }
    }

    std::cout << "Invalid character id: [" << id << "]" << std::endl;

    static const CharEntry null_character = { {}, {}, 0 };

    return null_character;
}

bool UICharSelect::update_character(int32_t id, const StatsEntry &stats) {
    for (auto &character : characters_) {
        if (character.id == id) {
            character.stats = stats;
            return true;
        }
    }

    return false;
}

Button::State UICharSelect::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::CHARACTER_SELECT: {
            Setting<DefaultCharacter>::get().save(selected_character_);
            int32_t id = characters_[selected_character_].id;

            switch (require_pic_) {
                case 0: {
                    std::function<void()> onok = [&]() { request_pic(); };

                    UI::get().emplace<UILoginNotice>(
                        UILoginNotice::Message::PIC_REQ,
                        onok);
                } break;
                case 1: {
                    std::function<void(const std::string &)> onok =
                        [id](const std::string &pic) {
                            fn_select_char_pic(pic, id);
                        };

                    UI::get().emplace<UISoftKey>(onok);
                } break;
                case 2: fn_select_char(id); break;
            }
        } break;
        case Buttons::CHARACTER_NEW:
            Sound(Sound::Name::SCROLL_UP).play();

            deactivate();

            tab_index_ = 0;
            tab_active_ = false;
            tab_move_ = false;
            tab_move_pos_ = 0;

            UI::get().emplace<UIRaceSelect>();
            break;
        case Buttons::CHARACTER_DELETE: {
            int32_t id = characters_[selected_character_].id;

            switch (require_pic_) {
                case 0: {
                    std::function<void()> onok = [&]() {
                        chat_slot_label_.change_text(get_slot_text());
                    };

                    UI::get().emplace<UILoginNotice>(
                        UILoginNotice::Message::CHAR_DEL_FAIL_NO_PIC,
                        onok);
                } break;
                case 1: {
                    std::function<void()> oncancel = [&]() {
                        chat_slot_label_.change_text(get_slot_text());
                    };

                    std::function<void()> onok = [&, id, oncancel]() {
                        std::function<void(const std::string &)> onok =
                            [&, id](const std::string &pic) {
                                fn_delete_char_pic(pic, id);
                                chat_slot_label_.change_text(get_slot_text());
                            };

                        UI::get().emplace<UISoftKey>(onok, oncancel);
                    };

                    const StatsEntry &character_stats =
                        characters_[selected_character_].stats;
                    uint16_t cjob = character_stats.stats[MapleStat::Id::JOB];

                    if (cjob < 1000) {
                        UI::get().emplace<UILoginNotice>(
                            UILoginNotice::Message::DELETE_CONFIRMATION,
                            onok,
                            oncancel);
                    } else {
                        UI::get().emplace<UILoginNotice>(
                            UILoginNotice::Message::CASH_ITEMS_CONFIRM_DELETION,
                            onok);
                    }
                } break;
                case 2:
                    fn_delete_char(id);
                    chat_slot_label_.change_text(get_slot_text());
                    break;
            }
        } break;
        case Buttons::PAGE_LEFT: {
            uint8_t previous_page = selected_page;

            if (selected_page > 0) {
                selected_page--;
            } else if (characters_count_ > PAGESIZE_) {
                selected_page = page_count_ - 1;
            }

            if (previous_page != selected_page) {
                update_buttons();
            }

            select_last_slot();
        } break;
        case Buttons::PAGE_RIGHT: {
            uint8_t previous_page = selected_page;

            if (selected_page < page_count_ - 1) {
                selected_page++;
            } else {
                selected_page = 0;
            }

            if (previous_page != selected_page) {
                update_buttons();

                button_pressed(Buttons::CHARACTER_SLOT0);
            }
        } break;
        case Buttons::CHANGE_PIC: break;
        case Buttons::RESET_PIC: {
            std::string url = Configuration::get().get_resetpic();

            // TODO: (rich) fix
            // ShellExecute(NULL, "open", url.c_str(), NULL, NULL,
            // SW_SHOWNORMAL);
        } break;
        case Buttons::EDIT_CHAR_LIST: break;
        case Buttons::BACK:
            deactivate();

            Sound(Sound::Name::SCROLL_UP).play();

            if (auto worldselect = UI::get().get_element<UIWorldSelect>()) {
                worldselect->get().makeactive();
            }

            break;
        default:
            if (buttonid >= Buttons::CHARACTER_SLOT0) {
                uint8_t previous_character = selected_character_;
                selected_character_ = buttonid - Buttons::CHARACTER_SLOT0
                                      + selected_page * PAGESIZE_;

                if (previous_character != selected_character_) {
                    if (previous_character < characters_count_) {
                        char_looks_[previous_character].set_stance(
                            Stance::Id::STAND1);
                        nametags_[previous_character].set_selected(false);
                    }

                    if (selected_character_ < characters_count_) {
                        update_selected_character();
                    }
                }
            }

            break;
    }

    return Button::State::NORMAL;
}

void UICharSelect::update_buttons() {
    for (int i = 0; i < PAGESIZE_; i++) {
        auto index = i + selected_page * PAGESIZE_;

        if (index < characters_count_) {
            buttons_[Buttons::CHARACTER_SLOT0 + i]->set_state(
                Button::State::NORMAL);
        } else {
            buttons_[Buttons::CHARACTER_SLOT0 + i]->set_state(
                Button::State::DISABLED);
        }
    }

    if (characters_count_ >= slots_) {
        buttons_[Buttons::CHARACTER_NEW]->set_state(Button::State::DISABLED);
    } else {
        buttons_[Buttons::CHARACTER_NEW]->set_state(Button::State::NORMAL);
    }

    bool character_found = false;

    for (int i = PAGESIZE_ - 1; i >= 0; i--) {
        int index = i + selected_page * PAGESIZE_;

        if (index < characters_count_) {
            character_found = true;

            break;
        }
    }

    buttons_[Buttons::CHARACTER_SELECT]->set_active(character_found);
    buttons_[Buttons::CHARACTER_DELETE]->set_state(
        character_found ? Button::State::NORMAL : Button::State::DISABLED);
}

void UICharSelect::update_selected_character() {
    Sound(Sound::Name::CHAR_SELECT).play();

    char_looks_[selected_character_].set_stance(Stance::Id::WALK1);
    nametags_[selected_character_].set_selected(true);

    const StatsEntry &character_stats = characters_[selected_character_].stats;

    name_label_.change_text(character_stats.name);

    for (size_t i = 0; i < InfoLabel::NUM_LABELS; i++) {
        info_labels_[i].change_text(get_infolabel(i, character_stats));
    }
}

void UICharSelect::select_last_slot() {
    for (int i = PAGESIZE_ - 1; i >= 0; i--) {
        auto index = i + selected_page * PAGESIZE_;

        if (index < characters_count_) {
            button_pressed(i + Buttons::CHARACTER_SLOT0);

            return;
        }
    }
}

std::string UICharSelect::get_slot_text() {
    show_timestamp_ = true;
    timestamp_ = 7 * 1000;
    charslot_y_ = 0;

    return pad_number_with_leading_zero(characters_count_) + "/"
           + pad_number_with_leading_zero(slots_);
}

std::string UICharSelect::pad_number_with_leading_zero(uint8_t value) const {
    std::string return_val = std::to_string(value);
    return_val.insert(return_val.begin(), 2 - return_val.length(), '0');

    return return_val;
}

Point<int16_t> UICharSelect::get_character_slot_pos(size_t index,
                                                    uint16_t x_adj,
                                                    uint16_t y_adj) const {
    int16_t x = 125 * (index % 4);
    int16_t y = 200 * (index > 3);

    return Point<int16_t>(x + x_adj, y + y_adj);
}

Point<int16_t> UICharSelect::get_infolabel_pos(size_t index) const {
    switch (index) {
        case InfoLabel::JOB: return Point<int16_t>(66, -74);
        case InfoLabel::STR: return Point<int16_t>(-6, -50);
        case InfoLabel::DEX: return Point<int16_t>(-6, -27);
        case InfoLabel::INT: return Point<int16_t>(65, -50);
        case InfoLabel::LUK: return Point<int16_t>(65, -27);
        case InfoLabel::NUM_LABELS: break;
        default: break;
    }

    return Point<int16_t>();
}

std::string UICharSelect::get_infolabel(size_t index,
                                        StatsEntry character_stats) const {
    switch (index) {
        case InfoLabel::JOB:
            return Job(character_stats.stats[MapleStat::Id::JOB]).get_name();
        case InfoLabel::STR:
            return std::to_string(character_stats.stats[MapleStat::Id::STR]);
        case InfoLabel::DEX:
            return std::to_string(character_stats.stats[MapleStat::Id::DEX]);
        case InfoLabel::INT:
            return std::to_string(character_stats.stats[MapleStat::Id::INT]);
        case InfoLabel::LUK:
            return std::to_string(character_stats.stats[MapleStat::Id::LUK]);
        case InfoLabel::NUM_LABELS: break;
        default: break;
    }

    return "";
}

void UICharSelect::request_pic() {
    std::function<void(const std::string &)> enterpic =
        [&](const std::string &entered_pic) {
            std::function<void(const std::string &)> verifypic =
                [&, entered_pic](const std::string &verify_pic) {
                    if (entered_pic == verify_pic) {
                        check_pic(entered_pic);
                    } else {
                        std::function<void()> onreturn = [&]() {
                            request_pic();
                        };

                        UI::get().emplace<UILoginNotice>(
                            UILoginNotice::Message::PASSWORD_IS_INCORRECT,
                            onreturn);
                    }
                };

            UI::get().emplace<UISoftKey>(
                verifypic,
                []() {},
                "Please re-enter your new PIC.",
                Point<int16_t>(24, 0));
        };

    UI::get().emplace<UISoftKey>(
        enterpic,
        []() {},
        "Your new PIC must at least be 6 characters long.",
        Point<int16_t>(24, 0));
}

void UICharSelect::check_pic(const std::string &entered_pic) const {
    const char *pStr = entered_pic.c_str();

    if (pStr == nullptr) {
        return;
    }

    int count = 0;
    char m = ' ';
    bool reptitive = false;

    while (*pStr) {
        if (*pStr == m) {
            count++;
        } else {
            count = 0;
            m = *pStr;
        }

        if (count > 2) {
            reptitive = true;
            break;
        }

        pStr++;
    }

    if (reptitive) {
        UI::get().emplace<UILoginNotice>(UILoginNotice::Message::PIC_REPITIVE);
    } else {
        fn_register_pic(characters_[selected_character_].id, entered_pic);
    }
}
}  // namespace ms