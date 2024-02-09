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

#include <ft2build.h>
#include FT_FREETYPE_H
#include "glfw/deps/glad/gl.h"
#include <nlnx/bitmap.hpp>
#include <android_native_app_glue.h>

#include "../Constants.h"
#include "../Error.h"
#include "../Util/QuadTree.h"
#include "Text.h"

namespace ms {
// Graphics engine which uses OpenGL
class GraphicsGL : public Singleton<GraphicsGL> {
public:
    GraphicsGL();

    // Initialize all resources
    Error init(android_app *pApp);

    // Re-initialise after changing screen modes.
    void reinit();

    // Clear all bitmaps if most of the space is used up
    void clear();

    // Add a bitmap to the available resources.
    void add_bitmap(const nl::bitmap &bmp);

    // Draw the bitmap with the given parameters.
    void draw(const nl::bitmap &bmp,
              const Rectangle<int16_t> &rect,
              const Color &color,
              float angle);

    // Create a layout for the text with the parameters specified.
    // Text::Layout create_layout(const std::string &text,
    //                            Text::Font font,
    //                            Text::Alignment alignment,
    //                            int16_t maxwidth,
    //                            bool formatted,
    //                            int16_t line_adj);

    Text::Layout create_layout(const std::vector<uint32_t> &text_as_values,
                               Text::Font id,
                               Text::Alignment alignment,
                               int16_t maxwidth,
                               bool formatted,
                               int16_t line_adj);

    // Draw a text with the given parameters.
    // void draw_text(const DrawArgument &args,
    //                const Range<int16_t> &vertical,
    //                const std::string &text,
    //                const Text::Layout &layout,
    //                Text::Font font,
    //                Color::Name color,
    //                Text::Background back);

    void draw_text(const DrawArgument &args,
                   const Range<int16_t> &vertical,
                   const std::vector<uint32_t> &text,
                   const Text::Layout &layout,
                   Text::Font id,
                   Color::Name colorid,
                   Text::Background background);

    // Draw a rectangle filled with the specified color.
    void draw_rectangle(int16_t x,
                        int16_t y,
                        int16_t width,
                        int16_t height,
                        float red,
                        float green,
                        float blue,
                        float alpha);

    // Fill the screen with the specified color.
    void draw_screen_fill(float red, float green, float blue, float alpha);

    // Lock the current scene
    void lock();

    // Unlock the scene.
    void unlock();

    // Draw the buffer contents with the specified scene opacity
    void flush(float opacity);

    // Clear the buffer contents.
    void clear_scene();

private:
    void clear_internal();

    bool add_font(AAssetManager *assetManager,
                  const char *name,
                  Text::Font id,
                  FT_UInt width,
                  FT_UInt height);

    struct Offset {
        GLshort left;
        GLshort right;
        GLshort top;
        GLshort bottom;

        Offset(GLshort x, GLshort y, GLshort width, GLshort height) :
            left(x),
            right(x + width),
            top(y),
            bottom(y + height) {}

        Offset() : left(0), right(0), top(0), bottom(0) {}
    };

    // Add a bitmap to the available resources.
    const Offset &get_offset(const nl::bitmap &bmp);

    struct Leftover {
        GLshort left;
        GLshort right;
        GLshort top;
        GLshort bottom;

        Leftover(GLshort x, GLshort y, GLshort width, GLshort height) {
            left = x;
            right = x + width;
            top = y;
            bottom = y + height;
        }

        Leftover() {
            left = 0;
            right = 0;
            top = 0;
            bottom = 0;
        }

        GLshort width() const { return right - left; }

        GLshort height() const { return bottom - top; }
    };

    struct Quad {
        struct Vertex {
            // Local Space Position
            GLshort localcoord_x;
            GLshort localcoord_y;

            // Texture Coordinates
            GLshort texcoord_x;
            GLshort texcoord_y;

            Color color;
        };

        static const size_t LENGTH = 4;
        Vertex vertices[LENGTH];

