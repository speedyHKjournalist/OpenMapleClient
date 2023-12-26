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
#include "UICharInfo.h"

#include <nlnx/nx.hpp>

#include "../../Gameplay/Stage.h"
#include "../../Net/Packets/PlayerInteractionPackets.h"
#include "../Components/MapleButton.h"

namespace ms {
namespace {
auto fn_char_info_request = []<typename... T>(T && ...args) {
    CharInfoRequestPacket(std::forward<T>(args)...).dispatch();
};
auto fn_trade_request = [](auto cid) {
    PlayerInteractionPacket(PlayerInteractionPacket::mode::CREATE,
                            PlayerInteractionPacket::CreateType::TRADE)
        .dispatch();
    PlayerInteractionPacket(PlayerInteractionPacket::mode::INVITE, cid)
        .dispatch();
};
auto fn_give_fame = []<typename... T>(T && ...args) {
    GiveFamePacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UICharInfo::UICharInfo(int32_t cid) :
    UIDragElement<PosCHARINFO>(),
    cid_(cid),
    is_loading_(true),
    timestep_(Constants::TIMESTEP),
    personality_enabled_(false),
    collect_enabled_(false),
    damage_enabled_(false),
    item_enabled_(false) {
    nl::node close = nl::nx::ui["Basic.img"]["BtClose3"];
    nl::node UserInfo = nl::nx::ui["UIWindow2.img"]["UserInfo"];
    nl::node character = UserInfo["character"];
    nl::node backgrnd = character["backgrnd"];

    /// Main Window
    sprites_.emplace_back(backgrnd);
    sprites_.emplace_back(character["backgrnd2"]);
    sprites_.emplace_back(character["name"]);

    Point<int16_t> backgrnd_dim = Texture(backgrnd).get_dimensions();
    Point<int16_t> close_dimensions = Point<int16_t>(backgrnd_dim.x() - 21, 6);

    buttons_[Buttons::BT_CLOSE] =
        std::make_unique<MapleButton>(close, close_dimensions);
    buttons_[Buttons::BT_COLLECT] =
        std::make_unique<MapleButton>(character["BtCollect"]);
    buttons_[Buttons::BT_DAMAGE] =
        std::make_unique<MapleButton>(character["BtDamage"]);
    buttons_[Buttons::BT_FAMILY] =
        std::make_unique<MapleButton>(character["BtFamily"]);
    buttons_[Buttons::BT_ITEM] =
        std::make_unique<MapleButton>(character["BtItem"]);
    buttons_[Buttons::BT_PARTY] =
        std::make_unique<MapleButton>(character["BtParty"]);
    buttons_[Buttons::BT_PERSONALITY] =
        std::make_unique<MapleButton>(character["BtPersonality"]);
    buttons_[Buttons::BT_PET] =
        std::make_unique<MapleButton>(character["BtPet"]);
    buttons_[Buttons::BT_POP_DOWN] =
        std::make_unique<MapleButton>(character["BtPopDown"]);
    buttons_[Buttons::BT_POP_UP] =
        std::make_unique<MapleButton>(character["BtPopUp"]);
    buttons_[Buttons::BT_RIDE] =
        std::make_unique<MapleButton>(character["BtRide"]);
    buttons_[Buttons::BT_TRADE] =
        std::make_unique<MapleButton>(character["BtTrad"]);

    name_ = Text(Text::Font::A12M, Text::Alignment::CENTER, Color::Name::WHITE);
    job_ = Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);
    level_ =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);
    fame_ = Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);
    guild_ =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);
    alliance_ =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);

    // TODO: Check if player has a mount or pet, disable if they don't
    buttons_[Buttons::BT_PET]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_RIDE]->set_state(Button::State::DISABLED);

    /// Farm
    nl::node farm = UserInfo["farm"];
    nl::node farm_backgrnd = farm["backgrnd"];

    loading_ = farm["loading"];

    farm_dim_ = Texture(farm_backgrnd).get_dimensions();
    farm_adj_ = Point<int16_t>(-farm_dim_.x(), 0);

    sprites_.emplace_back(farm_backgrnd, farm_adj_);
    sprites_.emplace_back(farm["backgrnd2"], farm_adj_);
    sprites_.emplace_back(farm["default"], farm_adj_);
    sprites_.emplace_back(farm["cover"], farm_adj_);

    buttons_[Buttons::BT_FRIEND] =
        std::make_unique<MapleButton>(farm["btFriend"], farm_adj_);
    buttons_[Buttons::BT_VISIT] =
        std::make_unique<MapleButton>(farm["btVisit"], farm_adj_);

    farm_name_ =
        Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::SUPERNOVA);
    farm_level_ = Charset(farm["number"], Charset::Alignment::LEFT);

