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
#include "GraphicsGL.h"
#include "glfw/deps/glad_gl.c"
#include <algorithm>
#include <iostream>

#include "../Configuration.h"
#include "Window.h"

namespace ms {
    GraphicsGL::GraphicsGL() : locked_(false) {
        VWIDTH = Constants::Constants::get().get_viewwidth();
        VHEIGHT = Constants::Constants::get().get_viewheight();
        SCREEN = Rectangle<int16_t>(0, VWIDTH, 0, VHEIGHT);
    }

    Error GraphicsGL::init(android_app *pApp) {
        // Set image to left top, multiplied by phone (screensize y / 300), image resolution is 800 * 600
        // Frag color changes to bgra because color is incorrect on Android platform
        const char *vertexShaderSource =
                "#version 320 es\n"
                "precision highp float;"
                "in vec4 coord;"
                "in vec4 color;"
                "out vec2 texpos;"
                "out vec4 colormod;"
                "uniform vec2 screensize;"
                "uniform int yoffset;"

                "void main(void)"
                "{"
                "	float x = (-1.0 + coord.x * (screensize.y / 300.0) / screensize.x);"
                "	float y = 1.0 - (coord.y + float(yoffset)) / 300.0;"
                "   gl_Position = vec4(x, y, 0.0, 1.0);"
                "	texpos = coord.zw;"
                "	colormod = color;"
                "}";

        const char *fragmentShaderSource =
                "#version 320 es\n"
                "precision highp float;"
                "in vec2 texpos;"
                "in vec4 colormod;"
                "out vec4 FragColor;"
                "uniform sampler2D texture;"
                "uniform vec2 atlassize;"
                "uniform int fontregion;"

                "void main(void)"
                "{"
                "	if (texpos.y == 0.0)"
                "	{"
                "		FragColor = colormod;"
                "	}"
                "	else if (texpos.y <= float(fontregion))"
                "	{"
                "		FragColor = vec4(1.0, 1.0, 1.0, texture(texture, texpos / "
                "atlassize).r) * colormod;"
                "	}"
                "	else"
                "	{"
                "		FragColor = texture(texture, texpos / atlassize).bgra * "
                "colormod;"
                "	}"
                "}";

        const GLsizei bufSize = 512;

        GLint success;
        GLchar infoLog[bufSize];

        if (!gladLoadGL(glfwGetProcAddress)) {
            printf("Something went wrong!\n");
            exit(-1);
        }

        std::cout << "Using OpenGL " << glGetString(GL_VERSION) << std::endl;
        // std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;

        if (FT_Init_FreeType(&ft_library_)) {
            return Error::Code::FREETYPE;
        }

        FT_Int ftmajor;
        FT_Int ftminor;
        FT_Int ftpatch;

        FT_Library_Version(ft_library_, &ftmajor, &ftminor, &ftpatch);

        std::cout << "Using FreeType " << ftmajor << "." << ftminor << "."
                  << ftpatch << std::endl;

        // Build and compile our shader program
        // ------------------------------------

        // Vertex Shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        // Check for shader compile errors
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

        if (success != GL_TRUE) {
            glGetShaderInfoLog(vertexShader, bufSize, nullptr, infoLog);

            return Error(Error::Code::VERTEX_SHADER, infoLog);
        }

        // Fragment Shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        // Check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

        if (success != GL_TRUE) {
            glGetShaderInfoLog(fragmentShader, bufSize, nullptr, infoLog);

            return Error(Error::Code::FRAGMENT_SHADER, infoLog);
        }

        // Link Shaders
        shader_program_ = glCreateProgram();
        glAttachShader(shader_program_, vertexShader);
        glAttachShader(shader_program_, fragmentShader);
        glLinkProgram(shader_program_);

        // Check for linking errors
        glGetProgramiv(shader_program_, GL_LINK_STATUS, &success);

        if (success != GL_TRUE) {
            glGetProgramInfoLog(shader_program_, bufSize, nullptr, infoLog);

            return Error(Error::Code::SHADER_PROGRAM_LINK, infoLog);
        }

        // Validate Program
        glValidateProgram(shader_program_);

        // Check for validation errors
        glGetProgramiv(shader_program_, GL_VALIDATE_STATUS, &success);

        if (success != GL_TRUE) {
            glGetProgramInfoLog(shader_program_, bufSize, nullptr, infoLog);

            return Error(Error::Code::SHADER_PROGRAM_VALID, infoLog);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        attribute_coord_ = glGetAttribLocation(shader_program_, "coord");
        attribute_color_ = glGetAttribLocation(shader_program_, "color");
        uniform_texture_ = glGetUniformLocation(shader_program_, "texture");
        uniform_atlas_size_ = glGetUniformLocation(shader_program_, "atlassize");
        uniform_screen_size_ = glGetUniformLocation(shader_program_, "screensize");
        uniform_yoffset = glGetUniformLocation(shader_program_, "yoffset");
        uniform_font_region_ = glGetUniformLocation(shader_program_, "fontregion");

        if (attribute_coord_ == -1 || attribute_color_ == -1
            || uniform_texture_ == -1 || uniform_atlas_size_ == -1
            || uniform_screen_size_ == -1 || uniform_yoffset == -1) {
            return Error::Code::SHADER_VARS;
        }

        // Vertex Buffer Object
        glGenBuffers(1, &VBO_);

        glGenTextures(1, &atlas_);
        glBindTexture(GL_TEXTURE_2D, atlas_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     ATLASW,
                     ATLASH,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     nullptr);

        font_border.set_y(1);

        const std::string FONT_NORMAL = Setting<FontPathNormal>().get().load();
        const std::string FONT_BOLD = Setting<FontPathBold>().get().load();

        if (FONT_NORMAL.empty() || FONT_BOLD.empty()) {
            return Error::Code::FONT_PATH;
        }

        const char *FONT_NORMAL_STR = FONT_NORMAL.c_str();
        const char *FONT_BOLD_STR = FONT_BOLD.c_str();
        AAssetManager* assetManager = pApp->activity->assetManager;

        add_font(assetManager, FONT_NORMAL_STR, Text::Font::A11M, 0, 11);
        add_font(assetManager, FONT_BOLD_STR, Text::Font::A11B, 0, 11);
        add_font(assetManager, FONT_NORMAL_STR, Text::Font::A12M, 0, 12);
        add_font(assetManager, FONT_BOLD_STR, Text::Font::A12B, 0, 12);
        add_font(assetManager, FONT_NORMAL_STR, Text::Font::A13M, 0, 13);
        add_font(assetManager, FONT_BOLD_STR, Text::Font::A13B, 0, 13);
        add_font(assetManager, FONT_BOLD_STR, Text::Font::A15B, 0, 15);
        add_font(assetManager, FONT_NORMAL_STR, Text::Font::A18M, 0, 18);

        font_ymax += font_border.y();

        leftovers_ = QuadTree<size_t, Leftover>(
                [](const Leftover &first, const Leftover &second) {
                    bool width_comparison = first.width() >= second.width();
                    bool height_comparison = first.height() >= second.height();

                    if (width_comparison && height_comparison) {
                        return QuadTree<size_t, Leftover>::Direction::RIGHT;
                    }
                    if (width_comparison) {
                        return QuadTree<size_t, Leftover>::Direction::DOWN;
                    }
                    if (height_comparison) {
                        return QuadTree<size_t, Leftover>::Direction::UP;
                    }
                    return QuadTree<size_t, Leftover>::Direction::LEFT;
                });

        return Error::Code::NONE;
    }

    bool GraphicsGL::add_font(AAssetManager *assetManager,
                              const char *name,
                              Text::Font id,
                              FT_UInt pixelw,
                              FT_UInt pixelh) {
        FT_Face face;
        AAsset* fontFile = AAssetManager_open(assetManager, name, AASSET_MODE_BUFFER);
        off_t fontDataSize = AAsset_getLength(fontFile);

        FT_Byte* fontData = new FT_Byte[fontDataSize];
        AAsset_read(fontFile, fontData, (size_t) fontDataSize);
        AAsset_close(fontFile);

        if (FT_New_Memory_Face(ft_library_, (const FT_Byte*)fontData, (FT_Long)fontDataSize, 0, &face)) {
            std::cerr << "Error: Failed to create new face." << std::endl;
            return false;
        }

        if (FT_Set_Pixel_Sizes(face, pixelw, pixelh)) {
            std::cerr << "Error: Failed to set pixel sizes." << std::endl;
            return false;
        }

        FT_GlyphSlot g = face->glyph;

        GLshort width = 0;
        GLshort height = 0;

        for (uint32_t c = 32; c < 256; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "Error: Failed to load a glyph into the glyph slot of "
                             "a face object."
                          << std::endl;
                continue;
            }

            auto w = static_cast<GLshort>(g->bitmap.width);
            auto h = static_cast<GLshort>(g->bitmap.rows);

            width += w;

            if (h > height) {
                height = h;
            }
        }

        if (font_border.x() + width > ATLASW) {
            font_border.set_x(0);
            font_border.set_y(font_ymax);
            font_ymax = 0;
        }

        GLshort x = font_border.x();
        GLshort y = font_border.y();

        font_border.shift_x(width);

        if (height > font_ymax) {
            font_ymax = height;
        }

        fonts_[id] = Font(width, height);

        GLshort ox = x;
        GLshort oy = y;

        for (uint32_t c = 32; c < 256; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "Error: Failed to load a glyph into the glyph slot of "
                             "a face object."
                          << std::endl;
                continue;
            }

            auto g = face->glyph;

            // Render the glyph in 8-bit grayscale mode
            FT_Render_Glyph(g, FT_RENDER_MODE_NORMAL);

            // Convert grayscale to BGRA
            FT_Byte* bgraBuffer = new FT_Byte[g->bitmap.rows * g->bitmap.width * 4];
            for (int i = 0; i < g->bitmap.rows * g->bitmap.width; ++i) {
                // Grayscale value
                auto intensity = g->bitmap.buffer[i];

                // Set BGRA components
                bgraBuffer[i * 4] = intensity;          // Blue channel
                bgraBuffer[i * 4 + 1] = intensity;      // Green channel
                bgraBuffer[i * 4 + 2] = intensity;      // Red channel
                bgraBuffer[i * 4 + 3] = 255;            // Alpha channel (fully opaque)
            }

            auto ax = static_cast<GLshort>(g->advance.x >> 6);
            auto ay = static_cast<GLshort>(g->advance.y >> 6);
            auto l = static_cast<GLshort>(g->bitmap_left);
            auto t = static_cast<GLshort>(g->bitmap_top);
            auto w = static_cast<GLshort>(g->bitmap.width);
            auto h = static_cast<GLshort>(g->bitmap.rows);

            glTexSubImage2D(GL_TEXTURE_2D,
                            0,
                            ox,
                            oy,
                            w,
                            h,
                            GL_RGBA,
                            GL_UNSIGNED_BYTE,
                            bgraBuffer);
            delete[] bgraBuffer;

            Offset offset = Offset(ox, oy, w, h);
            fonts_[id].chars[c] = { ax, ay, w, h, l, t, offset };

            ox += w;
        }

