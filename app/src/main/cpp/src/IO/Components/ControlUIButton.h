//
// Created by 任睿思 on 2024/4/26.
//

#ifndef GLFW_ANDROID_EXAMPLE_CONTROLUIBUTTON_H
#define GLFW_ANDROID_EXAMPLE_CONTROLUIBUTTON_H

#pragma once

#include "Button.h"
#include "Geometry.h"

namespace ms {
    class ControlUIButton : public Button {
    public:
        ControlUIButton(Point<int16_t> position, const std::string &text);

        void draw(Point<int16_t> parentpos) const override;

        void update() override;

        Rectangle<int16_t> bounds(Point<int16_t> parentpos) const override;

        int16_t width() const override;

        Point<int16_t> origin() const override;

        Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

        bool in_combobox(Point<int16_t> cursorpos) override;

        const std::string& get_text();

        void change_text(const std::string &t);

    private:
        ColorBox background_;
        Text text_;

        Point<int16_t> dimension_;
    };
}  // namespace ms


#endif //GLFW_ANDROID_EXAMPLE_CONTROLUIBUTTON_H
