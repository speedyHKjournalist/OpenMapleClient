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
#include "Texture.h"

#include "GraphicsGL.h"

namespace ms {
Texture::Texture(nl::node src) {
    if (src.data_type() == nl::node::type::bitmap) {
        origin_ = src["origin"];

        std::string source = src["source"];
        std::string _outlink = src["_outlink"];
        std::string _inlink = src["_inlink"];

        auto foundSource = find_child(src, source);
        auto foundOutlink = find_child(src, _outlink);

        auto foundChild = foundSource || foundOutlink;

        if (foundChild) {
            src = foundSource ? foundSource : foundOutlink;
        }

        if (!foundChild && !_inlink.empty()) {
            auto parent_node = src.root();

            for (auto child_node = parent_node.begin();
                 child_node != parent_node.end();
                 ++child_node) {
                auto found_node = child_node.resolve(_inlink);

                if (found_node.data_type() == nl::node::type::bitmap) {
                    src = found_node;
                    break;
                }
            }
        }

        bitmap_ = src;
        dimensions_ = Point<int16_t>(bitmap_.width(), bitmap_.height());

        GraphicsGL::get().add_bitmap(bitmap_);
    }
}

Texture::Texture() = default;

void Texture::draw(const DrawArgument &args) const {
    size_t id = bitmap_.id();

    if (id == 0) {
        return;
    }

    GraphicsGL::get().draw(bitmap_,
                           args.get_rectangle(origin_, dimensions_),
                           args.get_color(),
                           args.get_angle());
}

void Texture::shift(Point<int16_t> amount) {
    origin_ -= amount;
}

bool Texture::is_valid() const {
    return bitmap_.id() > 0;
}

int16_t Texture::width() const {
    return dimensions_.x();
}

int16_t Texture::height() const {
    return dimensions_.y();
}

Point<int16_t> Texture::get_origin() const {
    return origin_;
}

Point<int16_t> Texture::get_dimensions() const {
    return dimensions_;
}

nl::node Texture::find_child(const nl::node &source, const std::string &link) {
    if (!link.empty()) {
        nl::node parent_node = source.root();

        return parent_node.resolve(link.substr(link.find('/') + 1));
    }

    return nl::node();
}
}  // namespace ms