        return true;
    }

    void GraphicsGL::reinit() {
        int32_t new_width = Constants::Constants::get().get_viewwidth();
        int32_t new_height = Constants::Constants::get().get_viewheight();

        if (VWIDTH != new_width || VHEIGHT != new_height) {
            VWIDTH = new_width;
            VHEIGHT = new_height;
            SCREEN = Rectangle<int16_t>(0, VWIDTH, 0, VHEIGHT);
        }

        glUseProgram(shader_program_);

        glUniform1i(uniform_font_region_, font_ymax);
        glUniform2f(uniform_atlas_size_, ATLASW, ATLASH);
        glUniform2f(uniform_screen_size_, VWIDTH, VHEIGHT);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glVertexAttribPointer(attribute_coord_,
                              4,
                              GL_SHORT,
                              GL_FALSE,
                              sizeof(Quad::Vertex),
                              0);
        glVertexAttribPointer(attribute_color_,
                              4,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Quad::Vertex),
                              (const void *)offsetof(Quad::Vertex, color));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindTexture(GL_TEXTURE_2D, atlas_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        clear_internal();
    }

    void GraphicsGL::clear_internal() {
        border_ = Point<GLshort>(0, font_ymax);
        y_range_ = Range<GLshort>();

        offsets_.clear();
        leftovers_.clear();
        rlid_ = 1;
        wasted_ = 0;
    }

    void GraphicsGL::clear() {
        size_t used = ATLASW * border_.y() + border_.x() * y_range_.second();
        double usedpercent = static_cast<double>(used) / (ATLASW * ATLASH);

        if (usedpercent > 80.0) {
            clear_internal();
        }
    }

    void GraphicsGL::add_bitmap(const nl::bitmap &bmp) {
        get_offset(bmp);
    }

    const GraphicsGL::Offset &GraphicsGL::get_offset(const nl::bitmap &bmp) {
        size_t id = bmp.id();
        auto offiter = offsets_.find(id);

        if (offiter != offsets_.end()) {
            return offiter->second;
        }

        GLshort x = 0;
        GLshort y = 0;
        GLshort width = bmp.width();
        GLshort height = bmp.height();

        if (width <= 0 || height <= 0) {
            return null_offset_;
        }

        Leftover value = Leftover(x, y, width, height);

        size_t lid = leftovers_.findnode(
                value,
                [](const Leftover &val, const Leftover &leaf) {
                    return val.width() <= leaf.width() && val.height() <= leaf.height();
                });

        if (lid > 0) {
            const Leftover &leftover = leftovers_[lid];

            x = leftover.left;
            y = leftover.top;

            GLshort width_delta = leftover.width() - width;
            GLshort height_delta = leftover.height() - height;

            leftovers_.erase(lid);

            wasted_ -= width * height;

            if (width_delta >= MINLOSIZE && height_delta >= MINLOSIZE) {
                leftovers_.add(
                        rlid_,
                        Leftover(x + width, y + height, width_delta, height_delta));
                rlid_++;

                if (width >= MINLOSIZE) {
                    leftovers_.add(rlid_,
                                   Leftover(x, y + height, width, height_delta));
                    rlid_++;
                }

                if (height >= MINLOSIZE) {
                    leftovers_.add(rlid_,
                                   Leftover(x + width, y, width_delta, height));
                    rlid_++;
                }
            } else if (width_delta >= MINLOSIZE) {
                leftovers_.add(
                        rlid_,
                        Leftover(x + width, y, width_delta, height + height_delta));
                rlid_++;
            } else if (height_delta >= MINLOSIZE) {
                leftovers_.add(
                        rlid_,
                        Leftover(x, y + height, width + width_delta, height_delta));
                rlid_++;
            }
        } else {
            if (border_.x() + width > ATLASW) {
                border_.set_x(0);
                border_.shift_y(y_range_.second());

                if (border_.y() + height > ATLASH) {
                    clear_internal();
                } else {
                    y_range_ = Range<GLshort>();
                }
            }

            x = border_.x();
            y = border_.y();

            border_.shift_x(width);

            if (height > y_range_.second()) {
                if (x >= MINLOSIZE && height - y_range_.second() >= MINLOSIZE) {
                    leftovers_.add(rlid_,
                                   Leftover(0,
                                            y_range_.first(),
                                            x,
                                            height - y_range_.second()));
                    rlid_++;
                }

                wasted_ += x * (height - y_range_.second());

                y_range_ = Range<int16_t>(y + height, height);
            } else if (height < y_range_.first() - y) {
                if (width >= MINLOSIZE
                    && y_range_.first() - y - height >= MINLOSIZE) {
                    leftovers_.add(rlid_,
                                   Leftover(x,
                                            y + height,
                                            width,
                                            y_range_.first() - y - height));
                    rlid_++;
                }

                wasted_ += width * (y_range_.first() - y - height);
            }
        }

        // size_t used = ATLASW * border.y() + border.x() * yrange.second();
        //
        // double usedpercent = static_cast<double>(used) / (ATLASW * ATLASH);
        // double wastedpercent = static_cast<double>(wasted) / used;
        //
        // std::cout << "Used: [" << usedpercent << "] Wasted: [" << wastedpercent
        // << "]" << std::endl;

        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        x,
                        y,
                        width,
                        height,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        bmp.data());

        return offsets_
                .emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple(x, y, width, height))
                .first->second;
    }

    void GraphicsGL::draw(const nl::bitmap &bmp,
                          const Rectangle<int16_t> &rect,
                          const Color &color,
                          float angle) {
        if (locked_) {
            return;
        }

        if (color.invisible()) {
            return;
        }

        if (!rect.overlaps(SCREEN)) {
            return;
        }

        quads_.emplace_back(rect.left(),
                            rect.right(),
                            rect.top(),
                            rect.bottom(),
                            get_offset(bmp),
                            color,
                            angle);
    }

