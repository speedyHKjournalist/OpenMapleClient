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
#include "UINpcTalk.h"

#include <nlnx/nx.hpp>

#include "../Components/MapleButton.h"
#include "../Gameplay/Stage.h"
#include "../Net/Packets/NpcInteractionPackets.h"
#include "../UI.h"
#include "GameInfo.h"

namespace ms {
namespace {
auto fn_npc_talk_more = []<typename... T>(T && ...args) {
    NpcTalkMorePacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UINpcTalk::UINpcTalk() :
    offset_(0),
    unit_rows_(0),
    row_max_(0),
    show_slider_(false),
    draw_text_(false),
    formatted_text_(""),
    formatted_text_pos_(0),
    timestep_(0) {
    nl::node UIWindow2 = nl::nx::ui["UIWindow2.img"];
    nl::node UtilDlgEx = UIWindow2["UtilDlgEx"];

    top_ = UtilDlgEx["t"];
    fill_ = UtilDlgEx["c"];
    bottom_ = UtilDlgEx["s"];
    nametag_ = UtilDlgEx["bar"];

    min_height_ = 8 * fill_.height() + 14;

    buttons_[Buttons::ALL_LEVEL] =
        std::make_unique<MapleButton>(UtilDlgEx["BtAllLevel"]);
    buttons_[Buttons::CLOSE] =
        std::make_unique<MapleButton>(UtilDlgEx["BtClose"]);
    buttons_[Buttons::MY_LEVEL] =
        std::make_unique<MapleButton>(UtilDlgEx["BtMyLevel"]);
    buttons_[Buttons::NEXT] =
        std::make_unique<MapleButton>(UtilDlgEx["BtNext"]);

    // TODO: Replace when _inlink is fixed
    // buttons[Buttons::NO] = std::make_unique<MapleButton>(UtilDlgEx["BtNo"]);

    nl::node Quest = UIWindow2["Quest"];

    buttons_[Buttons::NO] = std::make_unique<MapleButton>(Quest["BtNo"]);
    buttons_[Buttons::OK] = std::make_unique<MapleButton>(UtilDlgEx["BtOK"]);
    buttons_[Buttons::PREV] =
        std::make_unique<MapleButton>(UtilDlgEx["BtPrev"]);
    buttons_[Buttons::QAFTER] =
        std::make_unique<MapleButton>(UtilDlgEx["BtQAfter"]);
    buttons_[Buttons::QCNO] =
        std::make_unique<MapleButton>(UtilDlgEx["BtQCNo"]);
    buttons_[Buttons::QCYES] =
        std::make_unique<MapleButton>(UtilDlgEx["BtQCYes"]);
    buttons_[Buttons::QGIVEUP] =
        std::make_unique<MapleButton>(UtilDlgEx["BtQGiveup"]);
    buttons_[Buttons::QNO] = std::make_unique<MapleButton>(UtilDlgEx["BtQNo"]);
    buttons_[Buttons::QSTART] =
        std::make_unique<MapleButton>(UtilDlgEx["BtQStart"]);
    buttons_[Buttons::QYES] =
        std::make_unique<MapleButton>(UtilDlgEx["BtQYes"]);
    buttons_[Buttons::YES] = std::make_unique<MapleButton>(UtilDlgEx["BtYes"]);

    name_ = Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::WHITE);

    onmoved_ = [&](bool upwards) {
        int16_t shift = upwards ? -unit_rows_ : unit_rows_;
        bool above = offset_ + shift >= 0;
        bool below = offset_ + shift <= row_max_ - unit_rows_;

        if (above && below) {
            offset_ += shift;
        }
    };

    UI::get().remove_textfield();
}

void UINpcTalk::draw(float inter) const {
    Point<int16_t> drawpos = position_;
    top_.draw(drawpos);
    drawpos.shift_y(top_.height());
    fill_.draw(DrawArgument(drawpos, Point<int16_t>(0, height_)));
    drawpos.shift_y(height_);
    bottom_.draw(drawpos);
    drawpos.shift_y(bottom_.height());

    UIElement::draw(inter);

    int16_t speaker_y = (top_.height() + height_ + bottom_.height()) / 2;
    Point<int16_t> speaker_pos = position_ + Point<int16_t>(22, 11 + speaker_y);
    Point<int16_t> center_pos =
        speaker_pos + Point<int16_t>(nametag_.width() / 2, 0);

    speaker_.draw(DrawArgument(center_pos, true));
    nametag_.draw(speaker_pos);
    name_.draw(center_pos + Point<int16_t>(0, -4));

    if (show_slider_) {
        int16_t text_min_height = position_.y() + top_.height() - 1;
        text_.draw(
            position_ + Point<int16_t>(162, 19 - offset_ * 400),
            Range<int16_t>(text_min_height, text_min_height + height_ - 18));
        slider_.draw(position_);
    } else {
        int16_t y_adj = height_ - min_height_;
        text_.draw(position_ + Point<int16_t>(166, 48 - y_adj));
    }
}

void UINpcTalk::update() {
    UIElement::update();

    if (draw_text_) {
        if (timestep_ > 4) {
            if (formatted_text_pos_ < formatted_text_.size()) {
                std::string t = text_.get_text();
                char c = formatted_text_[formatted_text_pos_];

                text_.change_text(t + c);

                formatted_text_pos_++;
                timestep_ = 0;
            } else {
                draw_text_ = false;
            }
        } else {
            timestep_++;
        }
    }
}

Button::State UINpcTalk::button_pressed(uint16_t buttonid) {
    deactivate();

    switch (type_) {
        case TalkType::SEND_NEXT:
        case TalkType::SEND_OK:
            // Type = 0
            switch (buttonid) {
                case Buttons::CLOSE: fn_npc_talk_more(type_, -1); break;
                case Buttons::NEXT:
                case Buttons::OK: fn_npc_talk_more(type_, 1); break;
            }
            break;
        case TalkType::SEND_NEXT_PREV:
            // Type = 0
            switch (buttonid) {
                case Buttons::CLOSE: fn_npc_talk_more(type_, -1); break;
                case Buttons::NEXT: fn_npc_talk_more(type_, 1); break;
                case Buttons::PREV: fn_npc_talk_more(type_, 0); break;
            }
            break;
        case TalkType::SEND_YESNO:
            // Type = 1
            switch (buttonid) {
                case Buttons::CLOSE: fn_npc_talk_more(type_, -1); break;
                case Buttons::NO: fn_npc_talk_more(type_, 0); break;
                case Buttons::YES: fn_npc_talk_more(type_, 1); break;
            }
            break;
        case TalkType::SEND_ACCEPT_DECLINE:
            // Type = 1
            switch (buttonid) {
                case Buttons::CLOSE: fn_npc_talk_more(type_, -1); break;
                case Buttons::QNO: fn_npc_talk_more(type_, 0); break;
                case Buttons::QYES: fn_npc_talk_more(type_, 1); break;
            }
            break;
        case TalkType::SEND_GET_TEXT:
            // TODO: What is this?
            break;
        case TalkType::SEND_GET_NUMBER:
            // Type = 3
            switch (buttonid) {
                case Buttons::CLOSE: fn_npc_talk_more(type_, 0); break;
                case Buttons::OK: fn_npc_talk_more(type_, 1); break;
            }
            break;
        case TalkType::SEND_SIMPLE:
            // Type = 4
            switch (buttonid) {
                case Buttons::CLOSE: fn_npc_talk_more(type_, 0); break;
                default:
                    fn_npc_talk_more(type_, 0);  // TODO: Selection
                    break;
            }
            break;
        default: break;
    }

    return Button::State::NORMAL;
}

Cursor::State UINpcTalk::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    Point<int16_t> cursor_relative = cursorpos - position_;

