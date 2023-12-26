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
#include "UISkillBook.h"

#include <nlnx/nx.hpp>

#include "../../Character/SkillId.h"
#include "../../Data/JobData.h"
#include "../../Data/SkillData.h"
#include "../../Gameplay/Stage.h"
#include "../../Net/Packets/PlayerPackets.h"
#include "../Components/MapleButton.h"
#include "../UI.h"

namespace ms {
namespace {
auto fn_spend_sp = []<typename... T>(T && ...args) {
    SpendSpPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UISkillBook::SkillIcon::SkillIcon(int32_t id) : skill_id_(id) {}

void UISkillBook::SkillIcon::drop_on_bindings(Point<int16_t> cursorposition,
                                              bool remove) const {
    auto keyconfig = UI::get().get_element<UIKeyConfig>();
    Keyboard::Mapping mapping = Keyboard::Mapping(KeyType::SKILL, skill_id_);

    if (remove) {
        keyconfig->get().unstage_mapping(mapping);
    } else {
        keyconfig->get().stage_mapping(cursorposition, mapping);
    }
}

Icon::IconType UISkillBook::SkillIcon::get_type() {
    return Icon::IconType::SKILL;
}

UISkillBook::SkillDisplayMeta::SkillDisplayMeta(int32_t i, int32_t l) :
    id_(i),
    level_(l) {
    const SkillData &data = SkillData::get(id_);

    Texture ntx = data.get_icon(SkillData::Icon::NORMAL);
    Texture dtx = data.get_icon(SkillData::Icon::DISABLED);
    Texture motx = data.get_icon(SkillData::Icon::MOUSEOVER);
    icon_ = std::make_unique<StatefulIcon>(std::make_unique<SkillIcon>(id_),
                                           ntx,
                                           dtx,
                                           motx);

    std::string namestr = data.get_name();
    std::string levelstr = std::to_string(level_);

    name_text_ = Text(Text::Font::A11M,
                      Text::Alignment::LEFT,
                      Color::Name::EMPEROR,
                      namestr);
    level_text_ = Text(Text::Font::A11M,
                       Text::Alignment::LEFT,
                       Color::Name::EMPEROR,
                       levelstr);

    constexpr uint16_t MAX_NAME_WIDTH = 97;
    size_t overhang = 3;

    while (name_text_.width() > MAX_NAME_WIDTH) {
        namestr.replace(namestr.end() - overhang, namestr.end(), "..");
        overhang += 1;

        name_text_.change_text(namestr);
    }
}

void UISkillBook::SkillDisplayMeta::draw(const DrawArgument &args) const {
    icon_->draw(args.get_pos());
    name_text_.draw(args + Point<int16_t>(38, -5));
    level_text_.draw(args + Point<int16_t>(38, 13));
}

int32_t UISkillBook::SkillDisplayMeta::get_id() const {
    return id_;
}

int32_t UISkillBook::SkillDisplayMeta::get_level() const {
    return level_;
}

StatefulIcon *UISkillBook::SkillDisplayMeta::get_icon() const {
    return icon_.get();
}

UISkillBook::UISkillBook(const CharStats &in_stats,
                         const SkillBook &in_skillbook) :
    UIDragElement<PosSKILL>(),
    stats_(in_stats),
    skillbook_(in_skillbook),
    tab_(0),
    grabbing_(false),
    macro_enabled_(false),
    sp_enabled_(false) {
    nl::node Skill = nl::nx::ui["UIWindow2.img"]["Skill"];
    nl::node main = Skill["main"];
    nl::node ui_backgrnd = main["backgrnd"];

    bg_dimensions_ = Texture(ui_backgrnd).get_dimensions();

    skilld_ = main["skill0"];
    skille_ = main["skill1"];
    skillb_ = main["skillBlank"];
    line_ = main["line"];

    buttons_[Buttons::BT_HYPER] =
        std::make_unique<MapleButton>(main["BtHyper"]);
    buttons_[Buttons::BT_GUILD_SKILL] =
        std::make_unique<MapleButton>(main["BtGuildSkill"]);
    buttons_[Buttons::BT_RIDE] = std::make_unique<MapleButton>(main["BtRide"]);
    buttons_[Buttons::BT_MACRO] =
        std::make_unique<MapleButton>(main["BtMacro"]);

    buttons_[Buttons::BT_HYPER]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_GUILD_SKILL]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_RIDE]->set_state(Button::State::DISABLED);

