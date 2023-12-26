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

#include <chrono>
#include <optional>

#include "../Character/CharStats.h"
#include "../Template/EnumMap.h"
#include "Components/EquipTooltip.h"
#include "Components/ItemTooltip.h"
#include "Components/MapTooltip.h"
#include "Components/SkillTooltip.h"
#include "Components/TextTooltip.h"
#include "UIState.h"

namespace ms {
class UIStateGame : public UIState {
public:
    UIStateGame(uint8_t channel_count);

    void draw(float inter, Point<int16_t> cursor) const override;

    void update() override;

    void doubleclick(Point<int16_t> pos) override;

    void rightclick(Point<int16_t> pos) override;

    void send_key(KeyType::Id type,
                  int32_t action,
                  bool pressed,
                  bool escape) override;

    Cursor::State send_cursor(Cursor::State cursorstate,
                              Point<int16_t> cursorpos) override;

    void send_scroll(double yoffset) override;

    void send_close() override;

    void drag_icon(Icon *icon) override;

    void clear_tooltip(Tooltip::Parent parent) override;

    void show_equip(Tooltip::Parent parent, int16_t slot) override;

    void show_item(Tooltip::Parent parent, int32_t itemid) override;

    void show_skill(Tooltip::Parent parent,
                    int32_t skill_id,
                    int32_t level,
                    int32_t masterlevel,
                    int64_t expiration) override;

    void show_text(Tooltip::Parent parent, std::string text) override;

    void show_map(Tooltip::Parent parent,
                  std::string name,
                  std::string description,
                  int32_t mapid,
                  bool bolded) override;

    Iterator pre_add(UIElement::Type type, bool toggled, bool focused) override;

    void remove(UIElement::Type type) override;

    UIElement *get(UIElement::Type type) override;

    UIElement *get_front(std::list<UIElement::Type> types) override;

    UIElement *get_front(Point<int16_t> pos) override;

private:
    const CharStats &stats_;

    bool drop_icon(const Icon &icon, Point<int16_t> pos);

    void remove_icon();

    void remove_cursors();

    void remove_cursor(UIElement::Type type);

    template<class T, typename... Args>
    void emplace(Args &&... args);

    EnumMap<UIElement::Type, UIElement::UPtr, UIElement::Type::NUM_TYPES>
        elements_;
    std::list<UIElement::Type> element_order_;
    UIElement::Type focused_;
    UIElement *dragged_;

    EquipTooltip eq_tooltip_;
    ItemTooltip it_tooltip_;
    SkillTooltip sk_tooltip_;
    TextTooltip te_tooltip_;
    MapTooltip ma_tooltip_;
    std::optional<std::reference_wrapper<Tooltip>> tooltip_;
    Tooltip::Parent tooltip_parent_;

    std::optional<std::reference_wrapper<Icon>> dragged_icon_;
    std::chrono::time_point<std::chrono::steady_clock> time_rel_grabbed =
        std::chrono::steady_clock::now();
    static constexpr std::chrono::microseconds MIN_DELAY_NEXT_GRAB_ { 10 };

    std::map<Icon::IconType, UIElement::Type> icon_map_ = {
        { Icon::IconType::NONE, UIElement::Type::NONE },
        { Icon::IconType::SKILL, UIElement::Type::SKILL_BOOK },
        { Icon::IconType::EQUIP, UIElement::Type::EQUIP_INVENTORY },
        { Icon::IconType::ITEM, UIElement::Type::ITEM_INVENTORY },
        { Icon::IconType::KEY, UIElement::Type::KEY_CONFIG },
        { Icon::IconType::NUM_TYPES, UIElement::Type::NUM_TYPES }
    };

    int16_t VWIDTH;
    int16_t VHEIGHT;
    uint8_t channel_count_;
};
}  // namespace ms