    if (show_slider_ && slider_.isenabled()) {
        if (Cursor::State sstate =
                slider_.send_cursor(cursor_relative, clicked)) {
            return sstate;
        }
    }

    Cursor::State estate = UIElement::send_cursor(clicked, cursorpos);

    if (estate == Cursor::State::CLICKING && clicked && draw_text_) {
        draw_text_ = false;
        text_.change_text(formatted_text_);
    }

    return estate;
}

void UINpcTalk::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed && escape) {
        deactivate();

        fn_npc_talk_more(type_, 0);
    }
}

UIElement::Type UINpcTalk::get_type() const {
    return TYPE;
}

UINpcTalk::TalkType UINpcTalk::get_by_value(int8_t value,
                                            uint8_t style_b0,
                                            uint8_t style_b1) {
    if (value > TalkType::NONE && value < TalkType::LENGTH) {
        if (value == 0) {
            bool is_send_ok = (style_b0 == 0) && (style_b1 == 0);
            bool is_next_prev = (style_b0 == 1) && (style_b1 == 1);
            bool is_next = (style_b0 == 0) && (style_b1 == 1);
            bool is_prev = (style_b0 == 1) && (style_b1 == 0);

            if (is_send_ok) {
                return TalkType::SEND_OK;
            }
            if (is_next_prev) {
                return TalkType::SEND_NEXT_PREV;
            }
            if (is_next) {
                return TalkType::SEND_NEXT;
            }
            if (is_prev) {
                return TalkType::SEND_PREV;
            }
        }
        return static_cast<TalkType>(value);
    }

    return TalkType::NONE;
}