// Text::Layout GraphicsGL::create_layout(const std::string &text,
//                                        Text::Font id,
//                                        Text::Alignment alignment,
//                                        int16_t maxwidth,
//                                        bool formatted,
//                                        int16_t line_adj) {
//     size_t length = text.length();

//     if (length == 0) {
//         return Text::Layout();
//     }

//     LayoutBuilder builder(fonts_[id], alignment, maxwidth, formatted,
//     line_adj);

//     const char *p_text = text.c_str();

//     size_t first = 0;
//     size_t offset = 0;

//     while (offset < length) {
//         size_t last = text.find_first_of(" \\#", offset + 1);

//         if (last == std::string::npos) {
//             last = length;
//         }

//         first = builder.add(p_text, first, offset, last);
//         offset = last;
//     }

//     return builder.finish(first, offset);
// }

    Text::Layout GraphicsGL::create_layout(
            const std::vector<uint32_t> &text_as_values,
            Text::Font id,
            Text::Alignment alignment,
            int16_t maxwidth,
            bool formatted,
            int16_t line_adj) {
        size_t length = text_as_values.size();

        if (length == 0) {
            return Text::Layout();
        }

        LayoutBuilder builder(fonts_[id], alignment, maxwidth, formatted, line_adj);

        const auto *p_text = text_as_values.data();

        size_t first = 0;
        size_t offset = 0;

        std::vector<uint32_t> to_find = { ' ', '\\', '#' };

        while (offset < length) {
            auto it = std::find_first_of(text_as_values.begin() + offset + 1,
                                         text_as_values.end(),
                                         to_find.begin(),
                                         to_find.end());
            size_t last = it - text_as_values.begin();

            if (it == text_as_values.end()) {
                last = length;
            }

            first = builder.add(p_text, first, offset, last);
            offset = last;
        }

        return builder.finish(first, offset);
    }

    size_t GraphicsGL::LayoutBuilder::add(const unsigned int *text,
                                          size_t prev,
                                          size_t first,
                                          size_t last) {
        if (first == last) {
            return prev;
        }

        Text::Font last_font = font_id_;
        Color::Name last_color = color_;
        size_t skip = 0;
        bool linebreak = false;

        if (formatted_) {
            switch (text[first]) {
                case '\\':
                    if (first + 1 < last) {
                        switch (text[first + 1]) {
                            case 'n': linebreak = true; break;
                            case 'r': linebreak = ax_ > 0; break;
                        }

                        skip++;
                    }

                    skip++;
                    break;
                case '#':
                    if (first + 1 < last) {
                        switch (text[first + 1]) {
                            case 'k': color_ = Color::Name::DARKGREY; break;
                            case 'b': color_ = Color::Name::BLUE; break;
                            case 'r': color_ = Color::Name::RED; break;
                            case 'c': color_ = Color::Name::ORANGE; break;
                        }

                        skip++;
                    }

                    skip++;
                    break;
            }
        }

        int16_t wordwidth = 0;

        if (!linebreak) {
            for (size_t i = first; i < last; i++) {
                auto c = text[i];

                if (c < 32) {
                    if (c == 10) { // 10 = line feed
                        linebreak = true;
                    } else if (c == 13) { // 13 = carriage return
                        linebreak = ax_ > 0;
                    }

                    continue;
                }

                wordwidth += font_.chars[c].ax;

                if (wordwidth > max_width_) {
                    if (last - first == 1) {
                        return last;
                    }

                    prev = add(text, prev, first, i);
                    return add(text, prev, i, last);
                }
            }
        }

        bool newword = skip > 0;
        bool newline = linebreak || ax_ + wordwidth > max_width_;

        if (newword || newline) {
            add_word(prev, first, last_font, last_color);
        }

        if (newline) {
            add_line();

            endy_ = ay_;
            ax_ = 0;
            ay_ += font_.linespace();

            if (!lines_.empty()) {
                ay_ -= line_adj_;
            }
        }

        for (size_t pos = first; pos < last; pos++) {
            auto c = text[pos];
            const Font::Char &ch = font_.chars[c];

            advances_.push_back(ax_);

            if (pos < first + skip || newline && c == ' ') {
                continue;
            }

            if (c < 32) {
                continue;
            }

            ax_ += ch.ax;

            if (width_ < ax_) {
                width_ = ax_;
            }
        }

        if (newword || newline) {
            return first + skip;
        }

        return prev;
    }

    GraphicsGL::LayoutBuilder::LayoutBuilder(const Font &f,
                                             Text::Alignment a,
                                             int16_t mw,
                                             bool fm,
                                             int16_t la) :
            font_(f),
            alignment_(a),
            max_width_(mw),
            formatted_(fm),
            line_adj_(la),
            font_id_(Text::Font::NUM_FONTS),
            color_(Color::Name::NUM_COLORS),
            ax_(0),
            ay_(font_.linespace()),
            width_(0),
            endy_(0) {
        if (max_width_ == 0) {
            max_width_ = 800;
        }
    }

