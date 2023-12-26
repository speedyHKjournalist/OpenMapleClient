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

#include <map>
#include <string>

namespace ms {
// Class that stores information on the quest log of an individual character
class QuestLog {
public:
    void add_started(int16_t qid, const std::string &quest_data);

    void add_in_progress(int16_t qid,
                         int16_t qidl,
                         const std::string &quest_data);

    void add_completed(int16_t qid, int64_t time);

    bool is_started(int16_t qid) const;

    bool is_completed(int16_t qid) const;

    int16_t get_last_started() const;

private:
    std::map<int16_t, std::string> started_;
    std::map<int16_t, std::pair<int16_t, std::string>> in_progress_;
    std::map<int16_t, int64_t> completed_;
};
}  // namespace ms