    nl::node skillPoint = nl::nx::ui["UIWindow4.img"]["Skill"]["skillPoint"];

    sp_backgrnd_ = skillPoint["backgrnd"];
    sp_backgrnd2_ = skillPoint["backgrnd2"];
    sp_backgrnd3_ = skillPoint["backgrnd3"];

    buttons_[Buttons::BT_CANCLE] =
        std::make_unique<MapleButton>(skillPoint["BtCancle"],
                                      Point<int16_t>(bg_dimensions_.x(), 0));
    buttons_[Buttons::BT_OKAY] =
        std::make_unique<MapleButton>(skillPoint["BtOkay"],
                                      Point<int16_t>(bg_dimensions_.x(), 0));
    buttons_[Buttons::BT_SPDOWN] =
        std::make_unique<MapleButton>(skillPoint["BtSpDown"],
                                      Point<int16_t>(bg_dimensions_.x(), 0));
    buttons_[Buttons::BT_SPMAX] =
        std::make_unique<MapleButton>(skillPoint["BtSpMax"],
                                      Point<int16_t>(bg_dimensions_.x(), 0));
    buttons_[Buttons::BT_SPUP] =
        std::make_unique<MapleButton>(skillPoint["BtSpUp"],
                                      Point<int16_t>(bg_dimensions_.x(), 0));

    buttons_[Buttons::BT_SPDOWN]->set_state(Button::State::DISABLED);

    sp_before_ = Charset(skillPoint["num"], Charset::Alignment::RIGHT);
    sp_after_ = Charset(skillPoint["num"], Charset::Alignment::RIGHT);
    sp_used_ =
        Text(Text::Font::A12B, Text::Alignment::RIGHT, Color::Name::WHITE);
    sp_remaining_ =
        Text(Text::Font::A12B, Text::Alignment::LEFT, Color::Name::SUPERNOVA);
    sp_name_ =
        Text(Text::Font::A12B, Text::Alignment::CENTER, Color::Name::WHITE);

    sprites_.emplace_back(ui_backgrnd, Point<int16_t>(1, 0));
    sprites_.emplace_back(main["backgrnd2"]);
    sprites_.emplace_back(main["backgrnd3"]);

    nl::node macro = Skill["macro"];

    macro_backgrnd_ = macro["backgrnd"];
    macro_backgrnd2_ = macro["backgrnd2"];
    macro_backgrnd3_ = macro["backgrnd3"];

    buttons_[Buttons::BT_MACRO_OK] =
        std::make_unique<MapleButton>(macro["BtOK"],
                                      Point<int16_t>(bg_dimensions_.x(), 0));

    buttons_[Buttons::BT_MACRO_OK]->set_state(Button::State::DISABLED);

    nl::node close = nl::nx::ui["Basic.img"]["BtClose3"];

    buttons_[Buttons::BT_CLOSE] = std::make_unique<MapleButton>(
        close,
        Point<int16_t>(bg_dimensions_.x() - 23, 6));

    nl::node Tab = main["Tab"];
    nl::node enabled = Tab["enabled"];
    nl::node disabled = Tab["disabled"];

    for (int i = Buttons::BT_TAB0; i <= Buttons::BT_TAB4; ++i) {
        auto tabid = i - Buttons::BT_TAB0;
        buttons_[i] =
            std::make_unique<TwoSpriteButton>(disabled[tabid], enabled[tabid]);
    }

    uint16_t y_adj = 0;

    for (int i = Buttons::BT_SPUP0; i <= Buttons::BT_SPUP11; ++i) {
        uint16_t x_adj = 0;
        auto spupid = i - Buttons::BT_SPUP0;

        if (spupid % 2) {
            x_adj = ROW_WIDTH_;
        }

        Point<int16_t> spup_position =
            SKILL_OFFSET_ + Point<int16_t>(124 + x_adj, 20 + y_adj);
        buttons_[i] =
            std::make_unique<MapleButton>(main["BtSpUp"], spup_position);

        if (spupid % 2) {
            y_adj += ROW_HEIGHT_;
        }
    }