// size_t GraphicsGL::LayoutBuilder::add(const char *text,
//                                       size_t prev,
//                                       size_t first,
//                                       size_t last) {
//     if (first == last) {
//         return prev;
//     }

//     Text::Font last_font = font_id_;
//     Color::Name last_color = color_;
//     size_t skip = 0;
//     bool linebreak = false;

//     if (formatted_) {
//         switch (text[first]) {
//             case '\\':
//                 if (first + 1 < last) {
//                     switch (text[first + 1]) {
//                         case 'n': linebreak = true; break;
//                         case 'r': linebreak = ax_ > 0; break;
//                     }

//                     skip++;
//                 }

//                 skip++;
//                 break;
//             case '#':
//                 if (first + 1 < last) {
//                     switch (text[first + 1]) {
//                         case 'k': color_ = Color::Name::DARKGREY; break;
//                         case 'b': color_ = Color::Name::BLUE; break;
//                         case 'r': color_ = Color::Name::RED; break;
//                         case 'c': color_ = Color::Name::ORANGE; break;
//                     }

//                     skip++;
//                 }

//                 skip++;
//                 break;
//         }
//     }

//     int16_t wordwidth = 0;

//     if (!linebreak) {
//         for (size_t i = first; i < last; i++) {
//             auto c = text[i];
//             wordwidth += font_.chars[c].ax;

