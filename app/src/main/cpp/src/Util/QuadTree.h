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

#include <functional>
#include <unordered_map>
#include <vector>

namespace ms {
template<typename K, typename V>
class QuadTree {
public:
    enum Direction { LEFT, RIGHT, UP, DOWN };

    QuadTree(std::function<Direction(const V &, const V &)> c) :
        root_(0),
        comparator_(c) {}

    QuadTree() : QuadTree(nullptr) {}

    void clear() {
        nodes_.clear();

        root_ = 0;
    }

    void add(K key, V value) {
        K parent = 0;

        if (root_) {
            K current = root_;

            while (current) {
                parent = current;
                current = nodes_[parent].addornext(key, value, comparator_);
            }
        } else {
            root_ = key;
        }

        nodes_.emplace(std::piecewise_construct,
                       std::forward_as_tuple(key),
                       std::forward_as_tuple(value, parent, 0, 0, 0, 0));
    }

    void erase(K key) {
        if (!nodes_.count(key)) {
            return;
        }

        Node &toerase = nodes_[key];

        std::vector<K> leaves;

        for (size_t i = LEFT; i <= DOWN; i++) {
            K leafkey = toerase[i];

            if (leafkey) {
                leaves.push_back(leafkey);
            }
        }

        K parent = toerase.parent;

        if (root_ == key) {
            root_ = 0;
        } else if (nodes_.count(parent)) {
            nodes_[parent].erase(key);
        }

        nodes_.erase(key);

        for (auto &leaf : leaves) {
            readd(parent, leaf);
        }
    }

    K findnode(const V &value,
               std::function<bool(const V &, const V &)> predicate) {
        if (root_) {
            K key = findfrom(root_, value, predicate);

            return predicate(value, nodes_[key].value) ? key : 0;
        }
        return 0;
    }

    V &operator[](K key) { return nodes_[key].value; }

    const V &operator[](K key) const { return nodes_.at(key).value; }

private:
    K findfrom(K start,
               const V &value,
               std::function<bool(const V &, const V &)> predicate) {
        if (!start) {
            return 0;
        }

        bool fulfilled = predicate(value, nodes_[start].value);
        Direction dir = comparator_(value, nodes_[start].value);

        if (dir == RIGHT) {
            K right = findfrom(nodes_[start].right, value, predicate);

            if (right && predicate(value, nodes_[right].value)) {
                return right;
            }
            return start;
        }
        if (dir == DOWN) {
            K bottom = findfrom(nodes_[start].bottom, value, predicate);

            if (bottom && predicate(value, nodes_[bottom].value)) {
                return bottom;
            }
            if (fulfilled) {
                return start;
            }
            K right = findfrom(nodes_[start].right, value, predicate);

            if (right && predicate(value, nodes_[right].value)) {
                return right;
            }

            return start;
        }
        if (dir == UP) {
            K top = findfrom(nodes_[start].top, value, predicate);

            if (top && predicate(value, nodes_[top].value)) {
                return top;
            }
            if (fulfilled) {
                return start;
            }
            K right = findfrom(nodes_[start].right, value, predicate);

            if (right && predicate(value, nodes_[right].value)) {
                return right;
            }

            return start;
        }
        K left = findfrom(nodes_[start].left, value, predicate);

        if (left && predicate(value, nodes_[left].value)) {
            return left;
        }
        if (fulfilled) {
            return start;
        }

        K bottom = findfrom(nodes_[start].bottom, value, predicate);

        if (bottom && predicate(value, nodes_[bottom].value)) {
            return bottom;
        }
        if (fulfilled) {
            return start;
        }

        K top = findfrom(nodes_[start].top, value, predicate);

        if (top && predicate(value, nodes_[top].value)) {
            return top;
        }
        if (fulfilled) {
            return start;
        }

        K right = findfrom(nodes_[start].right, value, predicate);

        if (right && predicate(value, nodes_[right].value)) {
            return right;
        }
        return start;
    }

    void readd(K start, K key) {
        if (start) {
            K parent = 0;
            K current = start;

            while (current) {
                parent = current;
                current = nodes_[parent].addornext(key,
                                                   nodes_[key].value,
                                                   comparator_);
            }

            nodes_[key].parent = parent;
        } else if (start == root_) {
            root_ = key;

            nodes_[key].parent = 0;
        } else if (root_) {
            readd(root_, key);
        }
    }

    struct Node {
        V value;
        K parent;
        K left;
        K right;
        K top;
        K bottom;

        Node(const V &v, K p, K l, K r, K t, K b) :
            value(v),
            parent(p),
            left(l),
            right(r),
            top(t),
            bottom(b) {}

        Node() : Node(V(), 0, 0, 0, 0, 0) {}

        void erase(K key) {
            if (left == key) {
                left = 0;
            } else if (right == key) {
                right = 0;
            } else if (top == key) {
                top = 0;
            } else if (bottom == key) {
                bottom = 0;
            }
        }

        K addornext(K key,
                    V val,
                    std::function<Direction(const V &, const V &)> comparator) {
            Direction dir = comparator(val, value);
            K dirkey = leaf(dir);

            if (!dirkey) {
                switch (dir) {
                    case LEFT: left = key; break;
                    case RIGHT: right = key; break;
                    case UP: top = key; break;
                    case DOWN: bottom = key; break;
                }
            }

            return dirkey;
        }

        K leaf(Direction dir) {
            switch (dir) {
                case LEFT: return left;
                case RIGHT: return right;
                case UP: return top;
                case DOWN: return bottom;
                default: return 0;
            }
        }

        K operator[](size_t d) {
            auto dir = static_cast<Direction>(d);

            return leaf(dir);
        }
    };

    std::function<Direction(const V &, const V &)> comparator_;
    std::unordered_map<K, Node> nodes_;
    K root_;
};
}  // namespace ms