    book_text_ = Text(Text::Font::A11M,
                      Text::Alignment::CENTER,
                      Color::Name::WHITE,
                      "",
                      150);
    sp_label_ =
        Text(Text::Font::A12M, Text::Alignment::RIGHT, Color::Name::BLACK);

    slider_ = Slider(Slider::Type::DEFAULT_SILVER,
                     Range<int16_t>(93, 317),
                     295,
                     ROWS_,
                     1,
                     [&](bool upwards) {
                         int16_t shift = upwards ? -1 : 1;
                         bool above = offset_ + shift >= 0;
                         bool below = offset_ + 4 + shift <= skill_count_;

                         if (above && below) {
                             change_offset(offset_ + shift);
                         }
                     });

    change_job(stats_.get_stat(MapleStat::Id::JOB));

    set_macro(false);
    set_skillpoint(false);

    dimension_ = bg_dimensions_;
    drag_area_ = Point<int16_t>(dimension_.x(), 20);
}

void UISkillBook::draw(float alpha) const {
    UIElement::draw_sprites(alpha);

    book_icon_.draw(position_ + Point<int16_t>(11, 85));
    book_text_.draw(position_ + Point<int16_t>(173, 59));
    sp_label_.draw(position_ + Point<int16_t>(304, 23));

    Point<int16_t> skill_position_l =
        position_ + SKILL_OFFSET_ + Point<int16_t>(-1, 0);
    Point<int16_t> skill_position_r =
        position_ + SKILL_OFFSET_ + Point<int16_t>(-1 + ROW_WIDTH_, 0);

    for (size_t i = 0; i < ROWS_; i++) {
        Point<int16_t> pos = skill_position_l;

        if (i % 2) {
            pos = skill_position_r;
        }

        if (i < skills_.size()) {
            if (check_required(skills_[i].get_id())) {
                skille_.draw(pos);
            } else {
                skilld_.draw(pos);
                skills_[i].get_icon()->set_state(StatefulIcon::State::DISABLED);
            }

            skills_[i].draw(pos + SKILL_META_OFFSET_);
        } else {
            skillb_.draw(pos);
        }

        if (i < ROWS_ - 2) {
            line_.draw(pos + LINE_OFFSET_);
        }

        if (i % 2) {
            skill_position_l.shift_y(ROW_HEIGHT_);
            skill_position_r.shift_y(ROW_HEIGHT_);
        }
    }

    slider_.draw(position_);

    if (macro_enabled_) {
        Point<int16_t> macro_pos =
            position_ + Point<int16_t>(bg_dimensions_.x(), 0);

        macro_backgrnd_.draw(macro_pos + Point<int16_t>(1, 0));
        macro_backgrnd2_.draw(macro_pos);
        macro_backgrnd3_.draw(macro_pos);
    }

    if (sp_enabled_) {
        Point<int16_t> sp_pos =
            position_ + Point<int16_t>(bg_dimensions_.x(), 0);

        sp_backgrnd_.draw(sp_pos);
        sp_backgrnd2_.draw(sp_pos);
        sp_backgrnd3_.draw(sp_pos);

        Point<int16_t> sp_level_pos = sp_pos + Point<int16_t>(78, 149);

        sp_before_.draw(sp_before_text_, 12, sp_level_pos);
        sp_after_.draw(sp_after_text_,
                       11,
                       sp_level_pos + Point<int16_t>(78, 0));
        sp_used_.draw(sp_pos + Point<int16_t>(82, 87));
        sp_remaining_.draw(sp_pos + Point<int16_t>(76, 65));
        sp_name_.draw(sp_pos + Point<int16_t>(97, 35));
        sp_skill_.draw(sp_pos + Point<int16_t>(13, 31));
    }

    UIElement::draw_buttons(alpha);
}

Button::State UISkillBook::button_pressed(uint16_t id) {
    int16_t cur_sp = std::stoi(sp_label_.get_text());

    switch (id) {
        case Buttons::BT_CLOSE: close(); break;
        case Buttons::BT_MACRO: set_macro(!macro_enabled_); break;
        case Buttons::BT_CANCLE: set_skillpoint(false); break;
        case Buttons::BT_OKAY: {
            int32_t used = std::stoi(sp_used_.get_text());

            while (used > 0) {
                spend_sp(sp_id_);
                used--;
            }

            change_sp();
            set_skillpoint(false);
        } break;
        case Buttons::BT_SPDOWN: {
            int32_t used = std::stoi(sp_used_.get_text());
            int32_t sp_after = std::stoi(sp_after_text_);
            int32_t sp_before = std::stoi(sp_before_text_);
            used--;
            sp_after--;

            sp_after_text_ = std::to_string(sp_after);
            sp_used_.change_text(std::to_string(used));
            sp_remaining_.change_text(std::to_string(cur_sp - used));

            buttons_[Buttons::BT_SPUP]->set_state(Button::State::NORMAL);
            buttons_[Buttons::BT_SPMAX]->set_state(Button::State::NORMAL);

            if (sp_after - 1 == sp_before) {
                return Button::State::DISABLED;
            }

            return Button::State::NORMAL;
        } break;
        case Buttons::BT_SPMAX: {
            int32_t used = std::stoi(sp_used_.get_text());
            int32_t sp_before = std::stoi(sp_before_text_);
            int32_t sp_touse = sp_master_level_ - sp_before - used;

            used += sp_touse;

            sp_after_text_ = std::to_string(sp_master_level_);
            sp_used_.change_text(std::to_string(used));
            sp_remaining_.change_text(std::to_string(cur_sp - used));

            buttons_[Buttons::BT_SPUP]->set_state(Button::State::DISABLED);
            buttons_[Buttons::BT_SPDOWN]->set_state(Button::State::NORMAL);

            return Button::State::DISABLED;
        } break;
        case Buttons::BT_SPUP: {
            int32_t used = std::stoi(sp_used_.get_text());
            int32_t sp_after = std::stoi(sp_after_text_);
            used++;
            sp_after++;

            sp_after_text_ = std::to_string(sp_after);
            sp_used_.change_text(std::to_string(used));
            sp_remaining_.change_text(std::to_string(cur_sp - used));

            buttons_[Buttons::BT_SPDOWN]->set_state(Button::State::NORMAL);

            if (sp_after == sp_master_level_) {
                buttons_[Buttons::BT_SPMAX]->set_state(Button::State::DISABLED);

                return Button::State::DISABLED;
            }

            return Button::State::NORMAL;
        } break;
        case Buttons::BT_TAB0:
        case Buttons::BT_TAB1:
        case Buttons::BT_TAB2:
        case Buttons::BT_TAB3:
        case Buttons::BT_TAB4:
            change_tab(id - Buttons::BT_TAB0);

            return Button::State::PRESSED;
        case Buttons::BT_SPUP0:
        case Buttons::BT_SPUP1:
        case Buttons::BT_SPUP2:
        case Buttons::BT_SPUP3:
        case Buttons::BT_SPUP4:
        case Buttons::BT_SPUP5:
        case Buttons::BT_SPUP6:
        case Buttons::BT_SPUP7:
        case Buttons::BT_SPUP8:
        case Buttons::BT_SPUP9:
        case Buttons::BT_SPUP10:
        case Buttons::BT_SPUP11:
            send_spup(id - Buttons::BT_SPUP0 + offset_);
            break;
        case Buttons::BT_HYPER:
        case Buttons::BT_GUILD_SKILL:
        case Buttons::BT_RIDE:
        case Buttons::BT_MACRO_OK:
        default: break;
    }

    return Button::State::NORMAL;
}

void UISkillBook::toggle_active() {
    if (!is_skillpoint_enabled()) {
        UIElement::toggle_active();

        clear_tooltip();
    }
}

void UISkillBook::doubleclick(Point<int16_t> cursorpos) {
    const SkillDisplayMeta *skill = skill_by_position(cursorpos - position_);

    if (skill) {
        int32_t skill_id = skill->get_id();
        int32_t skill_level = skillbook_.get_level(skill_id);

        if (skill_level > 0) {
            Stage::get().get_combat().use_move(skill_id);
        }
    }
}

void UISkillBook::remove_cursor() {
    UIDragElement::remove_cursor();

    slider_.remove_cursor();
}

Cursor::State UISkillBook::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    Cursor::State dstate = UIDragElement::send_cursor(clicked, cursorpos);

