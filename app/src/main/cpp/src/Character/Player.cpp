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
#include "Player.h"

#include "../Data/WeaponData.h"
#include "../IO/UI.h"
#include "../IO/UITypes/UIStatsInfo.h"
#include "../Net/Packets/GameplayPackets.h"
#include "../Net/Packets/InventoryPackets.h"
#include "PlayerStates.h"

namespace ms {
namespace {
auto fn_move_player = [](auto&&... args) {
    MovePlayerPacket(std::forward<decltype(args)>(args)...).dispatch();
};

auto fn_use_item = [](auto&&... args) {
    UseItemPacket(std::forward<decltype(args)>(args)...).dispatch();
};
}  // namespace

const PlayerNullState nullstate;

const PlayerState *get_state(Char::State state) {
    static PlayerStandState standing;
    static PlayerWalkState walking;
    static PlayerFallState falling;
    static PlayerProneState lying;
    static PlayerClimbState climbing;
    static PlayerSitState sitting;
    static PlayerFlyState flying;

    switch (state) {
        case Char::State::STAND: return &standing;
        case Char::State::WALK: return &walking;
        case Char::State::FALL: return &falling;
        case Char::State::PRONE: return &lying;
        case Char::State::LADDER:
        case Char::State::ROPE: return &climbing;
        case Char::State::SIT: return &sitting;
        case Char::State::SWIM: return &flying;
        default: return nullptr;
    }
}

Player::Player(const CharEntry &entry, uint8_t wid, uint8_t channel_id) :
    Char(entry.id, entry.look, entry.stats.name),
    stats_(entry.stats),
    cid_(entry.id),
    wid_(wid),
    channel_id_(channel_id) {
    attacking_ = false;
    underwater_ = false;

    set_state(Char::State::STAND);
    set_direction(true);
}

Player::Player() : Char(0, {}, "") {}

void Player::respawn(Point<int16_t> pos, bool uw) {
    set_position(pos.x(), pos.y());
    underwater_ = uw;
    keys_down_.clear();
    attacking_ = false;
    ladder_ = {};
    nullstate.update_state(*this);
}

void Player::send_action(KeyAction::Id action, bool down) {
    const PlayerState *pst = get_state(state_);

    if (pst) {
        pst->send_action(*this, action, down);
    }

    keys_down_[action] = down;
}

void Player::recalc_stats(bool equipchanged) {
    Weapon::Type weapontype = get_weapontype();

    stats_.set_weapontype(weapontype);
    stats_.init_totalstats();

    if (equipchanged) {
        inventory_.recalc_stats(weapontype);
    }

    for (auto stat : EquipStat::values) {
        int32_t inventory_total = inventory_.get_stat(stat);
        stats_.add_value(stat, inventory_total);
    }

    auto passive_skills = skill_book_.collect_passives();

    for (auto &[skill_id, skill_level] : passive_skills) {
        passive_buffs_.apply_buff(stats_, skill_id, skill_level);
    }

    for (const Buff &buff : buffs_.values()) {
        active_buffs_.apply_buff(stats_, buff.stat, buff.value);
    }

    stats_.close_totalstats();

    if (auto statsinfo = UI::get().get_element<UIStatsInfo>()) {
        statsinfo->get().update_all_stats();
    }
}

void Player::change_equip(int16_t slot) {
    if (int32_t itemid =
            inventory_.get_item_id(InventoryType::Id::EQUIPPED, slot)) {
        look_.add_equip(itemid);
    } else {
        look_.remove_equip(EquipSlot::by_id(slot));
    }
}

void Player::use_item(int32_t itemid) {
    InventoryType::Id type = InventoryType::by_item_id(itemid);

    if (int16_t slot = inventory_.find_item(type, itemid)) {
        if (type == InventoryType::Id::USE) {
            fn_use_item(slot, itemid);
        }
    }
}

void Player::draw(Layer::Id layer,
                  double viewx,
                  double viewy,
                  float alpha) const {
    if (layer == get_layer()) {
        Char::draw(viewx, viewy, alpha);
    }
}

int8_t Player::update(const Physics &physics) {
    const PlayerState *pst = get_state(state_);

    if (pst) {
        pst->update(*this);
        physics.move_object(phobj_);

        bool aniend = Char::update(physics, get_stancespeed());

        if (aniend && attacking_) {
            attacking_ = false;
            nullstate.update_state(*this);
        } else {
            pst->update_state(*this);
        }
    }

    uint8_t stancebyte = facing_right_ ? state_ : state_ + 1;
    Movement newmove(phobj_, stancebyte);
    bool needupdate = last_move_.hasmoved(newmove);

    if (needupdate) {
        fn_move_player(newmove);
        last_move_ = newmove;
    }

    return get_layer();
}

int8_t Player::get_integer_attackspeed() const {
    int32_t weapon_id = look_.get_equips().get_weapon();

    if (weapon_id <= 0) {
        return 0;
    }

    const WeaponData &weapon = WeaponData::get(weapon_id);

    int8_t base_speed = stats_.get_attackspeed();
    int8_t weapon_speed = weapon.get_speed();

    return base_speed + weapon_speed;
}

void Player::set_direction(bool flipped) {
    if (!attacking_) {
        Char::set_direction(flipped);
    }
}

void Player::set_channel_id(uint8_t ch) {
    channel_id_ = ch;
}

void Player::set_state(State st) {
    if (!attacking_) {
        Char::set_state(st);

        const PlayerState *pst = get_state(st);

        if (pst) {
            pst->initialize(*this);
        }
    }
}

bool Player::is_attacking() const {
    return attacking_;
}

bool Player::can_attack() const {
    return !attacking_ && !is_climbing() && !is_sitting()
           && look_.get_equips().has_weapon();
}

SpecialMove::ForbidReason Player::can_use(const SpecialMove &move) const {
    if (move.is_skill() && state_ == Char::State::PRONE) {
        return SpecialMove::ForbidReason::FBR_OTHER;
    }

    if (move.is_attack()
        && (state_ == Char::State::LADDER || state_ == Char::State::ROPE)) {
        return SpecialMove::ForbidReason::FBR_OTHER;
    }

    if (has_cooldown(move.get_id())) {
        return SpecialMove::ForbidReason::FBR_COOLDOWN;
    }

    int32_t level = skill_book_.get_level(move.get_id());
    Weapon::Type weapon = get_weapontype();
    const Job &job = stats_.get_job();
    uint16_t hp = stats_.get_stat(MapleStat::Id::HP);
    uint16_t mp = stats_.get_stat(MapleStat::Id::MP);
    uint16_t bullets = inventory_.get_bulletcount();

    return move.can_use(level, weapon, job, hp, mp, bullets);
}

Attack Player::prepare_attack(bool skill) const {
    Attack::Type attacktype;
    bool degenerate;

    if (state_ == Char::State::PRONE) {
        degenerate = true;
        attacktype = Attack::Type::CLOSE;
    } else {
        Weapon::Type weapontype;
        weapontype = get_weapontype();

        switch (weapontype) {
            case Weapon::Type::BOW:
            case Weapon::Type::CROSSBOW:
            case Weapon::Type::CLAW:
            case Weapon::Type::GUN:
                degenerate = !inventory_.has_projectile();
                attacktype =
                    degenerate ? Attack::Type::CLOSE : Attack::Type::RANGED;
                break;
            case Weapon::Type::WAND:
            case Weapon::Type::STAFF:
                degenerate = !skill;
                attacktype =
                    degenerate ? Attack::Type::CLOSE : Attack::Type::MAGIC;
                break;
            default:
                attacktype = Attack::Type::CLOSE;
                degenerate = false;
                break;
        }
    }

    Attack attack;
    attack.type = attacktype;
    attack.mindamage = stats_.get_mindamage();
    attack.maxdamage = stats_.get_maxdamage();

    if (degenerate) {
        attack.mindamage /= 10;
        attack.maxdamage /= 10;
    }

    attack.critical = stats_.get_critical();
    attack.ignoredef = stats_.get_ignoredef();
    attack.accuracy = stats_.get_total(EquipStat::Id::ACC);
    attack.playerlevel = stats_.get_stat(MapleStat::Id::LEVEL);
    attack.range = stats_.get_range();
    attack.bullet = inventory_.get_bulletid();
    attack.origin = get_position();
    attack.toleft = !facing_right_;
    attack.speed = get_integer_attackspeed();

    return attack;
}

void Player::rush(double targetx) {
    if (phobj_.onground) {
        uint16_t delay = get_attackdelay(1);
        phobj_.movexuntil(targetx, delay);
        phobj_.set_flag(PhysicsObject::Flag::TURN_AT_EDGES);
    }
}

bool Player::is_invincible() const {
    if (state_ == Char::State::DIED) {
        return true;
    }

    if (has_buff(Buffstat::Id::DARKSIGHT)) {
        return true;
    }

    return Char::is_invincible();
}

MobAttackResult Player::damage(const MobAttack &attack) {
    int32_t damage = stats_.calculate_damage(attack.watk);
    show_damage(damage);

    bool fromleft = attack.origin.x() > phobj_.get_x();

    bool missed = damage <= 0;
    bool immovable = ladder_ || state_ == Char::State::DIED;
    bool knockback = !missed && !immovable;

    if (knockback && randomizer_.above(stats_.get_stance())) {
        phobj_.hspeed = fromleft ? -1.5 : 1.5;
        phobj_.vforce -= 3.5;
    }

    uint8_t direction = fromleft ? 0 : 1;

    return { attack, damage, direction };
}

void Player::give_buff(Buff buff) {
    buffs_[buff.stat] = buff;
}

void Player::cancel_buff(Buffstat::Id stat) {
    buffs_[stat] = {};
    remove_recurring_effect();
}

bool Player::has_buff(Buffstat::Id stat) const {
    return buffs_[stat].value > 0;
}

void Player::change_skill(int32_t skill_id,
                          int32_t skill_level,
                          int32_t masterlevel,
                          int64_t expiration) {
    int32_t old_level = skill_book_.get_level(skill_id);
    skill_book_.set_skill(skill_id, skill_level, masterlevel, expiration);

    if (old_level != skill_level) {
        recalc_stats(false);
    }
}

void Player::add_cooldown(int32_t skill_id, int32_t cooltime) {
    cooldowns_[skill_id] = cooltime;
}

bool Player::has_cooldown(int32_t skill_id) const {
    auto iter = cooldowns_.find(skill_id);

    if (iter == cooldowns_.end()) {
        return false;
    }

    return iter->second > 0;
}

void Player::change_level(uint16_t level) {
    uint16_t oldlevel = get_level();

    if (level > oldlevel) {
        show_effect_id(CharEffect::Id::LEVEL_UP);
    }

    stats_.set_stat(MapleStat::Id::LEVEL, level);
}

uint16_t Player::get_level() const {
    return stats_.get_stat(MapleStat::Id::LEVEL);
}

int32_t Player::get_skilllevel(int32_t skillid) const {
    return skill_book_.get_level(skillid);
}

uint8_t Player::get_world_id() const {
    return wid_;
}

uint8_t Player::get_channel_id() const {
    return channel_id_;
}

void Player::change_job(uint16_t jobid) {
    show_effect_id(CharEffect::Id::JOB_CHANGE);
    stats_.change_job(jobid);
}

void Player::set_seat(std::optional<std::reference_wrapper<const Seat>> seat) {
    if (seat) {
        set_position(seat->get().getpos());
        set_state(Char::State::SIT);
    }
}

void Player::set_ladder(
    std::optional<std::reference_wrapper<const Ladder>> ldr) {
    ladder_ = ldr;

    if (ladder_) {
        phobj_.set_x(ldr->get().get_x());
        phobj_.hspeed = 0.0;
        phobj_.vspeed = 0.0;
        phobj_.fhlayer = 7;
        set_state(ldr->get().is_ladder() ? Char::State::LADDER
                                         : Char::State::ROPE);
    }
}

float Player::get_walkforce() const {
    return 0.05f
           + 0.11f * static_cast<float>(stats_.get_total(EquipStat::Id::SPEED))
                 / 100;
}

float Player::get_jumpforce() const {
    return 1.0f
           + 3.5f * static_cast<float>(stats_.get_total(EquipStat::Id::JUMP))
                 / 100;
}

float Player::get_climbforce() const {
    return static_cast<float>(stats_.get_total(EquipStat::Id::SPEED)) / 100;
}

float Player::get_flyforce() const {
    return 0.25f;
}

bool Player::is_underwater() const {
    return underwater_;
}

bool Player::is_key_down(KeyAction::Id action) const {
    return keys_down_.count(action) ? keys_down_.at(action) : false;
}

CharStats &Player::get_stats() {
    return stats_;
}

const CharStats &Player::get_stats() const {
    return stats_;
}

Inventory &Player::get_inventory() {
    return inventory_;
}

const Inventory &Player::get_inventory() const {
    return inventory_;
}

SkillBook &Player::get_skills() {
    return skill_book_;
}

QuestLog &Player::get_quests() {
    return quest_log_;
}

TeleportRock &Player::get_teleport_rock() {
    return teleport_rock_;
}

MonsterBook &Player::get_monsterbook() {
    return monster_book_;
}

std::optional<std::reference_wrapper<const Ladder>> Player::get_ladder() const {
    return ladder_;
}

int32_t Player::get_cid() const {
    return cid_;
}
}  // namespace ms