// TODO: Move this to GraphicsGL?
std::string UINpcTalk::format_text(const std::string &tx,
                                   const int32_t &npcid) {
    std::string formatted_text = tx;
    size_t begin = formatted_text.find("#p");

    if (begin != std::string::npos) {
        size_t end = formatted_text.find("#", begin + 1);

        if (end != std::string::npos) {
            std::string namestr =
                nl::nx::string["Npc.img"][std::to_string(npcid)]["name"];
            formatted_text.replace(begin, end - begin, namestr);
        }
    }

    begin = formatted_text.find("#h");

    if (begin != std::string::npos) {
        size_t end = formatted_text.find("#", begin + 1);

        if (end != std::string::npos) {
            std::string charstr = Stage::get().get_player().get_name();
            formatted_text.replace(begin, end - begin, charstr);
        }
    }

    begin = formatted_text.find("#t");

    if (begin != std::string::npos) {
        size_t end = formatted_text.find("#", begin + 1);

        if (end != std::string::npos) {
            size_t b = begin + 2;
            int32_t itemid = std::stoi(formatted_text.substr(b, end - b));
            std::string itemname =
                nl::nx::string["Consume.img"][itemid]["name"];

            formatted_text.replace(begin, end - begin, itemname);
        }
    }

    // temp fix for taxi cab below
    begin = formatted_text.find("#m");

    if (begin != std::string::npos) {
        size_t end = formatted_text.find("#", begin + 1);

        if (end != std::string::npos) {
            size_t b = begin + std::string("#m").length();
            int32_t mapid = std::stoi(formatted_text.substr(b, end - b));
            auto map_info = NxHelper::Map::get_map_info_by_id(mapid);

            formatted_text.replace(begin-2, end - begin + 2, map_info.name);
        }
    }

    begin = formatted_text.find("#m");

    if (begin != std::string::npos) {
        size_t end = formatted_text.find("#", begin + 1);

        if (end != std::string::npos) {
            size_t b = begin + std::string("#m").length();
            int32_t mapid = std::stoi(formatted_text.substr(b, end - b));
            auto map_info = NxHelper::Map::get_map_info_by_id(mapid);

            formatted_text.replace(begin-2, end - begin + 2, map_info.name);
        }
    }

    begin = formatted_text.find("#m");

    if (begin != std::string::npos) {
        size_t end = formatted_text.find("#", begin + 1);

        if (end != std::string::npos) {
            size_t b = begin + std::string("#m").length();
            int32_t mapid = std::stoi(formatted_text.substr(b, end - b));
            auto map_info = NxHelper::Map::get_map_info_by_id(mapid);

            formatted_text.replace(begin-2, end - begin + 2, map_info.name);
        }
    }

    begin = formatted_text.find("#m");

    if (begin != std::string::npos) {
        size_t end = formatted_text.find("#", begin + 1);

        if (end != std::string::npos) {
            size_t b = begin + std::string("#m").length();
            int32_t mapid = std::stoi(formatted_text.substr(b, end - b));
            auto map_info = NxHelper::Map::get_map_info_by_id(mapid);

            formatted_text.replace(begin-2, end - begin + 2, map_info.name);
        }
    }

    begin = formatted_text.find("#m");

    if (begin != std::string::npos) {
        size_t end = formatted_text.find("#", begin + 1);

        if (end != std::string::npos) {
            size_t b = begin + std::string("#m").length();
            int32_t mapid = std::stoi(formatted_text.substr(b, end - b));
            auto map_info = NxHelper::Map::get_map_info_by_id(mapid);

            formatted_text.replace(begin-2, end - begin + 2, map_info.name);
        }
    }

    return formatted_text;
}