    if (dragged_) {
        return dstate;
    }

    Point<int16_t> cursor_relative = cursorpos - position_;

    if (slider_.isenabled()) {
        if (Cursor::State new_state =
                slider_.send_cursor(cursor_relative, clicked)) {
            clear_tooltip();

            return new_state;
        }
    }

    Point<int16_t> skill_position_l =
        position_ + SKILL_OFFSET_ + Point<int16_t>(-1, 0);
    Point<int16_t> skill_position_r =
        position_ + SKILL_OFFSET_ + Point<int16_t>(-1 + ROW_WIDTH_, 0);

    if (!grabbing_) {
        for (size_t i = 0; i < skills_.size(); i++) {
            Point<int16_t> skill_position = skill_position_l;

            if (i % 2) {
                skill_position = skill_position_r;
            }

            constexpr Rectangle<int16_t> bounds =
                Rectangle<int16_t>(0, 32, 0, 32);
            bool inrange = bounds.contains(cursorpos - skill_position);

            if (inrange) {
                if (clicked) {
                    clear_tooltip();
                    grabbing_ = true;

                    int32_t skill_id = skills_[i].get_id();
                    int32_t skill_level = skillbook_.get_level(skill_id);

                    if (skill_level > 0
                        && !SkillData::get(skill_id).is_passive()) {
                        skills_[i].get_icon()->start_drag(cursorpos
                                                          - skill_position);
                        UI::get().drag_icon(skills_[i].get_icon());

                        return Cursor::State::GRABBING;
                    }
                    return Cursor::State::IDLE;
                }
                skills_[i].get_icon()->set_state(
                    StatefulIcon::State::MOUSEOVER);
                show_skill(skills_[i].get_id());

                return Cursor::State::IDLE;
            }

            if (i % 2) {
                skill_position_l.shift_y(ROW_HEIGHT_);
                skill_position_r.shift_y(ROW_HEIGHT_);
            }
        }

        for (size_t i = 0; i < skills_.size(); i++) {
            skills_[i].get_icon()->set_state(StatefulIcon::State::NORMAL);
        }
        clear_tooltip();
    } else {
        grabbing_ = false;
    }

