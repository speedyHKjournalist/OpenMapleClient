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
#include "FootholdTree.h"

#include <iostream>

namespace ms {
FootholdTree::FootholdTree(const nl::node &src) {
    int16_t leftw = 30000;
    int16_t rightw = -30000;
    int16_t botb = -30000;
    int16_t topb = 30000;

    for (const auto &basef : src) {
        uint8_t layer;

        try {
            layer = static_cast<uint8_t>(std::stoi(basef.name()));
        } catch (const std::exception &ex) {
            std::cout << __func__ << ": " << ex.what() << std::endl;
            continue;
        }

        for (const auto &midf : basef) {
            for (auto lastf : midf) {
                uint16_t id;

                try {
                    id = static_cast<uint16_t>(std::stoi(lastf.name()));
                } catch (const std::exception &ex) {
                    std::cout << __func__ << ": " << ex.what() << std::endl;
                    continue;
                }

                const Foothold &foothold =
                    footholds_
                        .emplace(std::piecewise_construct,
                                 std::forward_as_tuple(id),
                                 std::forward_as_tuple(lastf, id, layer))
                        .first->second;

                if (foothold.l() < leftw) {
                    leftw = foothold.l();
                }

                if (foothold.r() > rightw) {
                    rightw = foothold.r();
                }

                if (foothold.b() > botb) {
                    botb = foothold.b();
                }

                if (foothold.t() < topb) {
                    topb = foothold.t();
                }

                if (foothold.is_wall()) {
                    continue;
                }

                int16_t start = foothold.l();
                int16_t end = foothold.r();

                for (int i = start; i <= end; i++) {
                    footholds_by_x_.emplace(i, id);
                }
            }
        }
    }

    walls_ = { leftw + 25, rightw - 25 };
    borders_ = { topb - 300, botb + 100 };
}

FootholdTree::FootholdTree() = default;

void FootholdTree::limit_movement(PhysicsObject &phobj) const {
    if (phobj.hmobile()) {
        double crnt_x = phobj.crnt_x();
        double next_x = phobj.next_x();

        bool left = phobj.hspeed < 0.0f;
        double wall = get_wall(phobj.fhid, left, phobj.next_y());
        bool collision = left ? crnt_x >= wall && next_x <= wall
                              : crnt_x <= wall && next_x >= wall;

        if (!collision
            && phobj.is_flag_set(PhysicsObject::Flag::TURN_AT_EDGES)) {
            wall = get_edge(phobj.fhid, left);
            collision = left ? crnt_x >= wall && next_x <= wall
                             : crnt_x <= wall && next_x >= wall;
        }

        if (collision) {
            phobj.limitx(wall);
            phobj.clear_flag(PhysicsObject::Flag::TURN_AT_EDGES);
        }
    }

    if (phobj.vmobile()) {
        double crnt_y = phobj.crnt_y();
        double next_y = phobj.next_y();

        auto ground =
            Range<double>(get_fh(phobj.fhid).ground_below(phobj.crnt_x()),
                          get_fh(phobj.fhid).ground_below(phobj.next_x()));

        bool collision = crnt_y <= ground.first() && next_y >= ground.second();

        if (collision) {
            phobj.limity(ground.second());

            limit_movement(phobj);
        } else {
            if (next_y < borders_.first()) {
                phobj.limity(borders_.first());
            } else if (next_y > borders_.second()) {
                phobj.limity(borders_.second());
            }
        }
    }
}

void FootholdTree::update_fh(PhysicsObject &phobj) const {
    if (phobj.type == PhysicsObject::Type::FIXATED && phobj.fhid > 0) {
        return;
    }

    const Foothold &curfh = get_fh(phobj.fhid);
    bool checkslope = false;

    double x = phobj.crnt_x();
    double y = phobj.crnt_y();

    if (phobj.onground) {
        if (std::floor(x) > curfh.r()) {
            phobj.fhid = curfh.next();
        } else if (std::ceil(x) < curfh.l()) {
            phobj.fhid = curfh.prev();
        }

        if (phobj.fhid == 0) {
            phobj.fhid = get_fhid_below(x, y);
        } else {
            checkslope = true;
        }
    } else {
        phobj.fhid = get_fhid_below(x, y);

        // fix for stuttering slope walk
        if (phobj.fhid == 0) {
            return;
        }
    }

    const Foothold &nextfh = get_fh(phobj.fhid);
    phobj.fhslope = nextfh.slope();

    double ground = nextfh.ground_below(x);

    if (phobj.vspeed == 0.0 && checkslope) {
        double vdelta = abs(phobj.fhslope);

        if (phobj.fhslope < 0.0) {
            vdelta *= (ground - y);
        } else if (phobj.fhslope > 0.0) {
            vdelta *= (y - ground);
        }

        if (curfh.slope() != 0.0 || nextfh.slope() != 0.0) {
            if (phobj.hspeed > 0.0 && vdelta <= phobj.hspeed) {
                phobj.y = ground;
            } else if (phobj.hspeed < 0.0 && vdelta >= phobj.hspeed) {
                phobj.y = ground;
            }
        }
    }

    phobj.onground = phobj.y == ground;

    if (phobj.enablejd || phobj.is_flag_set(PhysicsObject::Flag::CHECK_BELOW)) {
        uint16_t belowid = get_fhid_below(x, nextfh.ground_below(x) + 1.0);

        if (belowid > 0) {
            double nextground = get_fh(belowid).ground_below(x);
            phobj.enablejd = (nextground - ground) < 600.0;
            phobj.groundbelow = ground + 1.0;
        } else {
            phobj.enablejd = false;
        }

        phobj.clear_flag(PhysicsObject::Flag::CHECK_BELOW);
    }

    if (phobj.fhlayer == 0 || phobj.onground) {
        phobj.fhlayer = nextfh.layer();
    }

    if (phobj.fhid == 0) {
        phobj.fhid = curfh.id();
        phobj.limitx(curfh.x1());
    }
}

const Foothold &FootholdTree::get_fh(uint16_t fhid) const {
    auto iter = footholds_.find(fhid);

    if (iter == footholds_.end()) {
        return null_fh_;
    }

    return iter->second;
}

double FootholdTree::get_wall(uint16_t curid, bool left, double fy) const {
    auto shorty = static_cast<int16_t>(fy);
    Range<int16_t> vertical(shorty - 50, shorty - 1);
    const Foothold &cur = get_fh(curid);

    if (left) {
        const Foothold &prev = get_fh(cur.prev());

        if (prev.is_blocking(vertical)) {
            return cur.l();
        }

        const Foothold &prev_prev = get_fh(prev.prev());

        if (prev_prev.is_blocking(vertical)) {
            return prev.l();
        }

        return walls_.first();
    }

    const Foothold &next = get_fh(cur.next());

    if (next.is_blocking(vertical)) {
        return cur.r();
    }

    const Foothold &next_next = get_fh(next.next());

    if (next_next.is_blocking(vertical)) {
        return next.r();
    }

    return walls_.second();
}

double FootholdTree::get_edge(uint16_t curid, bool left) const {
    const Foothold &fh = get_fh(curid);

    if (left) {
        uint16_t previd = fh.prev();

        if (!previd) {
            return fh.l();
        }

        const Foothold &prev = get_fh(previd);
        uint16_t prev_previd = prev.prev();

        if (!prev_previd) {
            return prev.l();
        }

        return walls_.first();
    }

    uint16_t nextid = fh.next();

    if (!nextid) {
        return fh.r();
    }

    const Foothold &next = get_fh(nextid);
    uint16_t next_nextid = next.next();

    if (!next_nextid) {
        return next.r();
    }

    return walls_.second();
}

uint16_t FootholdTree::get_fhid_below(double fx, double fy) const {
    uint16_t ret = 0;
    double comp = borders_.second();

    auto x = static_cast<int16_t>(fx);
    auto range = footholds_by_x_.equal_range(x);

    for (auto iter = range.first; iter != range.second; ++iter) {
        const Foothold &fh = footholds_.at(iter->second);
        double ycomp = fh.ground_below(fx);

        if (comp >= ycomp && ycomp >= fy) {
            comp = ycomp;
            ret = fh.id();
        }
    }

    return ret;
}

int16_t FootholdTree::get_y_below(Point<int16_t> position) const {
    if (uint16_t fhid = get_fhid_below(position.x(), position.y())) {
        const Foothold &fh = get_fh(fhid);

        return static_cast<int16_t>(fh.ground_below(position.x()));
    }

    return borders_.second();
}

Range<int16_t> FootholdTree::get_walls() const {
    return walls_;
}

Range<int16_t> FootholdTree::get_borders() const {
    return borders_;
}
}  // namespace ms