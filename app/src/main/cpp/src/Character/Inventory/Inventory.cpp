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
#include "Inventory.h"

#include <iostream>

#include "../../Data/BulletData.h"
#include "../../Data/EquipData.h"

namespace ms {
Inventory::Inventory() : running_uid_(0), meso_(0), bullet_slot_(0) {
    slot_maxima_[InventoryType::Id::EQUIPPED] = EquipSlot::Id::LENGTH;
}

void Inventory::recalc_stats(Weapon::Type type) {
    total_stats_.clear();

    for (auto &iter : inventories_[InventoryType::Id::EQUIPPED]) {
        auto equip_iter = equips_.find(iter.second.unique_id);

        if (equip_iter != equips_.end()) {
            const Equip &equip = equip_iter->second;

            for (auto stat_iter : total_stats_) {
                stat_iter.second += equip.get_stat(stat_iter.first);
            }
        }
    }

    int32_t prefix = 0;

    switch (type) {
        case Weapon::Type::BOW: prefix = 2060; break;
        case Weapon::Type::CROSSBOW: prefix = 2061; break;
        case Weapon::Type::CLAW: prefix = 2070; break;
        case Weapon::Type::GUN: prefix = 2330; break;
        default: prefix = 0; break;
    }

    bullet_slot_ = 0;

    if (prefix) {
        for (auto &iter : inventories_[InventoryType::Id::USE]) {
            const Slot &slot = iter.second;

            if (slot.count && slot.item_id / 1000 == prefix) {
                bullet_slot_ = iter.first;
                break;
            }
        }
    }

    if (int32_t bulletid = get_bulletid()) {
        total_stats_[EquipStat::Id::WATK] +=
            BulletData::get(bulletid).get_watk();
    }
}

void Inventory::set_meso(int64_t m) {
    meso_ = m;
}

void Inventory::set_slotmax(InventoryType::Id type, uint8_t slotmax) {
    slot_maxima_[type] = slotmax;
}

void Inventory::add_item(InventoryType::Id invtype,
                         int16_t slot,
                         int32_t item_id,
                         bool cash,
                         int64_t expire,
                         uint16_t count,
                         const std::string &owner,
                         int16_t flags) {
    items_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(add_slot(invtype, slot, item_id, count, cash)),
        std::forward_as_tuple(item_id, expire, owner, flags));
}

void Inventory::add_pet(InventoryType::Id invtype,
                        int16_t slot,
                        int32_t item_id,
                        bool cash,
                        int64_t expire,
                        const std::string &name,
                        int8_t level,
                        int16_t closeness,
                        int8_t fullness) {
    pets_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(add_slot(invtype, slot, item_id, 1, cash)),
        std::forward_as_tuple(item_id,
                              expire,
                              name,
                              level,
                              closeness,
                              fullness));
}

void Inventory::add_equip(InventoryType::Id invtype,
                          int16_t slot,
                          int32_t item_id,
                          bool cash,
                          int64_t expire,
                          uint8_t slots,
                          uint8_t level,
                          const EnumMap<EquipStat::Id, uint16_t> &stats,
                          const std::string &owner,
                          int16_t flag,
                          uint8_t ilevel,
                          uint16_t iexp,
                          int32_t vicious) {
    equips_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(add_slot(invtype, slot, item_id, 1, cash)),
        std::forward_as_tuple(item_id,
                              expire,
                              owner,
                              flag,
                              slots,
                              level,
                              stats,
                              ilevel,
                              iexp,
                              vicious));
}

void Inventory::remove(InventoryType::Id type, int16_t slot) {
    auto iter = inventories_[type].find(slot);

    if (iter == inventories_[type].end()) {
        return;
    }

    int32_t unique_id = iter->second.unique_id;
    inventories_[type].erase(iter);

    switch (type) {
        case InventoryType::Id::EQUIPPED:
        case InventoryType::Id::EQUIP: equips_.erase(unique_id); break;
        case InventoryType::Id::CASH:
            items_.erase(unique_id);
            pets_.erase(unique_id);
            break;
        default: items_.erase(unique_id); break;
    }
}

void Inventory::swap(InventoryType::Id firsttype,
                     int16_t firstslot,
                     InventoryType::Id secondtype,
                     int16_t secondslot) {
    Slot first = std::move(inventories_[firsttype][firstslot]);
    inventories_[firsttype][firstslot] =
        std::move(inventories_[secondtype][secondslot]);
    inventories_[secondtype][secondslot] = std::move(first);

    if (!inventories_[firsttype][firstslot].item_id) {
        remove(firsttype, firstslot);
    }

    if (!inventories_[secondtype][secondslot].item_id) {
        remove(secondtype, secondslot);
    }
}

int32_t Inventory::add_slot(InventoryType::Id type,
                            int16_t slot,
                            int32_t item_id,
                            int16_t count,
                            bool cash) {
    running_uid_++;
    inventories_[type][slot] = { running_uid_, item_id, count, cash };

    return running_uid_;
}

void Inventory::change_count(InventoryType::Id type,
                             int16_t slot,
                             int16_t count) {
    auto iter = inventories_[type].find(slot);

    if (iter != inventories_[type].end()) {
        iter->second.count = count;
    }
}