    return UIElement::send_cursor(clicked, cursorpos);
}

void UISkillBook::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            if (sp_enabled_) {
                set_skillpoint(false);
            } else {
                close();
            }
        } else if (keycode == KeyAction::Id::TAB) {
            clear_tooltip();

            Job::Level level = job_.get_level();
            uint16_t id = tab_ + 1;
            uint16_t new_tab = tab_ + Buttons::BT_TAB0;

            if (new_tab < Buttons::BT_TAB4 && id <= level) {
                new_tab++;
            } else {
                new_tab = Buttons::BT_TAB0;
            }

            change_tab(new_tab - Buttons::BT_TAB0);
        }
    }
}

UIElement::Type UISkillBook::get_type() const {
    return TYPE;
}

void UISkillBook::update_stat(MapleStat::Id stat, int16_t value) {
    switch (stat) {
        case MapleStat::Id::JOB: change_job(value); break;
        case MapleStat::Id::SP: change_sp(); break;
    }
}

void UISkillBook::update_skills(int32_t skill_id) {
    change_tab(tab_);
}

void UISkillBook::change_job(uint16_t id) {
    job_.change_job(id);

    Job::Level level = job_.get_level();

    for (int i = 0; i <= Job::Level::FOURTH; i++) {
        buttons_[Buttons::BT_TAB0 + i]->set_active(i <= level);
    }

    change_tab(level - Job::Level::BEGINNER);
}

void UISkillBook::change_sp() {
    Job::Level joblevel = joblevel_by_tab(tab_);
    uint16_t level = stats_.get_stat(MapleStat::Id::LEVEL);

    if (joblevel == Job::Level::BEGINNER) {
        int16_t remaining_beginner_sp = 0;

        if (level >= 7) {
            remaining_beginner_sp = 6;
        } else {
            remaining_beginner_sp = level - 1;
        }

        for (size_t i = 0; i < skills_.size(); i++) {
            int32_t skillid = skills_[i].get_id();

            if (skillid == SkillId::Id::THREE_SNAILS
                || skillid == SkillId::Id::HEAL
                || skillid == SkillId::Id::FEATHER) {
                remaining_beginner_sp -= skills_[i].get_level();
            }
        }

        beginner_sp_ = remaining_beginner_sp;
        sp_label_.change_text(std::to_string(beginner_sp_));
    } else {
        sp_ = stats_.get_stat(MapleStat::Id::SP);
        sp_label_.change_text(std::to_string(sp_));
    }

    change_offset(offset_);
    set_skillpoint(false);
}