void UINpcTalk::change_text(int32_t npcid,
                            int8_t msgtype,
                            uint8_t style_b0,
                            uint8_t style_b1,
                            int8_t speakerbyte,
                            const std::string &tx) {
    type_ = get_by_value(msgtype, style_b0, style_b1);

    timestep_ = 0;
    draw_text_ = true;
    formatted_text_pos_ = 0;
    formatted_text_ = format_text(tx, npcid);

    text_ = Text(Text::Font::A12M,
                 Text::Alignment::LEFT,
                 Color::Name::DARKGREY,
                 formatted_text_,
                 320);

    int16_t text_height = text_.height();

    text_.change_text("");

    if (speakerbyte == 0) {
        std::string strid = std::to_string(npcid);
        strid.insert(0, 7 - strid.size(), '0');
        strid.append(".img");

        speaker_ = nl::nx::npc[strid]["stand"]["0"];

        std::string namestr =
            nl::nx::string["Npc.img"][std::to_string(npcid)]["name"];
        name_.change_text(namestr);
    } else {
        speaker_ = Texture();
        name_.change_text("");
    }

    height_ = min_height_;
    show_slider_ = false;

    if (text_height > height_) {
        if (text_height > MAX_HEIGHT_) {
            height_ = MAX_HEIGHT_;
            show_slider_ = true;
            row_max_ = text_height / 400 + 1;
            unit_rows_ = 1;

            int16_t slider_y = top_.height() - 7;
            slider_ = Slider(Slider::Type::DEFAULT_SILVER,
                             Range<int16_t>(slider_y, slider_y + height_ - 20),
                             top_.width() - 26,
                             unit_rows_,
                             row_max_,
                             onmoved_);
        } else {
            height_ = text_height;
        }
    }

    for (auto &button : buttons_) {
        button.second->set_active(false);
        button.second->set_state(Button::State::NORMAL);
    }

    int16_t y_cord = height_ + 48;

    buttons_[Buttons::CLOSE]->set_position(Point<int16_t>(9, y_cord));
    buttons_[Buttons::CLOSE]->set_active(true);

    switch (type_) {
        case TalkType::SEND_OK:
            buttons_[Buttons::OK]->set_position(Point<int16_t>(469, y_cord));
            buttons_[Buttons::OK]->set_active(true);
            break;
        case TalkType::SEND_YESNO: {
            Point<int16_t> yes_position = Point<int16_t>(389, y_cord);

            buttons_[Buttons::YES]->set_position(yes_position);
            buttons_[Buttons::NO]->set_position(yes_position
                                                + Point<int16_t>(65, 0));
            buttons_[Buttons::YES]->set_active(true);
            buttons_[Buttons::NO]->set_active(true);
            break;
        }
        case TalkType::SEND_NEXT:
            buttons_[Buttons::NEXT]->set_position(
                Point<int16_t>(461, y_cord - 30));
            buttons_[Buttons::NEXT]->set_active(true);
            break;
        case TalkType::SEND_PREV:
            buttons_[Buttons::PREV]->set_position(Point<int16_t>(367, y_cord));
            buttons_[Buttons::PREV]->set_active(true);
            break;
        case TalkType::SEND_NEXT_PREV:
            buttons_[Buttons::NEXT]->set_position(
                Point<int16_t>(461, y_cord - 30));
            buttons_[Buttons::PREV]->set_position(Point<int16_t>(367, y_cord));
            buttons_[Buttons::NEXT]->set_active(true);
            buttons_[Buttons::PREV]->set_active(true);
            break;
        case TalkType::SEND_ACCEPT_DECLINE:
            buttons_[Buttons::QYES]->set_position(Point<int16_t>(386, y_cord));
            buttons_[Buttons::QNO]->set_position(Point<int16_t>(451, y_cord));
            buttons_[Buttons::QYES]->set_active(true);
            buttons_[Buttons::QNO]->set_active(true);
            break;
        case TalkType::SEND_GET_TEXT:
        case TalkType::SEND_GET_NUMBER:
        case TalkType::SEND_SIMPLE:
        default: break;
    }

    position_ = Point<int16_t>(400 - top_.width() / 2, 240 - height_ / 2);
    dimension_ = Point<int16_t>(top_.width(), height_ + 120);
}
}  // namespace ms