void Inventory::modify(InventoryType::Id type,
                       int16_t slot,
                       int8_t mode,
                       int16_t arg,
                       Movement move) {
    if (slot < 0) {
        slot = -slot;
        type = InventoryType::Id::EQUIPPED;
    }

    arg = (arg < 0) ? -arg : arg;

    switch (mode) {
        case Modification::CHANGE_COUNT: change_count(type, slot, arg); break;
        case Modification::SWAP:
            switch (move) {
                case Movement::MOVE_INTERNAL:
                    swap(type, slot, type, arg);
                    break;
                case Movement::MOVE_UNEQUIP:
                    swap(InventoryType::Id::EQUIPPED,
                         slot,
                         InventoryType::Id::EQUIP,
                         arg);
                    break;
                case Movement::MOVE_EQUIP:
                    swap(InventoryType::Id::EQUIP,
                         slot,
                         InventoryType::Id::EQUIPPED,
                         arg);
                    break;
                default: break;
            }

            break;
        case Modification::REMOVE: remove(type, slot); break;
    }
}

uint8_t Inventory::get_slotmax(InventoryType::Id type) const {
    return slot_maxima_[type];
}

uint16_t Inventory::get_stat(EquipStat::Id type) const {
    return total_stats_[type];
}

int64_t Inventory::get_meso() const {
    return meso_;
}

bool Inventory::has_projectile() const {
    return bullet_slot_ > 0;
}

bool Inventory::has_equipped(EquipSlot::Id slot) const {
    return inventories_[InventoryType::Id::EQUIPPED].count(slot) > 0;
}

int16_t Inventory::get_bulletslot() const {
    return bullet_slot_;
}

uint16_t Inventory::get_bulletcount() const {
    return get_item_count(InventoryType::Id::USE, bullet_slot_);
}

int32_t Inventory::get_bulletid() const {
    return get_item_id(InventoryType::Id::USE, bullet_slot_);
}

EquipSlot::Id Inventory::find_equipslot(int32_t itemid) const {
    const EquipData &cloth = EquipData::get(itemid);

    if (!cloth.is_valid()) {
        return EquipSlot::Id::NONE;
    }

    EquipSlot::Id eqslot = cloth.get_eqslot();

    if (eqslot == EquipSlot::Id::RING1) {
        if (!has_equipped(EquipSlot::Id::RING2)) {
            return EquipSlot::Id::RING2;
        }

        if (!has_equipped(EquipSlot::Id::RING3)) {
            return EquipSlot::Id::RING3;
        }

        if (!has_equipped(EquipSlot::Id::RING4)) {
            return EquipSlot::Id::RING4;
        }

        return EquipSlot::Id::RING1;
    }
    return eqslot;
}

int16_t Inventory::find_free_slot(InventoryType::Id type) const {
    int16_t counter = 1;

    for (const auto &iter : inventories_[type]) {
        if (iter.first != counter) {
            return counter;
        }

        counter++;
    }

    return counter <= slot_maxima_[type] ? counter : 0;
}

int16_t Inventory::find_item(InventoryType::Id type, int32_t itemid) const {
    for (const auto &iter : inventories_[type]) {
        if (iter.second.item_id == itemid) {
            return iter.first;
        }
    }

    return 0;
}

int16_t Inventory::get_item_count(InventoryType::Id type, int16_t slot) const {
    auto iter = inventories_[type].find(slot);

    if (iter != inventories_[type].end()) {
        return iter->second.count;
    }

    return 0;
}

int16_t Inventory::get_total_item_count(int32_t itemid) const {
    InventoryType::Id type = InventoryType::by_item_id(itemid);

    int16_t total_count = 0;

    for (const auto &iter : inventories_[type]) {
        if (iter.second.item_id == itemid) {
            total_count += iter.second.count;
        }
    }

    return total_count;
}

int32_t Inventory::get_item_id(InventoryType::Id type, int16_t slot) const {
    auto iter = inventories_[type].find(slot);

    if (iter != inventories_[type].end()) {
        return iter->second.item_id;
    }

    return 0;
}

std::optional<std::reference_wrapper<const Equip>> Inventory::get_equip(
    InventoryType::Id type,
    int16_t slot) const {
    if (type != InventoryType::Id::EQUIPPED
        && type != InventoryType::Id::EQUIP) {
        return {};
    }

    auto slot_iter = inventories_[type].find(slot);

    if (slot_iter == inventories_[type].end()) {
        return {};
    }

    auto equip_iter = equips_.find(slot_iter->second.unique_id);

    if (equip_iter == equips_.end()) {
        return {};
    }

    return equip_iter->second;
}

Inventory::Movement Inventory::movementbyvalue(int8_t value) {
    if (value >= Inventory::Movement::MOVE_INTERNAL
        && value <= Inventory::Movement::MOVE_EQUIP) {
        return static_cast<Movement>(value);
    }

    std::cout << "Unknown Inventory::Movement value: [" << value << "]"
              << std::endl;

    return Inventory::Movement::MOVE_NONE;
}
}  // namespace ms