void UISkillBook::change_tab(uint16_t new_tab) {
    buttons_[Buttons::BT_TAB0 + tab_]->set_state(Button::NORMAL);
    buttons_[Buttons::BT_TAB0 + new_tab]->set_state(Button::PRESSED);
    tab_ = new_tab;

    skills_.clear();
    skill_count_ = 0;

    Job::Level joblevel = joblevel_by_tab(tab_);
    uint16_t subid = job_.get_subjob(joblevel);

    const JobData &data = JobData::get(subid);

    book_icon_ = data.get_icon();
    book_text_.change_text(data.get_name());

    for (int skill_id : data.get_skills()) {
        int32_t level = skillbook_.get_level(skill_id);
        int32_t masterlevel = skillbook_.get_masterlevel(skill_id);

        bool invisible = SkillData::get(skill_id).is_invisible();

        if (invisible && masterlevel == 0) {
            continue;
        }

        skills_.emplace_back(skill_id, level);
        skill_count_++;
    }

    slider_.setrows(ROWS_, skill_count_);
    change_offset(0);
    change_sp();
}

void UISkillBook::change_offset(uint16_t new_offset) {
    offset_ = new_offset;

    for (int i = 0; i < ROWS_; i++) {
        auto index = Buttons::BT_SPUP0 + i;
        auto row = offset_ + i;
        buttons_[index]->set_active(row < skill_count_);

        if (row < skills_.size()) {
            int32_t skill_id = skills_[row].get_id();
            bool canraise = can_raise(skill_id);
            buttons_[index]->set_state(canraise ? Button::State::NORMAL
                                                : Button::State::DISABLED);
        }
    }
}

void UISkillBook::show_skill(int32_t id) {
    int32_t skill_id = id;
    int32_t level = skillbook_.get_level(id);
    int32_t masterlevel = skillbook_.get_masterlevel(id);
    int64_t expiration = skillbook_.get_expiration(id);

    UI::get().show_skill(Tooltip::Parent::SKILL_BOOK,
                         skill_id,
                         level,
                         masterlevel,
                         expiration);
}

void UISkillBook::clear_tooltip() {
    UI::get().clear_tooltip(Tooltip::Parent::SKILL_BOOK);
}

bool UISkillBook::can_raise(int32_t skill_id) const {
    Job::Level joblevel = joblevel_by_tab(tab_);

    if (joblevel == Job::Level::BEGINNER && beginner_sp_ <= 0) {
        return false;
    }

    if (tab_ + Buttons::BT_TAB0 != Buttons::BT_TAB0 && sp_ <= 0) {
        return false;
    }

    int32_t level = skillbook_.get_level(skill_id);
    int32_t masterlevel = skillbook_.get_masterlevel(skill_id);

    if (masterlevel == 0) {
        masterlevel = SkillData::get(skill_id).get_masterlevel();
    }

    if (level >= masterlevel) {
        return false;
    }

    switch (skill_id) {
        case SkillId::Id::ANGEL_BLESSING: return false;
        default: return check_required(skill_id);
    }
}

void UISkillBook::send_spup(uint16_t row) {
    if (row >= skills_.size()) {
        return;
    }

    int32_t id = skills_[row].get_id();

    if (sp_enabled_ && id == sp_id_) {
        set_skillpoint(false);
        return;
    }

    int32_t level = skills_[row].get_level();
    int32_t used = 1;

    const SkillData &skillData = SkillData::get(id);
    std::string name = skillData.get_name();
    int16_t cur_sp = std::stoi(sp_label_.get_text());

    sp_before_text_ = std::to_string(level);
    sp_after_text_ = std::to_string(level + used);
    sp_used_.change_text(std::to_string(used));
    sp_remaining_.change_text(std::to_string(cur_sp - used));
    sp_name_.change_text(name);
    sp_skill_ = skills_[row].get_icon()->get_texture();
    sp_id_ = id;
    sp_master_level_ = skillData.get_masterlevel();

    if (sp_master_level_ == 1) {
        buttons_[Buttons::BT_SPDOWN]->set_state(Button::State::DISABLED);
        buttons_[Buttons::BT_SPMAX]->set_state(Button::State::DISABLED);
        buttons_[Buttons::BT_SPUP]->set_state(Button::State::DISABLED);
    } else {
        buttons_[Buttons::BT_SPDOWN]->set_state(Button::State::DISABLED);
        buttons_[Buttons::BT_SPMAX]->set_state(Button::State::NORMAL);
        buttons_[Buttons::BT_SPUP]->set_state(Button::State::NORMAL);
    }

    if (!sp_enabled_) {
        set_skillpoint(true);
    }
}