#pragma region BottomWindow
    bottom_window_adj_ = Point<int16_t>(0, backgrnd_dim.y() + 1);

    /// Personality
    nl::node personality = UserInfo["personality"];
    nl::node personality_backgrnd = personality["backgrnd"];

    personality_sprites_.emplace_back(personality_backgrnd, bottom_window_adj_);
    personality_sprites_.emplace_back(personality["backgrnd2"],
                                      bottom_window_adj_);

    personality_sprites_enabled_[true].emplace_back(personality["backgrnd3"],
                                                    bottom_window_adj_);
    personality_sprites_enabled_[true].emplace_back(personality["backgrnd4"],
                                                    bottom_window_adj_);
    personality_sprites_enabled_[true].emplace_back(personality["center"],
                                                    bottom_window_adj_);
    personality_sprites_enabled_[false].emplace_back(
        personality["before30level"],
        bottom_window_adj_);

    personality_dimensions_ = Texture(personality_backgrnd).get_dimensions();

    /// Collect
    nl::node collect = UserInfo["collect"];
    nl::node collect_backgrnd = collect["backgrnd"];

    collect_sprites_.emplace_back(collect_backgrnd, bottom_window_adj_);
    collect_sprites_.emplace_back(collect["backgrnd2"], bottom_window_adj_);

    default_medal_ = collect["icon1"];

    buttons_[Buttons::BT_ARRAY_GET] =
        std::make_unique<MapleButton>(collect["BtArrayGet"],
                                      bottom_window_adj_);
    buttons_[Buttons::BT_ARRAY_NAME] =
        std::make_unique<MapleButton>(collect["BtArrayName"],
                                      bottom_window_adj_);

    medal_text_ = Text(Text::Font::A11M,
                       Text::Alignment::LEFT,
                       Color::Name::EMPEROR,
                       "Junior Adventurer");
    medal_total_ = Text(Text::Font::A11M,
                        Text::Alignment::LEFT,
                        Color::Name::EMPEROR,
                        "2");

    collect_dimensions_ = Texture(collect_backgrnd).get_dimensions();

    /// Damage
    nl::node damage = UserInfo["damage"];
    nl::node damage_backgrnd = damage["backgrnd"];

    damage_sprites_.emplace_back(damage_backgrnd, bottom_window_adj_);
    damage_sprites_.emplace_back(damage["backgrnd2"], bottom_window_adj_);
    damage_sprites_.emplace_back(damage["backgrnd3"], bottom_window_adj_);

    buttons_[Buttons::BT_FAQ] =
        std::make_unique<MapleButton>(damage["BtFAQ"], bottom_window_adj_);
    buttons_[Buttons::BT_REGIST] =
        std::make_unique<MapleButton>(damage["BtRegist"], bottom_window_adj_);

    damage_dimensions_ = Texture(damage_backgrnd).get_dimensions();
#pragma endregion

#pragma region RightWindow
    right_window_adj_ = Point<int16_t>(backgrnd_dim.x(), 0);

    /// Item
    nl::node item = UserInfo["item"];
    nl::node item_backgrnd = item["backgrnd"];

    item_sprites_.emplace_back(item_backgrnd, right_window_adj_);
    item_sprites_.emplace_back(item["backgrnd2"], right_window_adj_);

    item_dimensions_ = Texture(item_backgrnd).get_dimensions();
#pragma endregion

    dimension_ = backgrnd_dim;
    drag_area_ = Point<int16_t>(dimension_.x(), 20);

    target_character_ = &Stage::get().get_character(cid)->get();

    fn_char_info_request(cid);
}