        Quad(GLshort left,
             GLshort right,
             GLshort top,
             GLshort bottom,
             const Offset &offset,
             const Color &color,
             GLfloat rotation) {
            vertices[0] = { left, top, offset.left, offset.top, color };
            vertices[1] = { left, bottom, offset.left, offset.bottom, color };
            vertices[2] = { right, bottom, offset.right, offset.bottom, color };
            vertices[3] = { right, top, offset.right, offset.top, color };

            if (rotation != 0.0f) {
                GLfloat cos = std::cos(rotation);
                GLfloat sin = std::sin(rotation);
                GLshort center_x = (left + right) / 2;
                GLshort center_y = (top + bottom) / 2;

                for (size_t i = 0; i < LENGTH; i++) {
                    GLshort vertice_x = vertices[i].localcoord_x - center_x;
                    GLshort vertice_y = vertices[i].localcoord_y - center_y;
                    GLfloat rounded_x =
                        std::roundf(vertice_x * cos - vertice_y * sin);
                    GLfloat rounded_y =
                        std::roundf(vertice_x * sin + vertice_y * cos);
                    vertices[i].localcoord_x =
                        static_cast<GLshort>(rounded_x + center_x);
                    vertices[i].localcoord_y =
                        static_cast<GLshort>(rounded_y + center_y);
                }
            }
        }
    };

    struct Font {
        struct Char {
            GLshort ax;
            GLshort ay;
            GLshort bw;
            GLshort bh;
            GLshort bl;
            GLshort bt;
            Offset offset;
        };

        GLshort width;
        GLshort height;
        Char chars[256];

        Font(GLshort w, GLshort h) : width(w), height(h) {}

        Font() : width(0), height(0) {}

        int16_t linespace() const {
            return static_cast<int16_t>(height * 1.35 + 1);
        }
    };

    class LayoutBuilder {
    public:
        LayoutBuilder(const Font &font,
                      Text::Alignment alignment,
                      int16_t maxwidth,
                      bool formatted,
                      int16_t line_adj);

        // size_t add(const char *text, size_t prev, size_t first, size_t last);

        size_t add(const unsigned int *text,
                   size_t prev,
                   size_t first,
                   size_t last);

        Text::Layout finish(size_t first, size_t last);

    private:
        void add_word(size_t first,
                      size_t last,
                      Text::Font font,
                      Color::Name color);

        void add_line();

        const Font &font_;

        Text::Alignment alignment_;
        Text::Font font_id_;
        Color::Name color_;
        int16_t max_width_;
        bool formatted_;

        int16_t ax_;
        int16_t ay_;

        std::vector<Text::Layout::Line> lines_;
        std::vector<Text::Layout::Word> words_;
        std::vector<int16_t> advances_;
        int16_t width_;
        int16_t endy_;
        int16_t line_adj_;
    };

    int16_t VWIDTH;
    int16_t VHEIGHT;
    Rectangle<int16_t> SCREEN;

    static const GLshort ATLASW = 8192;
    static const GLshort ATLASH = 8192;
    static const GLshort MINLOSIZE = 32;

    bool locked_;

    std::vector<Quad> quads_;
    GLuint VBO_;
    GLuint atlas_;

    GLint shader_program_;
    GLint attribute_coord_;
    GLint attribute_color_;
    GLint uniform_texture_;
    GLint uniform_atlas_size_;
    GLint uniform_screen_size_;
    GLint uniform_yoffset;
    GLint uniform_font_region_;

    std::unordered_map<size_t, Offset> offsets_;
    Offset null_offset_;

    QuadTree<size_t, Leftover> leftovers_;
    size_t rlid_;
    size_t wasted_;
    Point<GLshort> border_;
    Range<GLshort> y_range_;

    FT_Library ft_library_;
    Font fonts_[Text::Font::NUM_FONTS];
    Point<GLshort> font_border;
    GLshort font_ymax;
};
}  // namespace ms