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

#include "../../Character/SkillBook.h"
#include "../KeyConfig.h"
#include "../KeyType.h"
#include "../Keyboard.h"
#include "../UIDragElement.h"
#include "Inventory.h"

namespace ms {
class UIKeyConfig : public UIDragElement<PosKEYCONFIG> {
public:
    static constexpr Type TYPE = UIElement::Type::KEY_CONFIG;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = true;

    UIKeyConfig(const Inventory &inventory, const SkillBook &skillbook);

    void draw(float inter) const override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;
    bool send_icon(const Icon &icon, Point<int16_t> cursorpos) override;
    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

    void close();

    void stage_mapping(Point<int16_t> cursorposition,
                       Keyboard::Mapping mapping);
    void unstage_mapping(Keyboard::Mapping mapping);

    void update_item_count(InventoryType::Id type,
                           int16_t slot,
                           int16_t change);

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    void load_keys_pos();
    void load_unbound_actions_pos();
    void load_key_textures();
    void load_action_mappings();
    void load_action_icons();
    void load_item_icons();
    void load_skill_icons();

    void safe_close();

    void show_item(int32_t item_id);
    void show_skill(int32_t skill_id);
    void clear_tooltip();

    void save_staged_mappings();
    void bind_staged_action_keys();
    void clear();
    void reset();

    Texture get_item_texture(int32_t item_id) const;
    Texture get_skill_texture(int32_t skill_id) const;
    KeyConfig::Key key_by_position(Point<int16_t> position) const;
    KeyAction::Id unbound_action_by_position(Point<int16_t> position) const;
    Keyboard::Mapping get_staged_mapping(int32_t keycode) const;
    bool is_action_mapping(Keyboard::Mapping mapping) const;
    static KeyType::Id get_keytype(KeyAction::Id action);

    enum Buttons : uint16_t { CLOSE, CANCEL, DEFAULT, DELETE, KEY_SETTING, OK };

    class MappingIcon : public Icon::Type {
    public:
        MappingIcon(Keyboard::Mapping mapping);
        MappingIcon(KeyAction::Id keyId);

        void drop_on_stage() const override;
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
        Icon::IconType get_type() override;

    private:
        Keyboard::Mapping mapping_;
    };

    // Used for displaying item counts
    class CountableMappingIcon : public MappingIcon {
    public:
        CountableMappingIcon(Keyboard::Mapping mapping, int16_t count);

        void set_count(int16_t count) override;

    private:
        int16_t count_;
    };

    const Inventory &inventory_;
    const SkillBook &skillbook_;

    bool dirty_;

    Keyboard *keyboard_ = nullptr;

    nl::node key_;
    nl::node icon_;

    EnumMap<KeyConfig::Key, Texture> key_textures_;
    EnumMap<KeyConfig::Key, Point<int16_t>> keys_pos_;

    EnumMap<KeyAction::Id, std::unique_ptr<Icon>> action_icons_;
    EnumMap<KeyAction::Id, Point<int16_t>> unbound_actions_pos_;

    std::map<int32_t, std::unique_ptr<Icon>> item_icons_;
    std::map<int32_t, std::unique_ptr<Icon>> skill_icons_;

    // Used to determine if mapping belongs to predefined action, e.g. attack,
    // pick up, faces, etc.
    std::vector<Keyboard::Mapping> action_mappings_;

    std::vector<KeyAction::Id> bound_actions_;
    std::map<int32_t, Keyboard::Mapping> staged_mappings_;

