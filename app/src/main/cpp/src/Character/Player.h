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

#include "../Gameplay/Combat/Skill.h"
#include "../Gameplay/MapleMap/Layer.h"
#include "../Gameplay/MapleMap/MapInfo.h"
#include "../Gameplay/Physics/Physics.h"
#include "../Gameplay/Playable.h"
#include "ActiveBuffs.h"
#include "Inventory/Inventory.h"
#include "MonsterBook.h"
#include "PassiveBuffs.h"
#include "QuestLog.h"
#include "SkillBook.h"
#include "TeleportRock.h"

namespace ms {
class Player : public Playable, public Char {
public:
    // Construct a player object from the given character entry.
    Player(const CharEntry &entry, uint8_t wid, uint8_t channel_id);

    Player();

    // Draw the player
    using Char::draw;
    void draw(Layer::Id layer, double viewx, double viewy, float alpha) const;

    // Update the player's animation, physics and states.
    int8_t update(const Physics &physics) override;

    // Set flipped ignore if attacking.
    void set_direction(bool flipped) override;

    // Set state ignore if attacking.
    void set_state(State stance) override;

    // Respawn the player at the given position
    void respawn(Point<int16_t> position, bool underwater);

    // Sends a Keyaction to the player's state, to apply forces, change the
    // state and other behaviour.
    void send_action(KeyAction::Id action, bool pressed) override;

    // Recalculates the total stats from base stats, inventories and skills.
    void recalc_stats(bool equipchanged);

    // Change the equipment at the specified slot and recalculate stats.
    void change_equip(int16_t slot);

    // Use the item from the player's inventory with the given id.
    void use_item(int32_t itemid);

    // Return if the player is attacking
    bool is_attacking() const;

    // Return whether the player can attack or not.
    bool can_attack() const;

    // Return whether the player can use a skill or not.
    SpecialMove::ForbidReason can_use(const SpecialMove &move) const;

    // Create an attack struct using the player's stats.
    Attack prepare_attack(bool skill) const;

    // Execute a rush movement
    void rush(double targetx);

    // Check whether the player is invincible
    bool is_invincible() const override;

    // Handle an attack to the player.
    MobAttackResult damage(const MobAttack &attack);

    // Apply a buff to the player
    void give_buff(Buff buff);

    // Cancel a buff.
    void cancel_buff(Buffstat::Id stat);

    // Return whether the buff is active.
    bool has_buff(Buffstat::Id stat) const;

    // Change a skill
    void change_skill(int32_t skill_id,
                      int32_t level,
                      int32_t masterlevel,
                      int64_t expiration);

    // Put a skill on cooldown.
    void add_cooldown(int32_t skill_id, int32_t time);

    // Check if a skill is on cooldown.
    bool has_cooldown(int32_t skill_id) const;

    // Change the player's level, display the "level up" effect.
    void change_level(uint16_t level);

    // Change the player's job, display the job change effect.
    void change_job(uint16_t jobid);

    // Return the character's level
    uint16_t get_level() const override;

    // Return the character's level of a skill.
    int32_t get_skilllevel(int32_t skillid) const override;

    // Return the character's attacking speed.
    int8_t get_integer_attackspeed() const override;

    // Returns the current walking force, calculated from the total ES_SPEED
    // stat.
    float get_walkforce() const;

    // Returns the current jumping force, calculated from the total ES_JUMP
    // stat.
    float get_jumpforce() const;

    // Returns the climbing force, calculated from the total ES_SPEED stat.
    float get_climbforce() const;

    // Returns the flying force.
    float get_flyforce() const;

    uint8_t get_world_id() const;

    uint8_t get_channel_id() const;

    // Return whether the player is underwater
    bool is_underwater() const;

    // Returns if a Keyaction is currently active.
    bool is_key_down(KeyAction::Id action) const;

    // Return a pointer to the ladder the player is on.
    std::optional<std::reference_wrapper<const Ladder>> get_ladder() const;

    // Change players position to the seat's position and stance to
    // Char::State::SIT
    void set_seat(std::optional<std::reference_wrapper<const Seat>> seat);

    // Change players x-pos to the ladder x and change stance to Char::LADDER or
    // Char::ROPE.
    void set_ladder(std::optional<std::reference_wrapper<const Ladder>> ladder);

    void set_channel_id(uint8_t ch);

    // Obtain a reference to the player's stats.
    CharStats &get_stats();

    // Obtain a reference to the player's stats.
    const CharStats &get_stats() const;

    // Obtain a reference to the player's inventory.
    Inventory &get_inventory();

    // Obtain a reference to the player's inventory.
    const Inventory &get_inventory() const;

    // Obtain a reference to the player's skills.
    SkillBook &get_skills();

    // Obtain a reference to the player's questlog.
    QuestLog &get_quests();

    // Obtain a reference to the player's telerock locations.
    TeleportRock &get_teleport_rock();

    // Obtain a reference to the player's monsterbook.
    MonsterBook &get_monsterbook();

    int32_t get_cid() const;

private:
    CharStats stats_;
    Inventory inventory_;
    SkillBook skill_book_;
    QuestLog quest_log_;
    TeleportRock teleport_rock_;
    MonsterBook monster_book_;

    EnumMap<Buffstat::Id, Buff> buffs_;
    ActiveBuffs active_buffs_;
    PassiveBuffs passive_buffs_;

    std::unordered_map<int32_t, int32_t> cooldowns_;

    std::map<KeyAction::Id, bool> keys_down_;

    Movement last_move_;

    Randomizer randomizer_;

    std::optional<std::reference_wrapper<const Ladder>> ladder_;

    bool underwater_;

    int32_t cid_;
    uint8_t wid_;
    uint8_t channel_id_;
};
}  // namespace ms