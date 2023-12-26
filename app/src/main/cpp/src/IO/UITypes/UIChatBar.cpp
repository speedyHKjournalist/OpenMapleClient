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
#include "UIChatBar.h"

#include <nlnx/nx.hpp>

#include "../../Net/Packets/MessagingPackets.h"
#include "../Components/MapleButton.h"

namespace ms {
namespace {
auto fn_general_chat = []<typename... T>(T && ...args) {
    GeneralChatPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIChatBar::UIChatBar() : UIDragElement<PosCHAT>(Point<int16_t>(410, -5)) {
    is_chat_open_ = Setting<Chatopen>::get().load();
    should_chat_open_persist_ = is_chat_open_;
    is_chatfield_open_ = false;
    chat_rows_ = 5;
    last_pos_ = 0;
    row_pos_ = 0;
    row_max_ = -1;

    nl::node chat = nl::nx::ui["StatusBar3.img"]["chat"];
    nl::node ingame = chat["ingame"];
    nl::node view = ingame["view"];
    nl::node input = ingame["input"];
    nl::node chatTarget = chat["common"]["chatTarget"];

    chatspace_[0] = view["min"]["top"];
    chatspace_[1] = view["min"]["center"];
    chatspace_[2] = view["min"]["bottom"];
    chatspace_[3] = view["drag"];

    int16_t chattop_y = get_chat_top(true) - 33;
    closechat_ = Point<int16_t>(387, 21);

    buttons_[Buttons::BT_OPEN_CHAT] =
        std::make_unique<MapleButton>(view["btMax"], Point<int16_t>(391, -7));
    buttons_[Buttons::BT_CLOSE_CHAT] = std::make_unique<MapleButton>(
        view["btMin"],
        closechat_ + Point<int16_t>(0, chattop_y));
    buttons_[Buttons::BT_CHAT] =
        std::make_unique<MapleButton>(input["button:chat"],
                                      Point<int16_t>(344, -8));
    buttons_[Buttons::BT_LINK] =
        std::make_unique<MapleButton>(input["button:itemLink"],
                                      Point<int16_t>(365, -8));
    buttons_[Buttons::BT_HELP] =
        std::make_unique<MapleButton>(input["button:help"],
                                      Point<int16_t>(386, -8));

    buttons_[is_chat_open_ ? Buttons::BT_OPEN_CHAT : Buttons::BT_CLOSE_CHAT]
        ->set_active(false);
    buttons_[Buttons::BT_CHAT]->set_active(is_chat_open_);
    buttons_[Buttons::BT_LINK]->set_active(is_chat_open_);
    buttons_[Buttons::BT_HELP]->set_active(is_chat_open_);

    chattab_x_ = 6;
    chattab_y_ = chattop_y;
    chattab_span_ = 54;

    for (size_t i = 0; i < ChatTab::NUM_CHATTAB; i++) {
        buttons_[Buttons::BT_TAB_0 + i] = std::make_unique<MapleButton>(
            view["tab"],
            Point<int16_t>(chattab_x_ + (i * chattab_span_), chattab_y_));
        buttons_[Buttons::BT_TAB_0 + i]->set_active(is_chat_open_);
        chattab_text_[ChatTab::CHT_ALL + i] = Text(Text::Font::A12M,
                                                   Text::Alignment::CENTER,
                                                   Color::Name::DUSTYGRAY,
                                                   chat_tab_text_[i]);
    }

    chattab_text_[ChatTab::CHT_ALL].change_color(Color::Name::WHITE);

    buttons_[Buttons::BT_TAB_0 + ChatTab::NUM_CHATTAB] =
        std::make_unique<MapleButton>(
            view["btAddTab"],
            Point<int16_t>(chattab_x_ + (ChatTab::NUM_CHATTAB * chattab_span_),
                           chattab_y_));
    buttons_[Buttons::BT_TAB_0 + ChatTab::NUM_CHATTAB]->set_active(
        is_chat_open_);

    buttons_[Buttons::BT_CHAT_TARGET] =
        std::make_unique<MapleButton>(chatTarget["all"], Point<int16_t>(5, -8));
    buttons_[Buttons::BT_CHAT_TARGET]->set_active(is_chat_open_);

    chatenter_ = input["layer:chatEnter"];
    chatcover_ = input["layer:backgrnd"];

    chatfield_ = Textfield(
        Text::A11M,
        Text::LEFT,
        Color::Name::WHITE,
        Rectangle<int16_t>(Point<int16_t>(62, -9), Point<int16_t>(330, 8)),
        0);
    chatfield_.set_state(is_chat_open_ ? Textfield::State::NORMAL
                                       : Textfield::State::DISABLED);

    chatfield_.set_enter_callback([&](std::string msg) {
        if (!msg.empty()) {
            size_t last = msg.find_last_not_of(' ');

            if (last != std::string::npos) {
                msg.erase(last + 1);

                fn_general_chat(msg, true);

                last_entered_.push_back(msg);
                last_pos_ = last_entered_.size();
            } else {
                toggle_chatfield();
            }

            chatfield_.change_text("");
        } else {
            toggle_chatfield();
        }
    });

    chatfield_.set_key_callback(KeyAction::Id::UP, [&]() {
        if (last_pos_ > 0) {
            last_pos_--;
            chatfield_.change_text(last_entered_[last_pos_]);
        }
    });

    chatfield_.set_key_callback(KeyAction::Id::DOWN, [&]() {
        if (!last_entered_.empty() && last_pos_ < last_entered_.size() - 1) {
            last_pos_++;
            chatfield_.change_text(last_entered_[last_pos_]);
        }
    });

    chatfield_.set_key_callback(KeyAction::Id::ESCAPE,
                                [&]() { toggle_chatfield(false); });

    // int16_t slider_x = 394;
    // int16_t slider_y = -80;
    // int16_t slider_height = slider_y + 56;
    // int16_t slider_unitrows = chat_rows_;
    // int16_t slider_rowmax = 1;
    // slider_ = Slider(Slider::Type::CHAT_BAR, Range<int16_t>(slider_y,
    // slider_height), slider_x, slider_unitrows, slider_rowmax, [&](bool
    // upwards) {});

    send_chatline("[Welcome] Welcome to MapleStory!!", LineType::YELLOW);

    dimension_ = Point<int16_t>(410, DIMENSION_Y_);

    // if (is_chat_open_)
    //	dimension.shift_y(get_chatbar_height());
}

void UIChatBar::draw(float inter) const {
    UIElement::draw_sprites(inter);

    if (is_chat_open_) {
        int16_t chattop = get_chat_top(is_chat_open_);

        auto pos_adj =
            is_chatfield_open_ ? Point<int16_t>(0, 0) : Point<int16_t>(0, 28);

        chatspace_[0].draw(position_ + Point<int16_t>(0, chattop) + pos_adj);

        if (chat_rows_ > 1) {
            chatspace_[1].draw(
                DrawArgument(position_ + Point<int16_t>(0, -28) + pos_adj,
                             Point<int16_t>(0, 28 + chattop)));
        }

        chatspace_[2].draw(position_ + Point<int16_t>(0, -28) + pos_adj);
        chatspace_[3].draw(position_ + Point<int16_t>(0, -15 + chattop)
                           + pos_adj);

        // slider_.draw(position);

        int16_t yshift = chattop;

        for (size_t i = 0; i < chat_rows_; i++) {
            int16_t rowid = row_pos_ - i;

            if (!row_texts_.count(rowid)) {
                break;
            }

            int16_t textheight =
                row_texts_.at(rowid).height() / CHAT_ROW_HEIGHT_;

            while (textheight > 0) {
                yshift += CHAT_ROW_HEIGHT_;
                textheight--;
            }

            row_texts_.at(rowid).draw(
                position_
                + Point<int16_t>(9, get_chat_top(is_chat_open_) - yshift - 21)
                + pos_adj);
        }
    } else {
        auto pos_adj =
            is_chatfield_open_ ? Point<int16_t>(0, -28) : Point<int16_t>(0, 0);

        chatspace_[0].draw(position_ + Point<int16_t>(0, -1) + pos_adj);
        chatspace_[1].draw(position_ + Point<int16_t>(0, -1) + pos_adj);
        chatspace_[2].draw(position_ + pos_adj);
        chatspace_[3].draw(position_ + Point<int16_t>(0, -16) + pos_adj);

        if (row_texts_.count(row_max_)) {
            row_texts_.at(row_max_).draw(position_ + Point<int16_t>(9, -6)
                                         + pos_adj);
        }
    }

    if (is_chatfield_open_) {
        chatcover_.draw(DrawArgument(position_ + Point<int16_t>(0, -13),
                                     Point<int16_t>(409, 0)));
        chatenter_.draw(DrawArgument(position_ + Point<int16_t>(0, -13),
                                     Point<int16_t>(285, 0)));
        chatfield_.draw(position_ + Point<int16_t>(-4, -4));
    }

    UIElement::draw_buttons(inter);

    if (is_chat_open_) {
        auto pos_adj = is_chat_open_ && !is_chatfield_open_
                           ? Point<int16_t>(0, 28)
                           : Point<int16_t>(0, 0);

        for (size_t i = 0; i < ChatTab::NUM_CHATTAB; i++) {
            chattab_text_[ChatTab::CHT_ALL + i].draw(
                position_
                + Point<int16_t>(chattab_x_ + (i * chattab_span_) + 25,
                                 chattab_y_ - 3)
                + pos_adj);
        }
    }
}

void UIChatBar::update() {
    UIElement::update();

    auto pos_adj = is_chat_open_ && !is_chatfield_open_ ? Point<int16_t>(0, 28)
                                                        : Point<int16_t>(0, 0);

    for (size_t i = 0; i < ChatTab::NUM_CHATTAB; i++) {
        buttons_[BT_TAB_0 + i]->set_position(
            Point<int16_t>(chattab_x_ + (i * chattab_span_), chattab_y_)
            + pos_adj);
    }

    buttons_[Buttons::BT_TAB_0 + ChatTab::NUM_CHATTAB]->set_position(
        Point<int16_t>(chattab_x_ + (ChatTab::NUM_CHATTAB * chattab_span_),
                       chattab_y_)
        + pos_adj);
    buttons_[Buttons::BT_CLOSE_CHAT]->set_position(
        closechat_ + Point<int16_t>(0, chattab_y_) + pos_adj);

    chatfield_.update(position_);

    for (auto iter : message_cooldowns_) {
        iter.second -= Constants::TIMESTEP;
    }
}

void UIChatBar::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (keycode == KeyAction::Id::RETURN) {
            toggle_chatfield();
        } else if (escape) {
            toggle_chatfield(false);
        }
    }
}

bool UIChatBar::is_in_range(Point<int16_t> cursorpos) const {
    auto bounds = get_bounds(dimension_);
    return bounds.contains(cursorpos);
}

Cursor::State UIChatBar::send_cursor(bool clicking, Point<int16_t> cursorpos) {
    if (is_chat_open_) {
        if (Cursor::State new_state =
                chatfield_.send_cursor(cursorpos, clicking)) {
            return new_state;
        }

        return check_drag_top(clicking, cursorpos);
    }
    return UIDragElement::send_cursor(clicking, cursorpos);
}

UIElement::Type UIChatBar::get_type() const {
    return TYPE;
}

Cursor::State UIChatBar::check_drag_top(bool clicking,
                                        Point<int16_t> cursorpos) {
    Rectangle<int16_t> bounds = get_bounds(dimension_);
    Point<int16_t> bounds_lt = bounds.get_left_top();
    Point<int16_t> bounds_rb = bounds.get_right_bottom();

    int16_t chattab_height = 20;
    int16_t bounds_rb_y = bounds_rb.y();
    int16_t bounds_lt_y = bounds_lt.y() + chattab_height;

    auto chattop_rb = Point<int16_t>(bounds_rb.x() - 1, bounds_rb_y - 27);
    auto chattop =
        Rectangle<int16_t>(Point<int16_t>(bounds_lt.x() + 1, bounds_lt_y),
                           chattop_rb);

    auto chattopleft =
        Rectangle<int16_t>(Point<int16_t>(bounds_lt.x(), bounds_lt_y),
                           Point<int16_t>(bounds_lt.x(), chattop_rb.y()));
    auto chattopright =
        Rectangle<int16_t>(Point<int16_t>(chattop_rb.x() + 1, bounds_lt_y),
                           Point<int16_t>(chattop_rb.x() + 1, chattop_rb.y()));
    auto chatleft = Rectangle<int16_t>(
        Point<int16_t>(bounds_lt.x(), bounds_lt_y),
        Point<int16_t>(bounds_lt.x(), bounds_lt_y + bounds_rb_y));
    auto chatright = Rectangle<int16_t>(
        Point<int16_t>(chattop_rb.x() + 1, bounds_lt_y),
        Point<int16_t>(chattop_rb.x() + 1, bounds_lt_y + bounds_rb_y));

    bool in_chattop = chattop.contains(cursorpos);
    bool in_chattopleft = chattopleft.contains(cursorpos);
    bool in_chattopright = chattopright.contains(cursorpos);
    bool in_chatleft = chatleft.contains(cursorpos);
    bool in_chatright = chatright.contains(cursorpos);

    if (should_drag_chat_top_) {
        if (clicking) {
            int16_t ydelta = cursorpos.y() - bounds_rb_y + 10;

            while (ydelta > 0 && chat_rows_ > MIN_CHAT_ROWS_) {
                chat_rows_--;
                ydelta -= CHAT_ROW_HEIGHT_;
            }

            while (ydelta < 0 && chat_rows_ < MAX_CHAT_ROWS_) {
                chat_rows_++;
                ydelta += CHAT_ROW_HEIGHT_;
            }

            // slider_.setrows(row_pos_, chat_rows_, row_max_);
            // slider_.setvertical(Range<int16_t>(0, CHAT_ROW_HEIGHT_ *
            // chat_rows_ - 14));

            chattab_y_ = get_chat_top(is_chat_open_) - 33;
            // dimension.set_y(get_chatbar_height());

            return Cursor::State::CLICKING;
        }
        should_drag_chat_top_ = false;

    } else if (in_chattop) {
        if (clicking) {
            should_drag_chat_top_ = true;

            return Cursor::State::CLICKING;
        }
        return Cursor::State::CHAT_BAR_VDRAG;

    } else if (in_chattopleft) {
        if (clicking) {
            // dragchattopleft = true;

            return Cursor::State::CLICKING;
        }
        return Cursor::State::CHAT_BAR_BRTLDRAG;

    } else if (in_chattopright) {
        if (clicking) {
            // dragchattopright = true;

            return Cursor::State::CLICKING;
        }
        return Cursor::State::CHAT_BAR_BLTRDRAG;

    } else if (in_chatleft) {
        if (clicking) {
            // dragchatleft = true;

            return Cursor::State::CLICKING;
        }
        return Cursor::State::CHAT_BAR_HDRAG;

    } else if (in_chatright) {
        if (clicking) {
            // dragchatright = true;

            return Cursor::State::CLICKING;
        }
        return Cursor::State::CHAT_BAR_HDRAG;
    }

    return UIDragElement::send_cursor(clicking, cursorpos);
}

bool UIChatBar::is_in_drag_range(Point<int16_t> cursorpos) const {
    auto bounds = get_bounds(drag_area_);

    return bounds.contains(cursorpos);
}

void UIChatBar::send_chatline(const std::string &line, LineType type) {
    row_max_++;
    row_pos_ = row_max_;

    // slider_.setrows(row_pos_, chat_rows_, row_max_);

    Color::Name color = Color::Name::WHITE;

    switch (type) {
        case LineType::RED: color = Color::Name::DARKRED; break;
        case LineType::BLUE: color = Color::Name::MEDIUMBLUE; break;
        case LineType::YELLOW: color = Color::Name::YELLOW; break;
        case LineType::GREEN: color = Color::Name::GREEN; break;
        default: color = Color::Name::WHITE; break;
    }

    row_texts_.emplace(std::piecewise_construct,
                       std::forward_as_tuple(row_max_),
                       std::forward_as_tuple(Text::Font::A11M,
                                             Text::Alignment::LEFT,
                                             color,
                                             line,
                                             480));
}

void UIChatBar::display_message(Messages::Type line, UIChatBar::LineType type) {
    if (message_cooldowns_[line] > 0) {
        return;
    }

    std::string message = Messages::messages[line];
    send_chatline(message, type);

    message_cooldowns_[line] = MESSAGE_COOLDOWN_;
}

void UIChatBar::toggle_chat() {
    should_chat_open_persist_ = !should_chat_open_persist_;
    toggle_chat(should_chat_open_persist_);
}

void UIChatBar::toggle_chat(bool chat_open) {
    if (!chat_open && should_chat_open_persist_) {
        return;
    }

    is_chat_open_ = chat_open;

    if (!is_chat_open_ && is_chatfield_open_) {
        toggle_chatfield();
    }

    buttons_[Buttons::BT_OPEN_CHAT]->set_active(!chat_open);
    buttons_[Buttons::BT_CLOSE_CHAT]->set_active(chat_open);

    for (size_t i = 0; i < ChatTab::NUM_CHATTAB; i++) {
        buttons_[Buttons::BT_TAB_0 + i]->set_active(chat_open);
    }

    buttons_[Buttons::BT_TAB_0 + ChatTab::NUM_CHATTAB]->set_active(chat_open);
}

void UIChatBar::toggle_chatfield() {
    is_chatfield_open_ = !is_chatfield_open_;
    toggle_chatfield(is_chatfield_open_);
}

void UIChatBar::toggle_chatfield(bool chatfield_open) {
    chatfield_open = chatfield_open;

    toggle_chat(chatfield_open);

    if (chatfield_open) {
        buttons_[Buttons::BT_CHAT]->set_active(true);
        buttons_[Buttons::BT_HELP]->set_active(true);
        buttons_[Buttons::BT_LINK]->set_active(true);
        buttons_[Buttons::BT_CHAT_TARGET]->set_active(true);

        chatfield_.set_state(Textfield::State::FOCUSED);

        // dimension.shift_y(get_chatbar_height());
    } else {
        buttons_[Buttons::BT_CHAT]->set_active(false);
        buttons_[Buttons::BT_HELP]->set_active(false);
        buttons_[Buttons::BT_LINK]->set_active(false);
        buttons_[Buttons::BT_CHAT_TARGET]->set_active(false);

        chatfield_.set_state(Textfield::State::DISABLED);
        chatfield_.change_text("");

        // dimension.set_y(DIMENSION_Y_);
    }
}

bool UIChatBar::is_chat_open() {
    return is_chat_open_;
}

bool UIChatBar::is_chatfield_open() {
    return is_chatfield_open_;
}

Button::State UIChatBar::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::BT_OPEN_CHAT:
        case Buttons::BT_CLOSE_CHAT: toggle_chat(); break;
        case Buttons::BT_TAB_0:
        case Buttons::BT_TAB_1:
        case Buttons::BT_TAB_2:
        case Buttons::BT_TAB_3:
        case Buttons::BT_TAB_4:
        case Buttons::BT_TAB_5:
            for (size_t i = 0; i < ChatTab::NUM_CHATTAB; i++) {
                buttons_[Buttons::BT_TAB_0 + i]->set_state(
                    Button::State::NORMAL);
                chattab_text_[ChatTab::CHT_ALL + i].change_color(
                    Color::Name::DUSTYGRAY);
            }

            chattab_text_[buttonid - Buttons::BT_TAB_0].change_color(
                Color::Name::WHITE);

            return Button::State::PRESSED;
    }

    Setting<Chatopen>::get().save(is_chat_open_);

    return Button::State::NORMAL;
}

int16_t UIChatBar::get_chat_top(bool chat_open) const {
    if (chat_open) {
        return get_chatbar_height() * -1;
    }
    return -1;
}

int16_t UIChatBar::get_chatbar_height() const {
    return 15 + chat_rows_ * CHAT_ROW_HEIGHT_;
}

Rectangle<int16_t> UIChatBar::get_bounds(Point<int16_t> additional_area) const {
    int16_t screen_adj = (is_chat_open_) ? 35 : 16;

    auto absp = position_ + Point<int16_t>(0, get_chat_top(is_chat_open_));
    auto da = absp + additional_area;

    absp = Point<int16_t>(absp.x(), absp.y() - screen_adj);
    da = Point<int16_t>(da.x(), da.y());

    return Rectangle<int16_t>(absp, da);
}
}  // namespace ms