    std::map<int32_t, Keyboard::Mapping> alternate_keys_ = {
        { KeyConfig::Key::ESCAPE,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::MAIN_MENU) },
        { KeyConfig::Key::F1,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE1) },
        { KeyConfig::Key::F2,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE2) },
        { KeyConfig::Key::F3,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE3) },
        { KeyConfig::Key::F5,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE4) },
        { KeyConfig::Key::F6,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE5) },
        { KeyConfig::Key::F7,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE6) },
        { KeyConfig::Key::F8,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE7) },
        { KeyConfig::Key::SCROLL_LOCK,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::SCREENSHOT) },
        { KeyConfig::Key::GRAVE_ACCENT,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::CASHSHOP) },
        { KeyConfig::Key::INSERT,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::SAY) },
        { KeyConfig::Key::HOME,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::PARTY_CHAT) },
        { KeyConfig::Key::PAGE_UP,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::FRIENDS_CHAT) },
        { KeyConfig::Key::T,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::BOSS_PARTY) },
        { KeyConfig::Key::Y,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::ITEM_POT) },
        { KeyConfig::Key::U,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::EQUIPMENT) },
        { KeyConfig::Key::I,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::ITEMS) },
        { KeyConfig::Key::P,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::PARTY) },
        { KeyConfig::Key::LEFT_BRACKET,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::MENU) },
        { KeyConfig::Key::RIGHT_BRACKET,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::QUICK_SLOTS) },
        { KeyConfig::Key::BACKSLASH,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::KEY_BINDINGS) },
        { KeyConfig::Key::DELETE,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::GUILD_CHAT) },
        { KeyConfig::Key::END,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::ALLIANCE_CHAT) },
        { KeyConfig::Key::G,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::GUILD) },
        { KeyConfig::Key::H,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::WHISPER) },
        { KeyConfig::Key::J,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::QUEST_LOG) },
        { KeyConfig::Key::K,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::SKILLS) },
        { KeyConfig::Key::L,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::NOTIFIER) },
        { KeyConfig::Key::SEMICOLON,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::MEDALS) },
        { KeyConfig::Key::APOSTROPHE,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::TOGGLE_CHAT) },
        { KeyConfig::Key::Z,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::PICKUP) },
        { KeyConfig::Key::X,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::SIT) },
        { KeyConfig::Key::C,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::STATS) },
        { KeyConfig::Key::V,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::EVENT) },
        { KeyConfig::Key::B,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::PROFESSION) },
        { KeyConfig::Key::N,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::WORLD_MAP) },
        { KeyConfig::Key::M,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::MINI_MAP) },
        { KeyConfig::Key::PERIOD,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::FRIENDS) },
        { KeyConfig::Key::LEFT_CONTROL,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::ATTACK) },
        { KeyConfig::Key::LEFT_ALT,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::JUMP) },
        { KeyConfig::Key::SPACE,
          Keyboard::Mapping(KeyType::Id::MENU,
                            KeyAction::Id::INTERACT_HARVEST) },
        { KeyConfig::Key::RIGHT_ALT,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::JUMP) },
        { KeyConfig::Key::RIGHT_CONTROL,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::ATTACK) }
    };

    std::map<int32_t, Keyboard::Mapping> basic_keys_ = {
        { KeyConfig::Key::ESCAPE,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::MAIN_MENU) },
        { KeyConfig::Key::F1,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE1) },
        { KeyConfig::Key::F2,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE2) },
        { KeyConfig::Key::F3,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE3) },
        { KeyConfig::Key::F5,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE4) },
        { KeyConfig::Key::F6,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE5) },
        { KeyConfig::Key::F7,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE6) },
        { KeyConfig::Key::F8,
          Keyboard::Mapping(KeyType::Id::FACE, KeyAction::Id::FACE7) },
        { KeyConfig::Key::SCROLL_LOCK,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::SCREENSHOT) },
        { KeyConfig::Key::GRAVE_ACCENT,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::CASHSHOP) },
        { KeyConfig::Key::NUM1,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::SAY) },
        { KeyConfig::Key::NUM2,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::PARTY_CHAT) },
        { KeyConfig::Key::NUM3,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::FRIENDS_CHAT) },
        { KeyConfig::Key::NUM4,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::GUILD_CHAT) },
        { KeyConfig::Key::NUM5,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::ALLIANCE_CHAT) },
        { KeyConfig::Key::Q,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::QUEST_LOG) },
        { KeyConfig::Key::W,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::WORLD_MAP) },
        { KeyConfig::Key::E,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::EQUIPMENT) },
        { KeyConfig::Key::R,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::FRIENDS) },
        { KeyConfig::Key::T,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::BOSS_PARTY) },
        { KeyConfig::Key::Y,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::ITEM_POT) },
        { KeyConfig::Key::U,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::GUIDE) },
        { KeyConfig::Key::I,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::ITEMS) },
        { KeyConfig::Key::O,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::ENHANCE_EQUIP) },
        { KeyConfig::Key::P,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::PARTY) },
        { KeyConfig::Key::LEFT_BRACKET,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::MENU) },
        { KeyConfig::Key::RIGHT_BRACKET,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::QUICK_SLOTS) },
        { KeyConfig::Key::BACKSLASH,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::KEY_BINDINGS) },
        { KeyConfig::Key::S,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::STATS) },
        { KeyConfig::Key::G,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::GUILD) },
        { KeyConfig::Key::H,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::WHISPER) },
        { KeyConfig::Key::K,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::SKILLS) },
        { KeyConfig::Key::L,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::NOTIFIER) },
        { KeyConfig::Key::SEMICOLON,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::MEDALS) },
        { KeyConfig::Key::APOSTROPHE,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::TOGGLE_CHAT) },
        { KeyConfig::Key::Z,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::PICKUP) },
        { KeyConfig::Key::X,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::SIT) },
        { KeyConfig::Key::C,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::MAPLE_CHAT) },
        { KeyConfig::Key::V,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::EVENT) },
        { KeyConfig::Key::B,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::PROFESSION) },
        { KeyConfig::Key::M,
          Keyboard::Mapping(KeyType::Id::MENU, KeyAction::Id::MINI_MAP) },
        { KeyConfig::Key::LEFT_CONTROL,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::ATTACK) },
        { KeyConfig::Key::LEFT_ALT,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::JUMP) },
        { KeyConfig::Key::SPACE,
          Keyboard::Mapping(KeyType::Id::MENU,
                            KeyAction::Id::INTERACT_HARVEST) },
        { KeyConfig::Key::RIGHT_ALT,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::JUMP) },
        { KeyConfig::Key::RIGHT_CONTROL,
          Keyboard::Mapping(KeyType::Id::ACTION, KeyAction::Id::ATTACK) }
    };
};
}  // namespace ms