void UICharInfo::draw(float inter) const {
    UIElement::draw_sprites(inter);

    for (size_t i = 0; i < Buttons::BT_ARRAY_GET; i++) {
        if (auto *const button = buttons_.at(i).get()) {
            button->draw(position_);
        }
    }

    /// Main Window
    int16_t row_height = 18;
    Point<int16_t> text_pos = position_ + Point<int16_t>(153, 65);

    target_character_->draw_preview(position_ + Point<int16_t>(63, 129), inter);

    name_.draw(position_ + Point<int16_t>(59, 131));
    level_.draw(text_pos + Point<int16_t>(0, row_height * 0));
    job_.draw(text_pos + Point<int16_t>(0, row_height * 1));
    fame_.draw(text_pos + Point<int16_t>(0, row_height * 2));
    guild_.draw(text_pos + Point<int16_t>(0, row_height * 3)
                + Point<int16_t>(0, 1));
    alliance_.draw(text_pos + Point<int16_t>(0, row_height * 4));

    /// Farm
    Point<int16_t> farm_pos = position_ + farm_adj_;

    if (is_loading_) {
        loading_.draw(farm_pos, inter);
    }

    farm_name_.draw(farm_pos + Point<int16_t>(136, 51));
    farm_level_.draw(farm_level_text_, farm_pos + Point<int16_t>(126, 34));

    /// Personality
    if (personality_enabled_) {
        for (const Sprite &sprite : personality_sprites_) {
            sprite.draw(position_, inter);
        }

        bool show_personality = (target_character_->get_level() >= 30);

        for (const Sprite &sprite :
             personality_sprites_enabled_[show_personality]) {
            sprite.draw(position_, inter);
        }
    }

    /// Collect
    if (collect_enabled_) {
        for (const Sprite &sprite : collect_sprites_) {
            sprite.draw(position_, inter);
        }

        for (size_t i = 0; i < 15; i++) {
            div_t div = std::div(i, 5);
            default_medal_.draw(
                position_ + bottom_window_adj_ + Point<int16_t>(61, 66)
                    + Point<int16_t>(38 * div.rem, 38 * div.quot),
                inter);
        }

        for (size_t i = Buttons::BT_ARRAY_GET; i < Buttons::BT_FAQ; i++) {
            if (auto *const button = buttons_.at(i).get()) {
                button->draw(position_);
            }
        }

        Point<int16_t> text_pos = Point<int16_t>(121, 8);

        medal_text_.draw(position_ + bottom_window_adj_ + text_pos);
        medal_total_.draw(position_ + bottom_window_adj_ + text_pos
                          + Point<int16_t>(0, 19));
    }

    /// Damage
    if (damage_enabled_) {
        for (const Sprite &sprite : damage_sprites_) {
            sprite.draw(position_, inter);
        }

        for (size_t i = Buttons::BT_FAQ; i < buttons_.size(); i++) {
            if (auto *const button = buttons_.at(i).get()) {
                button->draw(position_);
            }
        }
    }

    /// Item
    if (item_enabled_) {
        for (const Sprite &sprite : item_sprites_) {
            sprite.draw(position_, inter);
        }
    }
}

void UICharInfo::update() {
    if (timestep_ >= Constants::TIMESTEP * UCHAR_MAX) {
        is_loading_ = false;
    } else {
        loading_.update();
        timestep_ += Constants::TIMESTEP;
    }
}

Button::State UICharInfo::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::BT_CLOSE: deactivate(); return Button::State::NORMAL;
        case Buttons::BT_FAMILY:
        case Buttons::BT_PARTY: break;
        case Buttons::BT_ITEM:
            show_right_window(buttonid);
            return Button::State::NORMAL;
        case Buttons::BT_COLLECT:
        case Buttons::BT_PERSONALITY:
        case Buttons::BT_RIDE:
        case Buttons::BT_PET:
        case Buttons::BT_DAMAGE:
            show_bottom_window(buttonid);
            return Button::State::NORMAL;
        case Buttons::BT_POP_DOWN:
            fn_give_fame(target_character_->get_oid(),
                         GiveFamePacket::mode::DECREASE);
            break;
        case Buttons::BT_POP_UP:
            fn_give_fame(target_character_->get_oid(),
                         GiveFamePacket::mode::INCREASE);
            break;
        case Buttons::BT_TRADE: fn_trade_request(cid_); break;
        case Buttons::BT_FRIEND:
        case Buttons::BT_VISIT:
        default: break;
    }

    return Button::State::DISABLED;
}