void UISkillBook::spend_sp(int32_t skill_id) {
    fn_spend_sp(skill_id);

    UI::get().disable();
}

Job::Level UISkillBook::joblevel_by_tab(uint16_t t) const {
    switch (t) {
        case 1: return Job::Level::FIRST;
        case 2: return Job::Level::SECOND;
        case 3: return Job::Level::THIRD;
        case 4: return Job::Level::FOURTH;
        default: return Job::Level::BEGINNER;
    }
}

const UISkillBook::SkillDisplayMeta *UISkillBook::skill_by_position(
    Point<int16_t> cursorpos) const {
    int16_t x = cursorpos.x();

    if (x < SKILL_OFFSET_.x() || x > SKILL_OFFSET_.x() + 2 * ROW_WIDTH_) {
        return nullptr;
    }

    int16_t y = cursorpos.y();

    if (y < SKILL_OFFSET_.y()) {
        return nullptr;
    }

    uint16_t row = (y - SKILL_OFFSET_.y()) / ROW_HEIGHT_;

    if (row < 0 || row >= ROWS_) {
        return nullptr;
    }

    uint16_t offset_row = offset_ + row;

    if (offset_row >= ROWS_) {
        return nullptr;
    }

    uint16_t col = (x - SKILL_OFFSET_.x()) / ROW_WIDTH_;

    uint16_t skill_idx = 2 * offset_row + col;

    if (skill_idx >= skills_.size()) {
        return nullptr;
    }

    const auto *iter = skills_.data() + skill_idx;

    return iter;
}

void UISkillBook::close() {
    clear_tooltip();
    deactivate();
}

bool UISkillBook::check_required(int32_t id) const {
    std::unordered_map<int32_t, int32_t> required =
        skillbook_.collect_required(id);

    if (required.empty()) {
        required = SkillData::get(id).get_reqskills();
    }

    for (auto [req_skill_id, req_level] : required) {
        int32_t req_skill_level = skillbook_.get_level(req_skill_id);

        if (req_skill_level < req_level) {
            return false;
        }
    }

    return true;
}

void UISkillBook::set_macro(bool enabled) {
    macro_enabled_ = enabled;

    if (macro_enabled_) {
        dimension_ = bg_dimensions_
                     + Point<int16_t>(macro_backgrnd_.get_dimensions().x(), 0);
    } else if (!sp_enabled_) {
        dimension_ = bg_dimensions_;
    }

    buttons_[Buttons::BT_MACRO_OK]->set_active(macro_enabled_);

    if (macro_enabled_ && sp_enabled_) {
        set_skillpoint(false);
    }
}

void UISkillBook::set_skillpoint(bool enabled) {
    sp_enabled_ = enabled;

    if (sp_enabled_) {
        dimension_ = bg_dimensions_
                     + Point<int16_t>(sp_backgrnd_.get_dimensions().x(), 0);
    } else if (!macro_enabled_) {
        dimension_ = bg_dimensions_;
    }

    buttons_[Buttons::BT_CANCLE]->set_active(sp_enabled_);
    buttons_[Buttons::BT_OKAY]->set_active(sp_enabled_);
    buttons_[Buttons::BT_SPDOWN]->set_active(sp_enabled_);
    buttons_[Buttons::BT_SPMAX]->set_active(sp_enabled_);
    buttons_[Buttons::BT_SPUP]->set_active(sp_enabled_);

    if (sp_enabled_ && macro_enabled_) {
        set_macro(false);
    }
}

bool UISkillBook::is_skillpoint_enabled() {
    return sp_enabled_;
}
}  // namespace ms