//	This file is part of the continued Journey MMORPG client
//	Copyright (C) 2015-2020  Daniel Allendorf, Ryan Payton
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
#include "Game.h"
#include <iostream>

//#include "Audio/Audio.h"
//#include "Character/Char.h"
//#include "Configuration.h"
#include "Constants.h"
//#include "Gameplay/Combat/DamageNumber.h"
#include "Gameplay/Stage.h"
#include "IO/UI.h"
#include "IO/Window.h"
#include "Net/PacketProcessor.h"
#include "Timer.h"
#include "Constants.h"
#include "Util/HardwareInfo.h"
#include "Util/NxFiles.h"
#include "ScreenResolution.h"
#include "Singleton.h"

namespace ms {
Game::Game() {
    HardwareInfo();
    ScreenResolution();
}

Error Game::init(android_app *pApp) {
    if (Error error = PacketProcessor::get().init()) {
        return error;
    }
    std::cout << "Session init success." << std::endl;
    if (Error error = NxFiles::init(pApp)) {
        return error;
    }
    std::cout << "NxFiles init success." << std::endl;
    if (Error error = Window::get().init(pApp)) {
        return error;
    }
    std::cout << "Window init success." << std::endl;
    if (Error error = Music::init()) {
        return error;
    }
    std::cout << "Music init success." << std::endl;
    if (Error error = Sound::init()) {
        return error;
    }
    std::cout << "Sound init success." << std::endl;

    Char::init();
    DamageNumber::init();
    MapPortals::init();
    Stage::get().init();
    UI::get().init();

    return Error::NONE;
}

void Game::update() {
    Window::get().check_events();
    Window::get().update();
    Stage::get().update();
    UI::get().update();
    PacketProcessor::get().process();
    Music::update_context();
}

void Game::draw(float alpha) {
    Window::get().begin();
    Stage::get().draw(alpha);
    UI::get().draw(alpha);
    Window::get().end();
}

bool Game::is_running() {
    return PacketProcessor::get().is_connected() && UI::get().not_quitted()
           && Window::get().not_closed();
    return true;
}

void Game::game_loop() {
    Timer::get().start();

    const int64_t timestep = Constants::TIMESTEP * 1000;
    int64_t accumulator = timestep;

    int64_t period = 0;
    int32_t samples = 0;

    bool show_fps = Configuration::get().get_show_fps();

    while (is_running()) {
        int64_t elapsed = Timer::get().stop();

        // Update game with constant timestep as many times as possible.
        for (accumulator += elapsed; accumulator >= timestep;
             accumulator -= timestep) {
            update();
        }
        // Draw the game. Interpolate to account for remaining time.
        float alpha = static_cast<float>(accumulator) / timestep;
        draw(alpha);
        if (show_fps) {
            if (samples < 100) {
                period += elapsed;
                samples++;
            } else if (period) {
                int64_t fps = (samples * 1000000) / period;

                std::cout << "FPS: " << fps << std::endl;

                period = 0;
                samples = 0;
            }
        }
    }

    Sound::close();
}

void Game::start(android_app *pApp) {
    // Initialize and check for errors
    if (Error error = init(pApp)) {
        auto message = error.get_message();
        auto details = error.get_details();
        bool can_retry = error.can_retry();

        std::cout << "Error: " << message << std::endl;
        std::cout << "Message: " << details << std::endl;

        if (can_retry) {
            std::cout << "Enter 'retry' to try again." << std::endl;

            std::string command;
            std::cin >> command;

            if (command == "retry") {
                start(pApp);
            }
        }
    } else {
        game_loop();
    }
}
}  // namespace ms