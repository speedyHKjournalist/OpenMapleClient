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
#include "Audio.h"

#include <bass.h>

#include <nlnx/audio.hpp>
#include <nlnx/nx.hpp>
#include <utility>

#include "../Configuration.h"

namespace ms {
Sound::Sound(Name name) : id_(soundids_[name]) {}

Sound::Sound(int32_t itemid) {
    auto fitemid = format_id(itemid);

    if (itemids_.find(fitemid) != itemids_.end()) {
        id_ = itemids_.at(fitemid);
    } else {
        auto pid = (10000 * (itemid / 10000));
        auto fpid = format_id(pid);

        if (itemids_.find(fpid) != itemids_.end()) {
            id_ = itemids_.at(fpid);
        } else {
            id_ = itemids_.at("02000000");
        }
    }
}

Sound::Sound(const nl::node &src) : id_(add_sound(src)) {}

Sound::Sound() : id_(0) {}

void Sound::play() const {
    if (id_ > 0) {
        play(id_);
    }
}

Error Sound::init() {
    if (!BASS_Init(-1, 44100, 0, nullptr, 0)) {
        return Error::Code::AUDIO;
    }

    nl::node uisrc = nl::nx::sound["UI.img"];

    add_sound(Sound::Name::BUTTON_CLICK, uisrc["BtMouseClick"]);
    add_sound(Sound::Name::BUTTON_OVER, uisrc["BtMouseOver"]);
    add_sound(Sound::Name::CHAR_SELECT, uisrc["CharSelect"]);
    add_sound(Sound::Name::DLG_NOTICE, uisrc["DlgNotice"]);
    add_sound(Sound::Name::MENU_DOWN, uisrc["MenuDown"]);
    add_sound(Sound::Name::MENU_UP, uisrc["MenuUp"]);
    add_sound(Sound::Name::RACE_SELECT, uisrc["RaceSelect"]);
    add_sound(Sound::Name::SCROLL_UP, uisrc["ScrollUp"]);
    add_sound(Sound::Name::SELECT_MAP, uisrc["SelectMap"]);
    add_sound(Sound::Name::TAB, uisrc["Tab"]);
    add_sound(Sound::Name::WORLD_SELECT, uisrc["WorldSelect"]);
    add_sound(Sound::Name::DRAG_START, uisrc["DragStart"]);
    add_sound(Sound::Name::DRAG_END, uisrc["DragEnd"]);
    add_sound(Sound::Name::WORLD_MAP_OPEN, uisrc["WorldmapOpen"]);
    add_sound(Sound::Name::WORLD_MAP_CLOSE, uisrc["WorldmapClose"]);

    nl::node gamesrc = nl::nx::sound["Game.img"];

    add_sound(Sound::Name::GAME_START, gamesrc["GameIn"]);
    add_sound(Sound::Name::JUMP, gamesrc["Jump"]);
    add_sound(Sound::Name::DROP, gamesrc["DropItem"]);
    add_sound(Sound::Name::PICKUP, gamesrc["PickUpItem"]);
    add_sound(Sound::Name::PORTAL, gamesrc["Portal"]);
    add_sound(Sound::Name::LEVEL_UP, gamesrc["LevelUp"]);
    add_sound(Sound::Name::TOMBSTONE, gamesrc["Tombstone"]);

    nl::node itemsrc = nl::nx::sound["Item.img"];

    for (const auto &node : itemsrc) {
        add_sound(node.name(), node["Use"]);
    }

    uint8_t volume = Setting<SFXVolume>::get().load();

    if (!set_sfxvolume(volume)) {
        return Error::Code::AUDIO;
    }

    return Error::Code::NONE;
}

void Sound::close() {
    BASS_Free();
}

bool Sound::set_sfxvolume(uint8_t vol) {
    return BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, vol * 100) == TRUE;
}

void Sound::play(size_t id) {
    if (!samples_.count(id)) {
        return;
    }

    auto channel =
        BASS_SampleGetChannel(static_cast<HSAMPLE>(samples_.at(id)), false);
    BASS_ChannelPlay(channel, true);
}

size_t Sound::add_sound(const nl::node &src) {
    nl::audio ad = src;

    const auto *data = reinterpret_cast<const void *>(ad.data());

    if (data) {
        size_t id = ad.id();

        if (auto sample =
                BASS_SampleLoad(true,
                                data,
                                82,  // not used when mem (1st param) is true
                                static_cast<DWORD>(ad.length()),
                                4,
                                BASS_SAMPLE_OVER_POS);
            sample != 0) {
            samples_[id] = sample;
        } else {
            const auto *data2 = reinterpret_cast<const char *>(ad.data());
            auto sample2 =
                BASS_SampleLoad(true,
                                reinterpret_cast<const void *>(data2 + 82),
                                82,  // not used when mem (1st param) is true
                                static_cast<DWORD>(ad.length()),
                                4,
                                BASS_SAMPLE_OVER_POS);

            if (sample2 == 0) {
//                auto ec = BASS_ErrorGetCode();
            } else {
                samples_[id] = sample2;
            }
        }

        return id;
    }

    return 0;
}

void Sound::add_sound(Name name, const nl::node &src) {
    size_t id = add_sound(src);

    if (id) {
        soundids_[name] = id;
    }
}

void Sound::add_sound(const std::string &itemid, const nl::node &src) {
    size_t id = add_sound(src);

    if (id) {
        itemids_[itemid] = id;
    }
}

std::string Sound::format_id(int32_t itemid) {
    std::string strid = std::to_string(itemid);
    strid.insert(0, 8 - strid.size(), '0');

    return strid;
}

std::unordered_map<size_t, uint64_t> Sound::samples_;
EnumMap<Sound::Name, size_t> Sound::soundids_;
std::unordered_map<std::string, size_t> Sound::itemids_;

Music::Music(std::string p) {
    path_ = std::move(p);
}

void Music::play() const {
    static HSTREAM stream = 0;
    static std::string bgmpath = "";

    if (path_ == bgmpath) {
        return;
    }

    nl::audio ad = nl::nx::sound.resolve(path_);
    const auto *data = reinterpret_cast<const void *>(ad.data());

    if (data) {
        if (stream) {
            BASS_ChannelStop(stream);
            BASS_StreamFree(stream);
        }

        stream = BASS_StreamCreateFile(true,
                                       data,
                                       82,
                                       ad.length(),
                                       BASS_SAMPLE_FLOAT | BASS_SAMPLE_LOOP);
        BASS_ChannelPlay(stream, true);

        bgmpath = path_;
    }
}

void Music::play_once() const {
    static HSTREAM stream = 0;
    static std::string bgmpath = "";

    if (path_ == bgmpath) {
        return;
    }

    nl::audio ad = nl::nx::sound.resolve(path_);
    const auto *data = reinterpret_cast<const void *>(ad.data());

    if (data) {
        if (stream) {
            BASS_ChannelStop(stream);
            BASS_StreamFree(stream);
        }

        stream = BASS_StreamCreateFile(true,
                                       data,
                                       82,
                                       ad.length(),
                                       BASS_SAMPLE_FLOAT);
        BASS_ChannelPlay(stream, true);

        bgmpath = path_;
    }
}

Error Music::init() {
    uint8_t volume = Setting<BGMVolume>::get().load();

    if (!set_bgmvolume(volume)) {
        return Error::Code::AUDIO;
    }

    return Error::Code::NONE;
}

bool Music::set_bgmvolume(uint8_t vol) {
    return BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, vol * 100) == TRUE;
}

void Music::update_context() {}
}  // namespace ms