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
#include "UIQuit.h"

#include <nlnx/nx.hpp>

#include "../../Character/ExpTable.h"
#include "../../Gameplay/Stage.h"
#include "../../Net/Session.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "../Window.h"
#include "PacketProcessor.h"
#include "../Timer.h"

namespace ms {
UIQuit::UIQuit(const CharStats &st) : stats_(st), screen_adj_(212, 114) {
    nl::node askReward = nl::nx::ui["UIWindow6.img"]["askReward"];
    nl::node userLog = askReward["userLog"];
    nl::node exp = userLog["exp"];
    nl::node level = userLog["level"];
    nl::node time = userLog["time"];
    nl::node backgrnd = userLog["backgrnd"];

    sprites_.emplace_back(backgrnd, -screen_adj_);

    buttons_[Buttons::NO] =
        std::make_unique<MapleButton>(askReward["btNo"], Point<int16_t>(0, 37));
    buttons_[Buttons::YES] =
        std::make_unique<MapleButton>(askReward["btYes"],
                                      Point<int16_t>(0, 37));

    Stage &stage = Stage::get();

    /// Time
    int64_t uptime = stage.get_uptime() / 1000 / 1000;
    minutes_ = uptime / 60;
    hours_ = minutes_ / 60;

    minutes_ -= hours_ * 60;

    time_minutes_ = Charset(time["number"], Charset::Alignment::LEFT);
    time_minutes_pos_ = time["posM"];
    time_minutes_text_ = pad_time(minutes_);

    time_hours_ = Charset(time["number"], Charset::Alignment::LEFT);
    time_hours_pos_ = time["posH"];
    time_hours_text_ = pad_time(hours_);

    time_number_width_ = time["numberWidth"];

    time_lt_ = time["tooltip"]["lt"];
    time_rb_ = time["tooltip"]["rb"];

    /// Level
    levelup_effect_ = level["levelupEffect"];

    uplevel_ = stage.get_uplevel();

    level_before_ = Charset(level["number"], Charset::Alignment::LEFT);
    level_before_pos_ = level["posBefore"];
    level_before_text_ = std::to_string(uplevel_);

    cur_level_ = stats_.get_stat(MapleStat::Id::LEVEL);

    level_after_ = Charset(level["number"], Charset::Alignment::LEFT);
    level_after_pos_ = level["posAfter"];
    level_after_text_ = std::to_string(cur_level_);

    level_number_width_ = level["numberWidth"];

    level_adj_ = Point<int16_t>(40, 0);

    /// Experience
    int64_t upexp = stage.get_upexp();
    float expPercentBefore = getexppercent(uplevel_, upexp);
    std::string expBeforeString = std::to_string(100 * expPercentBefore);
    std::string expBeforeText =
        expBeforeString.substr(0, expBeforeString.find('.') + 3) + '%';

    exp_before_ = Text(Text::Font::A11M,
                       Text::Alignment::LEFT,
                       Color::Name::WHITE,
                       expBeforeText);
    exp_before_pos_ = exp["posBefore"];

    int64_t cur_exp = stats_.get_exp();
    float expPercentAfter = getexppercent(cur_level_, cur_exp);
    std::string expAfterString = std::to_string(100 * expPercentAfter);
    std::string expAfterText =
        expAfterString.substr(0, expAfterString.find('.') + 3) + '%';

    exp_after_ = Text(Text::Font::A11M,
                      Text::Alignment::LEFT,
                      Color::Name::ELECTRICLIME,
                      expAfterText);
    exp_after_pos_ = exp["posAfter"];

    exp_adj_ = Point<int16_t>(0, 6);

    int16_t width = Constants::Constants::get().get_viewwidth();
    int16_t height = Constants::Constants::get().get_viewheight();

    background_ = ColorBox(width, height, Color::Name::BLACK, 0.5f);
    position_ = Point<int16_t>(width / 2, height / 2);
    dimension_ = Texture(backgrnd).get_dimensions();
}

void UIQuit::draw(float inter) const {
    background_.draw(Point<int16_t>(0, 0));

    UIElement::draw(inter);

    time_minutes_.draw(time_minutes_text_,
                       time_number_width_,
                       position_ + time_minutes_pos_ - screen_adj_);
    time_hours_.draw(time_hours_text_,
                     time_number_width_,
                     position_ + time_hours_pos_ - screen_adj_);

    level_before_.draw(
        level_before_text_,
        level_number_width_,
        position_ + level_before_pos_ + level_adj_ - screen_adj_);
    level_after_.draw(level_after_text_,
                      level_number_width_,
                      position_ + level_after_pos_ + level_adj_ - screen_adj_);

    if (cur_level_ > uplevel_) {
        levelup_effect_.draw(position_ - screen_adj_, inter);
    }

    exp_before_.draw(position_ + exp_before_pos_ - exp_adj_ - screen_adj_);
    exp_after_.draw(position_ + exp_after_pos_ - exp_adj_ - screen_adj_);
}

void UIQuit::update() {
    UIElement::update();

    levelup_effect_.update();
}

Cursor::State UIQuit::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    auto lt = position_ + time_lt_ - screen_adj_;
    auto rb = position_ + time_rb_ - screen_adj_;

    auto bounds = Rectangle<int16_t>(lt, rb);

    if (bounds.contains(cursorpos)) {
        UI::get().show_text(Tooltip::Parent::TEXT,
                            std::to_string(hours_) + "Hour "
                                + std::to_string(minutes_) + "Minute");
    } else {
        UI::get().clear_tooltip(Tooltip::Parent::TEXT);
    }

    return UIElement::send_cursor(clicked, cursorpos);
}

void UIQuit::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            close();
        } else if (keycode == KeyAction::Id::RETURN) {
            button_pressed(Buttons::YES);
        }
    }
}

UIElement::Type UIQuit::get_type() const {
    return TYPE;
}

Button::State UIQuit::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::NO: deactivate(); break;
        case Buttons::YES: {
            Constants::Constants::get().set_viewwidth(800);
            Constants::Constants::get().set_viewheight(600);

            float fadestep = 0.025f;

            Window::get().fadeout(fadestep, []() {
                GraphicsGL::get().clear();

                UI::get().change_state(UI::State::LOGIN);
                UI::get().set_scrollnotice("");
                PacketProcessor::get().reconnect();

                UI::get().enable();
                Timer::get().start();
                GraphicsGL::get().unlock();
            });

            GraphicsGL::get().lock();
            Stage::get().clear();
            Timer::get().start();
        } break;
        default: break;
    }

    return Button::State::NORMAL;
}

std::string UIQuit::pad_time(int64_t time) {
    std::string ctime = std::to_string(time);
    size_t length = ctime.length();

    if (length > 2) {
        return "99";
    }

    return std::string(2 - length, '0') + ctime;
}

float UIQuit::getexppercent(uint16_t level, int64_t exp) const {
    if (level >= ExpTable::LEVELCAP) {
        return 0.0f;
    }

    return static_cast<float>(static_cast<double>(exp)
                              / ExpTable::values[level]);
}

void UIQuit::close() {
    deactivate();

    UI::get().clear_tooltip(Tooltip::Parent::TEXT);
}
}  // namespace ms