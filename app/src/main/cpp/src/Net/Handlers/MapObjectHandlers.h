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

#include "../../Character/Buff.h"
#include "../PacketHandler.h"

namespace ms {
// Spawn a character on the stage
// Opcode: SPAWN_CHAR(160)
class SpawnCharHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Remove a character from the stage
// Opcode: REMOVE_CHAR(161)
class RemoveCharHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Spawn a pet on the stage
// Opcode: SPAWN_PET(168)
class SpawnPetHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Move a character
// Opcode: CHAR_MOVED(185)
class CharMovedHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Update the look of a character
// Opcode: UPDATE_CHARLOOK(197)
class UpdateCharLookHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Display an effect on a character
// Opcode: SHOW_FOREIGN_EFFECT(198)
class ShowForeignEffectHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Base class for packets which need to parse buff stats
class ForeignBuffHandler : public PacketHandler {
public:
    void handle(InPacket &recv) const override;

protected:
    virtual void handle_buff(InPacket &recv,
                             int32_t cid,
                             Buffstat::Id stat) const = 0;
};

// Give character a foreign buff
// Opcode: GIVE_FOREIGN_BUFF(199)
class GiveForeignBuffHandler : public ForeignBuffHandler {
    void handle_buff(InPacket &recv,
                     int32_t cid,
                     Buffstat::Id stat) const override;
};

// Cancel a foreign buff on a character
// Opcode: CANCEL_FOREIGN_BUFF(200)
class CancelForeignBuffHandler : public ForeignBuffHandler {
    void handle_buff(InPacket &recv,
                     int32_t cid,
                     Buffstat::Id stat) const override;
};

// Spawn a mob on the stage
// Opcode: SPAWN_MOB(236)
class SpawnMobHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Remove a map from the stage, either by killing it or making it invisible.
// Opcode: KILL_MOB(237)
class KillMobHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Spawn a mob on the stage and take control of it
// Opcode: SPAWN_MOB_C(238)
class SpawnMobControllerHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Update mob state and position with the client
// Opcode: MOB_MOVED(239)
class MobMovedHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Response to a mob move packet
// Opcode: MOB_MOVE_RESPONSE(240)
class MobMoveResponseHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Apply mob status
// Opcode: APPLY_MONSTER_STATUS(242)
class ApplyMobStatusHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Cancel mob status
// Opcode: CANCEL_MONSTER_STATUS(243)
class CancelMobStatusHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Updates a mob's hp with the client
// Opcode: SHOW_MOB_HP(250)
class ShowMobHpHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Spawn an NPC on the current stage
// Opcode: SPAWN_NPC(257)
class SpawnNpcHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Spawn an NPC on the current stage and take control of it
// Opcode: SPAWN_NPC_C(259)
class SpawnNpcControllerHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Spawn an NPC on the current stage and take control of it
// Opcode: SET_NPC_SCRIPTABLE(263)
class SetNpcScriptableHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Drop an item on the stage
// Opcode: DROP_LOOT(268)
class DropLootHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Remove an item from the stage
// Opcode: REMOVE_LOOT(269)
class RemoveLootHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Change state of reactor
// Opcode: HIT_REACTOR(277)
class HitReactorHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Parse a ReactorSpawn and send it to the Stage spawn queue
// Opcode: SPAWN_REACTOR(279)
class SpawnReactorHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};

// Remove a reactor from the stage
// Opcode: REMOVE_REACTOR(280)
class RemoveReactorHandler : public PacketHandler {
    void handle(InPacket &recv) const override;
};
}  // namespace ms