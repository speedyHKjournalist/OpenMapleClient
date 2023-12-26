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

#include <array>
#include <list>

#include "../../Character/Look/CharLook.h"
#include "../Components/Charset.h"
#include "../Components/NameTag.h"
#include "../UIElement.h"

namespace ms {
// The character selection screen
class UICharSelect : public UIElement {
public:
    static constexpr Type TYPE = UIElement::Type::CHAR_SELECT;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = false;

    UICharSelect(std::vector<CharEntry> characters,
                 int8_t characters_count,
                 int32_t slots,
                 int8_t require_pic);

    void draw(float inter) const override;

    void update() override;

    void doubleclick(Point<int16_t> cursorpos) override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

    void add_character(CharEntry &&character);

    void post_add_character();

    void remove_character(int32_t id);

    const CharEntry &get_character(int32_t id);

    bool update_character(int32_t id, const StatsEntry &stats);

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    void update_buttons();

    void update_selected_character();

    void select_last_slot();

    std::string get_slot_text();

    std::string pad_number_with_leading_zero(uint8_t value) const;

    Point<int16_t> get_character_slot_pos(size_t index,
                                          uint16_t x_adj,
                                          uint16_t y_adj) const;

    Point<int16_t> get_infolabel_pos(size_t index) const;

    std::string get_infolabel(size_t index, StatsEntry character_stats) const;

    void request_pic();

    void check_pic(const std::string &entered_pic) const;

    static constexpr uint8_t PAGESIZE_ = 8;

    enum Buttons : uint16_t {
        CHARACTER_SELECT,
        CHARACTER_NEW,
        CHARACTER_DELETE,
        PAGE_LEFT,
        PAGE_RIGHT,
        CHANGE_PIC,
        RESET_PIC,
        EDIT_CHAR_LIST,
        BACK,
        CHARACTER_SLOT0
    };

    std::vector<CharEntry> characters_;
    int8_t characters_count_;
    int32_t slots_;
    int8_t require_pic_;
    Text version_;
    Point<int16_t> page_pos_;
    Point<int16_t> world_pos_;
    Point<int16_t> charinfo_pos_;
    uint8_t selected_character_;
    uint8_t selected_page;
    uint8_t page_count_;
    Texture tab_;
    uint8_t tab_index_;
    bool tab_active_;
    bool tab_move_;
    std::array<Point<int16_t>, 3> tab_pos_;
    int16_t tab_move_pos_;
    std::map<uint8_t, uint16_t> tab_map_;
    Point<int16_t> world_dimensions_;
    Animation burning_notice_;
    Text burning_count_;
    std::vector<Sprite> world_sprites_;
    Texture charinfo_;
    Texture charslot_;
    Texture pagebase_;
    Charset pagenumber_;
    nl::node pagenumber_pos_;
    std::array<Texture, 3> sign_post_;
    nl::node nametag_;
    Charset level_set_;
    OutlinedText name_label_;
    std::vector<CharLook> char_looks_;
    std::vector<NameTag> nametags_;
    Animation empty_slot_effect_;
    Texture empty_slot_;
    std::array<Animation, 2> selected_slot_effect_;
    OutlinedText chat_slot_label_;
    int16_t timestamp_;
    uint16_t charslot_y_;
    bool show_timestamp_;
    bool burning_character_;

    enum InfoLabel : uint8_t { JOB, STR, DEX, INT, LUK, NUM_LABELS };

    std::array<OutlinedText, UICharSelect::InfoLabel::NUM_LABELS> info_labels_;
};
}  // namespace ms