//             if (wordwidth > max_width_) {
//                 if (last - first == 1) {
//                     return last;
//                 }

//                 prev = add(text, prev, first, i);
//                 return add(text, prev, i, last);
//             }
//         }
//     }

//     bool newword = skip > 0;
//     bool newline = linebreak || ax_ + wordwidth > max_width_;

//     if (newword || newline) {
//         add_word(prev, first, last_font, last_color);
//     }

//     if (newline) {
//         add_line();

//         endy_ = ay_;
//         ax_ = 0;
//         ay_ += font_.linespace();

//         if (!lines_.empty()) {
//             ay_ -= line_adj_;
//         }
//     }

//     for (size_t pos = first; pos < last; pos++) {
//         auto c = text[pos];
//         const Font::Char &ch = font_.chars[c];

//         advances_.push_back(ax_);

//         if (pos < first + skip || newline && c == ' ') {
//             continue;
//         }

//         ax_ += ch.ax;

//         if (width_ < ax_) {
//             width_ = ax_;
//         }
//     }

//     if (newword || newline) {
//         return first + skip;
//     }

//     return prev;
// }

    Text::Layout GraphicsGL::LayoutBuilder::finish(size_t first, size_t last) {
        add_word(first, last, font_id_, color_);
        add_line();

        advances_.push_back(ax_);

        return Text::Layout(lines_, advances_, width_, ay_, ax_, endy_);
    }

    void GraphicsGL::LayoutBuilder::add_word(size_t word_first,
                                             size_t word_last,
                                             Text::Font word_font,
                                             Color::Name word_color) {
        words_.push_back({ word_first, word_last, word_font, word_color });
    }

    void GraphicsGL::LayoutBuilder::add_line() {
        int16_t line_x = 0;
        int16_t line_y = ay_;

        switch (alignment_) {
            case Text::Alignment::CENTER: line_x -= ax_ / 2; break;
            case Text::Alignment::RIGHT: line_x -= ax_; break;
        }

        lines_.push_back({ words_, { line_x, line_y } });
        words_.clear();
    }

