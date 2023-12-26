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

#include <cstdint>
#include <functional>
#include <queue>

#include "../Constants.h"

namespace ms {
template<typename T>
class TimedQueue {
public:
    TimedQueue(std::function<void(const T &)> in_action) : action_(in_action) {
        time_ = 0;
    }

    void push(int64_t delay, const T &t) { queue_.emplace(time_ + delay, t); }

    template<typename... Args>
    void emplace(int64_t delay, Args &&... args) {
        queue_.emplace(time_ + delay, std::move(args)...);
    }

    void update(int64_t timestep = Constants::TIMESTEP) {
        time_ += timestep;

        for (; !queue_.empty(); queue_.pop()) {
            const Timed &top = queue_.top();

            if (top.when > time_) {
                break;
            }

            action_(top.value);
        }
    }

private:
    struct Timed {
        T value;
        int64_t when;

        Timed(int64_t w, const T &v) : value { v }, when { w } {}

        template<typename... Args>
        Timed(int64_t w, Args &&... args) :
            value { std::forward<Args>(args)... },
            when { w } {}
    };

    struct TimedComparator {
        bool operator()(const Timed &a, const Timed &b) const {
            return a.when > b.when;
        }
    };

    std::priority_queue<Timed, std::vector<Timed>, TimedComparator> queue_;
    std::function<void(const T &)> action_;
    int64_t time_;
};
}  // namespace ms