bool UICharInfo::is_in_range(Point<int16_t> cursorpos) const {
    Rectangle<int16_t> bounds =
        Rectangle<int16_t>(position_, position_ + dimension_);

    Rectangle<int16_t> farm_bounds =
        Rectangle<int16_t>(position_, position_ + farm_dim_);
    farm_bounds.shift(farm_adj_);

    Rectangle<int16_t> bottom_bounds =
        Rectangle<int16_t>(Point<int16_t>(0, 0), Point<int16_t>(0, 0));
    Rectangle<int16_t> right_bounds =
        Rectangle<int16_t>(Point<int16_t>(0, 0), Point<int16_t>(0, 0));

    int16_t cur_x = cursorpos.x();
    int16_t cur_y = cursorpos.y();

    if (personality_enabled_) {
        bottom_bounds =
            Rectangle<int16_t>(position_, position_ + personality_dimensions_);
        bottom_bounds.shift(bottom_window_adj_);
    }

    if (collect_enabled_) {
        bottom_bounds =
            Rectangle<int16_t>(position_, position_ + collect_dimensions_);
        bottom_bounds.shift(bottom_window_adj_);
    }

    if (damage_enabled_) {
        bottom_bounds =
            Rectangle<int16_t>(position_, position_ + damage_dimensions_);
        bottom_bounds.shift(bottom_window_adj_);
    }

    if (item_enabled_) {
        right_bounds =
            Rectangle<int16_t>(position_, position_ + item_dimensions_);
        right_bounds.shift(right_window_adj_);
    }

    return bounds.contains(cursorpos) || farm_bounds.contains(cursorpos)
           || bottom_bounds.contains(cursorpos)
           || right_bounds.contains(cursorpos);
}

void UICharInfo::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed && escape) {
        deactivate();
    }
}

UIElement::Type UICharInfo::get_type() const {
    return TYPE;
}

void UICharInfo::update_stats(int32_t character_id,
                              int16_t job_id,
                              uint8_t lv,
                              int16_t f,
                              const std::string &g,
                              const std::string &a) {
    cid_ = character_id;
    int32_t player_id = Stage::get().get_player().get_oid();

    auto bt_state = character_id == player_id ? Button::State::DISABLED
                                              : Button::State::NORMAL;

    buttons_[Buttons::BT_PARTY]->set_state(bt_state);
    buttons_[Buttons::BT_TRADE]->set_state(bt_state);
    buttons_[Buttons::BT_POP_DOWN]->set_state(bt_state);
    buttons_[Buttons::BT_POP_UP]->set_state(bt_state);
    buttons_[Buttons::BT_FRIEND]->set_state(bt_state);

    Job character_job = Job(job_id);

    target_character_ = &Stage::get().get_character(character_id)->get();
    name_.change_text(target_character_->get_name());
    job_.change_text(character_job.get_name());
    level_.change_text(std::to_string(lv));
    fame_.change_text(std::to_string(f));
    guild_.change_text((g.empty() ? "-" : g));
    alliance_.change_text(a);

    farm_name_.change_text("");
    farm_level_text_ = "1";
}

void UICharInfo::show_bottom_window(uint16_t buttonid) {
    bool was_personality_enabled = personality_enabled_;
    bool was_collect_enabled = collect_enabled_;
    bool was_damage_enabled = damage_enabled_;
    personality_enabled_ = false;
    collect_enabled_ = false;
    damage_enabled_ = false;

    switch (buttonid) {
        case Buttons::BT_PERSONALITY:
            personality_enabled_ = !was_personality_enabled;
            break;
        case Buttons::BT_COLLECT:
            collect_enabled_ = !was_collect_enabled;
            break;
        case Buttons::BT_DAMAGE: damage_enabled_ = !was_damage_enabled; break;
    }
}

void UICharInfo::show_right_window(uint16_t buttonid) {
    bool was_item_enabled = item_enabled_;
    item_enabled_ = false;

    switch (buttonid) {
        case Buttons::BT_ITEM: item_enabled_ = !was_item_enabled; break;
    }
}
}  // namespace ms