// void GraphicsGL::draw_text(const DrawArgument &args,
//                            const Range<int16_t> &vertical,
//                            const std::string &text,
//                            const Text::Layout &layout,
//                            Text::Font id,
//                            Color::Name colorid,
//                            Text::Background background) {
//     if (locked_) {
//         return;
//     }

//     const Color &color = args.get_color();

//     if (text.empty() || color.invisible()) {
//         return;
//     }

//     auto vec = to_utf8_vector(text);

//     const Font &font = fonts_[id];

//     GLshort x = args.get_pos().x();
//     GLshort y = args.get_pos().y();
//     GLshort w = layout.width();
//     GLshort h = layout.height();
//     GLshort minheight = vertical.first() > 0 ? vertical.first() :
//     SCREEN.top(); GLshort maxheight =
//         vertical.second() > 0 ? vertical.second() : SCREEN.bottom();

//     switch (background) {
//         case Text::Background::NAMETAG:
//             // If ever changing code in here confirm placements with map
//             10000 for (const Text::Layout::Line &line : layout) {
//                 GLshort left = x + line.position.x() - 1;
//                 GLshort right = left + w + 3;
//                 GLshort top = y + line.position.y() - font.linespace() + 6;
//                 GLshort bottom = top + h - 2;
//                 Color ntcolor = Color(0.0f, 0.0f, 0.0f, 0.6f);

