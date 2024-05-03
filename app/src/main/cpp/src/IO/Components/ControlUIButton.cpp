//
// Created by 任睿思 on 2024/4/26.
//

#include "ControlUIButton.h"

namespace ms {
    ControlUIButton::ControlUIButton(ms::Point<int16_t> position, const std::string &text) :
            background_(100, 100, Color::Name::BLACK, 0.535f),
            text_(Text::Font::A18M, Text::Alignment::CENTER, Color::Name::YELLOW) {
        position_ = position;
        text_.change_text(text);
        dimension_ = Point<int16_t>(100, 100);
        active_ = false;
        pressed_ = false;
        draggable_ = false;
        state_ = Button::State::NORMAL;
    }

    void ControlUIButton::draw(Point<int16_t> parentpos) const {
        if (active_) {
            background_.draw(position_);
            text_.draw(position_ + Point<int16_t>(50, 25));
        }
    }

    void ControlUIButton::update() {

    }

    Rectangle<int16_t> ControlUIButton::bounds(Point<int16_t> parentpos) const {
        Point<int16_t> absp = position_ + parentpos;
        return {absp, absp + dimension_};
    }

    int16_t ControlUIButton::width() const {
        return dimension_.x();
    }

    Point<int16_t> ControlUIButton::origin() const {
        return position_;
    }

    Cursor::State ControlUIButton::send_cursor(bool clicked, Point<int16_t> cursorpos) {
        return Cursor::State::IDLE;
    }

    bool ControlUIButton::in_combobox(Point<int16_t> cursorpos) {
        if (active_) {
            if (bounds(Point<int16_t>()).contains(cursorpos)) {
                return true;
            }
        }
        return false;
    }

    const std::string& ControlUIButton::get_text() {
        return text_.get_text();
    }

    void ControlUIButton::change_text(const std::string &t) {
        text_.change_text(t);
    }
}
