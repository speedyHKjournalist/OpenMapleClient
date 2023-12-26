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
#pragma once

#include "../../Character/CharStats.h"
#include "../../Character/SkillBook.h"
#include "../Components/Charset.h"
#include "../Components/Slider.h"
#include "../Components/StatefulIcon.h"
#include "../UIDragElement.h"

namespace ms {
class UISkillBook : public UIDragElement<PosSKILL> {
public:
    static constexpr Type TYPE = UIElement::Type::SKILL_BOOK;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = true;

    UISkillBook(const CharStats &stats, const SkillBook &skillbook);

    void draw(float alpha) const override;

    void toggle_active() override;

    void doubleclick(Point<int16_t> cursorpos) override;

    void remove_cursor() override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

    void update_stat(MapleStat::Id stat, int16_t value);

    void update_skills(int32_t skill_id);

    bool is_skillpoint_enabled();

protected:
    Button::State button_pressed(uint16_t id) override;

private:
    class SkillIcon : public StatefulIcon::Type {
    public:
        SkillIcon(int32_t skill_id);

        void drop_on_stage() const override {}
        void drop_on_equips(EquipSlot::Id) const override {}
        bool drop_on_items(InventoryType::Id,
                           EquipSlot::Id,
                           int16_t,
                           bool) const override {
            return true;
        }
        void drop_on_bindings(Point<int16_t> cursorposition,
                              bool remove) const override;

        void set_count(int16_t) override {}

        void set_state(StatefulIcon::State) override {}

        Icon::IconType get_type() override;

    private:
        int32_t skill_id_;
    };

    class SkillDisplayMeta {
    public:
        SkillDisplayMeta(int32_t id, int32_t level);

        void draw(const DrawArgument &args) const;

        int32_t get_id() const;

        int32_t get_level() const;

        StatefulIcon *get_icon() const;

    private:
        int32_t id_;
        int32_t level_;
        std::unique_ptr<StatefulIcon> icon_;
        Text name_text_;
        Text level_text_;
    };

    void change_job(uint16_t id);

    void change_sp();

    void change_tab(uint16_t new_tab);

    void change_offset(uint16_t new_offset);

    void show_skill(int32_t skill_id);

    void clear_tooltip();

    bool can_raise(int32_t skill_id) const;

    void send_spup(uint16_t row);

    void spend_sp(int32_t skill_id);

    Job::Level joblevel_by_tab(uint16_t tab) const;

    const UISkillBook::SkillDisplayMeta *skill_by_position(
        Point<int16_t> cursorpos) const;

    void close();

    bool check_required(int32_t id) const;

    void set_macro(bool enabled);

    void set_skillpoint(bool enabled);

    enum Buttons : uint16_t {
        BT_CLOSE,
        BT_HYPER,
        BT_GUILD_SKILL,
        BT_RIDE,
        BT_MACRO,
        BT_MACRO_OK,
        BT_CANCLE,
        BT_OKAY,
        BT_SPDOWN,
        BT_SPMAX,
        BT_SPUP,
        BT_TAB0,
        BT_TAB1,
        BT_TAB2,
        BT_TAB3,
        BT_TAB4,
        BT_SPUP0,
        BT_SPUP1,
        BT_SPUP2,
        BT_SPUP3,
        BT_SPUP4,
        BT_SPUP5,
        BT_SPUP6,
        BT_SPUP7,
        BT_SPUP8,
        BT_SPUP9,
        BT_SPUP10,
        BT_SPUP11
    };

    static constexpr int16_t ROWS_ = 12;
    static constexpr int16_t ROW_HEIGHT_ = 40;
    static constexpr int16_t ROW_WIDTH_ = 143;
    static constexpr Point<int16_t> SKILL_OFFSET_ = Point<int16_t>(11, 93);
    static constexpr Point<int16_t> SKILL_META_OFFSET_ = Point<int16_t>(2, 2);
    static constexpr Point<int16_t> LINE_OFFSET_ = Point<int16_t>(0, 37);

    const CharStats &stats_;
    const SkillBook &skillbook_;

    Slider slider_;
    Texture skille_;
    Texture skilld_;
    Texture skillb_;
    Texture line_;
    Texture book_icon_;
    Text book_text_;
    Text sp_label_;

    Job job_;
    int16_t sp_;
    int16_t beginner_sp_;

    uint16_t tab_;
    uint16_t skill_count_;
    uint16_t offset_;

    std::vector<SkillDisplayMeta> skills_;
    bool grabbing_;

    Point<int16_t> bg_dimensions_;

    bool macro_enabled_;
    Texture macro_backgrnd_;
    Texture macro_backgrnd2_;
    Texture macro_backgrnd3_;

    bool sp_enabled_;
    Texture sp_backgrnd_;
    Texture sp_backgrnd2_;
    Texture sp_backgrnd3_;
    Charset sp_before_;
    Charset sp_after_;
    std::string sp_before_text_;
    std::string sp_after_text_;
    Text sp_used_;
    Text sp_remaining_;
    Text sp_name_;
    Texture sp_skill_;
    int32_t sp_id_;
    int32_t sp_master_level_;
};
}  // namespace ms