//                 quads_.emplace_back(left,
//                                     right,
//                                     top,
//                                     bottom,
//                                     null_offset_,
//                                     ntcolor,
//                                     0.0f);
//                 quads_.emplace_back(left - 1,
//                                     left,
//                                     top + 1,
//                                     bottom - 1,
//                                     null_offset_,
//                                     ntcolor,
//                                     0.0f);
//                 quads_.emplace_back(right,
//                                     right + 1,
//                                     top + 1,
//                                     bottom - 1,
//                                     null_offset_,
//                                     ntcolor,
//                                     0.0f);
//             }

//             break;
//     }

//     for (const Text::Layout::Line &line : layout) {
//         Point<int16_t> position = line.position;

//         for (const Text::Layout::Word &word : line.words) {
//             GLshort ax = position.x() + layout.advance(word.first);
//             GLshort ay = position.y();

//             const GLfloat *wordcolor = nullptr;

//             if (word.color < Color::Name::NUM_COLORS) {
//                 wordcolor = Color::colors[word.color];
//             } else {
//                 wordcolor = Color::colors[colorid];
//             }

//             Color abscolor =
//                 color * Color(wordcolor[0], wordcolor[1],
//                 wordcolor[2], 1.0f);

//             for (size_t pos = word.first; pos < word.last; ++pos) {
//                 auto c = static_cast<unsigned char>(text[pos]);
//                 const Font::Char &ch = font.chars[c];

//                 GLshort char_x = x + ax + ch.bl;
//                 GLshort char_y = y + ay - ch.bt;
//                 GLshort char_width = ch.bw;
//                 GLshort char_height = ch.bh;
//                 GLshort char_bottom = char_y + char_height;

//                 Offset offset = ch.offset;

//                 if (char_bottom > maxheight) {
//                     GLshort bottom_adjust = char_bottom - maxheight;

//                     if (bottom_adjust < 10) {
//                         offset.bottom -= bottom_adjust;
//                         char_bottom -= bottom_adjust;
//                     } else {
//                         continue;
//                     }
//                 }

//                 if (char_y < minheight) {
//                     continue;
//                 }

//                 if (ax == 0 && c == ' ') {
//                     continue;
//                 }

//                 ax += ch.ax;

//                 if (char_width <= 0 || char_height <= 0) {
//                     continue;
//                 }

//                 quads_.emplace_back(char_x,
//                                     char_x + char_width,
//                                     char_y,
//                                     char_bottom,
//                                     offset,
//                                     abscolor,
//                                     0.0f);
//             }
//         }
//     }
// }

    void GraphicsGL::draw_text(const DrawArgument &args,
                               const Range<int16_t> &vertical,
                               const std::vector<uint32_t> &text,
                               const Text::Layout &layout,
                               Text::Font id,
                               Color::Name colorid,
                               Text::Background background) {
        if (locked_) {
            return;
        }

        const Color &color = args.get_color();

        if (text.empty() || color.invisible()) {
            return;
        }

        const Font &font = fonts_[id];

        GLshort x = args.get_pos().x();
        GLshort y = args.get_pos().y();
        GLshort w = layout.width();
        GLshort h = layout.height();
        GLshort minheight = vertical.first() > 0 ? vertical.first() : SCREEN.top();
        GLshort maxheight =
                vertical.second() > 0 ? vertical.second() : SCREEN.bottom();

        switch (background) {
            case Text::Background::NAMETAG:
                // If ever changing code in here confirm placements with map 10000
                for (const Text::Layout::Line &line : layout) {
                    GLshort left = x + line.position.x() - 1;
                    GLshort right = left + w + 3;
                    GLshort top = y + line.position.y() - font.linespace() + 6;
                    GLshort bottom = top + h - 2;
                    Color ntcolor = Color(0.0f, 0.0f, 0.0f, 0.6f);

                    quads_.emplace_back(left,
                                        right,
                                        top,
                                        bottom,
                                        null_offset_,
                                        ntcolor,
                                        0.0f);
                    quads_.emplace_back(left - 1,
                                        left,
                                        top + 1,
                                        bottom - 1,
                                        null_offset_,
                                        ntcolor,
                                        0.0f);
                    quads_.emplace_back(right,
                                        right + 1,
                                        top + 1,
                                        bottom - 1,
                                        null_offset_,
                                        ntcolor,
                                        0.0f);
                }

                break;
        }

        for (const Text::Layout::Line &line : layout) {
            Point<int16_t> position = line.position;

            for (const Text::Layout::Word &word : line.words) {
                GLshort ax = position.x() + layout.advance(word.first);
                GLshort ay = position.y();

                const GLfloat *wordcolor = nullptr;

                if (word.color < Color::Name::NUM_COLORS) {
                    wordcolor = Color::colors[word.color];
                } else {
                    wordcolor = Color::colors[colorid];
                }

                Color abscolor =
                        color * Color(wordcolor[0], wordcolor[1], wordcolor[2], 1.0f);

                for (size_t pos = word.first; pos < word.last; ++pos) {
                    auto c = text[pos];
                    const Font::Char &ch = font.chars[c];

                    GLshort char_x = x + ax + ch.bl;
                    GLshort char_y = y + ay - ch.bt;
                    GLshort char_width = ch.bw;
                    GLshort char_height = ch.bh;
                    GLshort char_bottom = char_y + char_height;

                    Offset offset = ch.offset;

                    if (char_bottom > maxheight) {
                        GLshort bottom_adjust = char_bottom - maxheight;

                        if (bottom_adjust < 10) {
                            offset.bottom -= bottom_adjust;
                            char_bottom -= bottom_adjust;
                        } else {
                            continue;
                        }
                    }

                    if (char_y < minheight) {
                        continue;
                    }

                    if (ax == 0 && c == ' ') {
                        continue;
                    }

                    ax += ch.ax;

                    if (char_width <= 0 || char_height <= 0) {
                        continue;
                    }

                    quads_.emplace_back(char_x,
                                        char_x + char_width,
                                        char_y,
                                        char_bottom,
                                        offset,
                                        abscolor,
                                        0.0f);
                }
            }
        }
    }

    void GraphicsGL::draw_rectangle(int16_t x,
                                    int16_t y,
                                    int16_t width,
                                    int16_t height,
                                    float red,
                                    float green,
                                    float blue,
                                    float alpha) {
        if (locked_) {
            return;
        }

        quads_.emplace_back(x,
                            x + width,
                            y,
                            y + height,
                            null_offset_,
                            Color(red, green, blue, alpha),
                            0.0f);
    }

    void GraphicsGL::draw_screen_fill(float red,
                                      float green,
                                      float blue,
                                      float alpha) {
        draw_rectangle(0, 0, VWIDTH, VHEIGHT, red, green, blue, alpha);
    }

    void GraphicsGL::lock() {
        locked_ = true;
    }

    void GraphicsGL::unlock() {
        locked_ = false;
    }

    void GraphicsGL::flush(float opacity) {
        bool coverscene = opacity != 1.0f;

        if (coverscene) {
            float complement = 1.0f - opacity;
            Color color = Color(0.0f, 0.0f, 0.0f, complement);

            quads_.emplace_back(SCREEN.left(),
                                SCREEN.right(),
                                SCREEN.top(),
                                SCREEN.bottom(),
                                null_offset_,
                                color,
                                0.0f);
        }

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

//        GLsizeiptr csize = quads_.size() * sizeof(Quad);
//        GLsizeiptr fsize = quads_.size() * Quad::LENGTH;

        std::vector<Quad::Vertex> triangles;
        for (const Quad& quad : quads_) {
            triangles.push_back(quad.vertices[0]);
            triangles.push_back(quad.vertices[1]);
            triangles.push_back(quad.vertices[2]);

            triangles.push_back(quad.vertices[0]);
            triangles.push_back(quad.vertices[2]);
            triangles.push_back(quad.vertices[3]);
        }

        glEnableVertexAttribArray(attribute_coord_);
        glEnableVertexAttribArray(attribute_color_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferData(GL_ARRAY_BUFFER, triangles.size() * sizeof(Quad::Vertex), triangles.data(), GL_STREAM_DRAW);

        glDrawArrays(GL_TRIANGLES, 0, triangles.size());

        glDisableVertexAttribArray(attribute_coord_);
        glDisableVertexAttribArray(attribute_color_);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if (coverscene) {
            quads_.pop_back();
        }
    }

    void GraphicsGL::clear_scene() {
        if (!locked_) {
            quads_.clear();
        }
    }
}  // namespace ms