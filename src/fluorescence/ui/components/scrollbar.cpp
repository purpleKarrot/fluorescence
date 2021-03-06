/*
 * fluorescence is a free, customizable Ultima Online client.
 * Copyright (C) 2011-2012, http://fluorescence-client.org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This file is based on (and partly copied from ClanLib's CL_ScrollBar component.
 *
**  ClanLib SDK
**  Copyright (c) 1997-2011 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Harry Storbacka
**    Magnus Norddahl
**    Kenneth Gangstoe
*/

#include "scrollbar.hpp"

#include <ClanLib/Core/Math/cl_math.h>
#include <ClanLib/Core/Math/quad.h>
#include <ClanLib/Display/Window/keys.h>
#include <ClanLib/GUI/gui_message_pointer.h>
#include <ClanLib/Display/2D/draw.h>
#include <ClanLib/Display/Render/texture.h>

#include <data/manager.hpp>
#include <data/huesloader.hpp>
#include <misc/log.hpp>
#include <ui/manager.hpp>
#include <ui/render/shadermanager.hpp>

namespace fluo {
namespace ui {
namespace components {

ScrollBar::ScrollBar(CL_GUIComponent* parent) : GumpComponent(parent),
    incrementIndex_(0), decrementIndex_(0), thumbIndex_(0), shaderInitialized_(false),
    vertical(false), scroll_min(0), scroll_max(1), line_step(1),
    page_step(10), position(0), mouse_down_mode(mouse_down_none),
    thumb_start_position(0), thumb_start_pixel_position(0) {

    set_type_name("scrollbar");

    func_process_message().set(this, &ScrollBar::on_process_message);
    func_render().set(this, &ScrollBar::on_render);

    mouse_down_timer.func_expired().set(this, &ScrollBar::on_timer_expired);

    for (unsigned int i = 0; i < 3; ++i) {
        incrementImageStates_[i].setOwner(this);
        decrementImageStates_[i].setOwner(this);
        thumbImageStates_[i].setOwner(this);
    }
    trackImageState_.setOwner(this);

    // set size to make sure it is not skipped at rendering
    setWidth(1);
    setHeight(1);
}

ScrollBar::~ScrollBar() {
}

void ScrollBar::on_render(CL_GraphicContext &gc, const CL_Rect &update_rect) {
    boost::shared_ptr<ui::Texture> incTex = getPartTexture(incrementImageStates_, incrementIndex_);
    boost::shared_ptr<ui::Texture> decTex = getPartTexture(decrementImageStates_, decrementIndex_);
    boost::shared_ptr<ui::Texture> thumbTex = getPartTexture(thumbImageStates_, thumbIndex_);
    boost::shared_ptr<ui::Texture> trackTex = trackImageState_.getTexture();
    if (!incTex || !incTex->isReadComplete() ||
            !decTex || !decTex->isReadComplete() ||
            !thumbTex || !thumbTex->isReadComplete() ||
            !trackTex || !trackTex->isReadComplete()) {
        ui::Manager::getSingleton()->queueComponentRepaint(this);
        return;
    }

    // if the increment button is at the top left, the part positions were not set yet
    // => textures are fully loaded for the first time
    if (rect_button_increment.top == 0 && rect_button_increment.left == 0) {
        update_part_positions();
    }

    CL_Rectf centeredTrack(
            (get_width() - trackTex->getWidth()) / 2, rect_track_decrement.top,
            CL_Sizef(trackTex->getWidth(), trackTex->getHeight())
    );
    CL_Rectf trackTexCoords = trackTex->getNormalizedTextureCoords();
    if (vertical) {
        while (centeredTrack.bottom <= rect_button_increment.top) {
            renderTexture(gc, &trackImageState_, 0, centeredTrack);
            centeredTrack.translate(0, trackTex->getHeight() - 1);
        }

        // draw rest
        int heightLeft = rect_button_increment.top - centeredTrack.top;
        float factor = heightLeft / centeredTrack.get_height();
        centeredTrack.bottom = centeredTrack.top + (centeredTrack.get_height() * factor);
        trackTexCoords.bottom = trackTexCoords.top + (trackTexCoords.get_height() * factor);
        renderTexture(gc, &trackImageState_, 0, centeredTrack);
    } else {
        while (centeredTrack.right <= rect_button_increment.left) {
            renderTexture(gc, &trackImageState_, 0, centeredTrack);
            centeredTrack.translate(trackTex->getWidth() - 1, 0);
        }

        // draw rest
        int widthLeft = rect_button_increment.left - centeredTrack.left;
        float factor = widthLeft / centeredTrack.get_width();
        centeredTrack.left = centeredTrack.right + (centeredTrack.get_width() * factor);
        trackTexCoords.left = trackTexCoords.right + (trackTexCoords.get_width() * factor);
        centeredTrack.translate(trackTex->getWidth() - 1, 0);
    }

    CL_Rectf centeredDec(
            (get_width() - decTex->getWidth()) / 2, rect_button_decrement.top,
            CL_Sizef(decTex->getWidth(), decTex->getHeight())
    );
    renderTexture(gc, decrementImageStates_, decrementIndex_, centeredDec);

    CL_Rectf centeredInc(
            (get_width() - incTex->getWidth()) / 2, rect_button_increment.top,
            CL_Sizef(incTex->getWidth(), incTex->getHeight())
    );
    renderTexture(gc, incrementImageStates_, incrementIndex_, centeredInc);

    CL_Rectf centeredThumb(
            (get_width() - thumbTex->getWidth()) / 2, rect_thumb.top,
            CL_Sizef(thumbTex->getWidth(), thumbTex->getHeight())
    );
    renderTexture(gc, thumbImageStates_, thumbIndex_, centeredThumb);

    // clean up renderer if necessary
    if (shaderInitialized_) {
        gc.reset_textures();
        gc.reset_program_object();
        shaderInitialized_ = false;
    }
}

void ScrollBar::renderTexture(CL_GraphicContext& gc, ImageState* states, unsigned int currentState, const CL_Rect& vertexRect) {
    unsigned int hue = getPartHue(states, currentState);
    boost::shared_ptr<ui::Texture> tex = getPartTexture(states, currentState);
    if (hue == 0) {
        // no need for the shader
        CL_Draw::texture(gc, tex->getTexture(), CL_Quadf(vertexRect), getPartRgba(states, currentState), tex->getNormalizedTextureCoords());
        return;
    }

    if (!shaderInitialized_) {
        boost::shared_ptr<CL_ProgramObject> shader = ui::Manager::getShaderManager()->getGumpShader();
        gc.set_program_object(*shader, cl_program_matrix_modelview_projection);

        CL_Texture huesTexture = data::Manager::getHuesLoader()->getHuesTexture();
        gc.set_texture(0, huesTexture);
        // set texture unit 1 active to avoid overriding the hue texture with newly loaded object textures
        gc.set_texture(1, huesTexture);

        shader->set_uniform1i("HueTexture", 0);
        shader->set_uniform1i("ObjectTexture", 1);

        shaderInitialized_ = true;
    }

    CL_Rectf textureRect = tex->getNormalizedTextureCoords();
    gc.set_texture(1, tex->getTexture());

    CL_Vec2f tex1_coords[6] = {
        CL_Vec2f(textureRect.left, textureRect.top),
        CL_Vec2f(textureRect.right, textureRect.top),
        CL_Vec2f(textureRect.left, textureRect.bottom),
        CL_Vec2f(textureRect.right, textureRect.top),
        CL_Vec2f(textureRect.left, textureRect.bottom),
        CL_Vec2f(textureRect.right, textureRect.bottom)
    };

    CL_Vec2f vertexCoords[6];
    vertexCoords[0] = CL_Vec2f(vertexRect.left, vertexRect.top);
    vertexCoords[1] = CL_Vec2f(vertexRect.right, vertexRect.top);
    vertexCoords[2] = CL_Vec2f(vertexRect.left, vertexRect.bottom);
    vertexCoords[3] = CL_Vec2f(vertexRect.right, vertexRect.top);
    vertexCoords[4] = CL_Vec2f(vertexRect.left, vertexRect.bottom);
    vertexCoords[5] = CL_Vec2f(vertexRect.right, vertexRect.bottom);

    CL_PrimitivesArray primarray(gc);
    primarray.set_attributes(0, vertexCoords);
    primarray.set_attributes(1, tex1_coords);

    CL_Vec3f hueVec = CL_Vec3f((getPartPartialHue(states, currentState) ? 1 : 0), hue, getPartAlpha(states, currentState));
    primarray.set_attribute(2, hueVec);

    gc.draw_primitives(cl_triangles, 6, primarray);
}

boost::shared_ptr<ui::Texture> ScrollBar::getPartTexture(ImageState* states, unsigned int currentState) {
    if (currentState != 0 && states[currentState].overrideTexture_) {
        return states[currentState].getTexture();
    } else {
        return states[0].getTexture();
    }
}

unsigned int ScrollBar::getPartHue(ImageState* states, unsigned int currentState) {
    if (currentState != 0 && states[currentState].overrideHue_) {
        return states[currentState].getHue();
    } else {
        return states[0].getHue();
    }
}

CL_Colorf ScrollBar::getPartRgba(ImageState* states, unsigned int currentState) {
    if (currentState != 0 && states[currentState].overrideRgba_) {
        return states[currentState].getRgba();
    } else {
        return states[0].getRgba();
    }
}

float ScrollBar::getPartAlpha(ImageState* states, unsigned int currentState) {
    if (currentState != 0 && states[currentState].overrideAlpha_) {
        return states[currentState].getAlpha();
    } else {
        return states[0].getAlpha();
    }
}

bool ScrollBar::getPartPartialHue(ImageState* states, unsigned int currentState) {
    if (currentState != 0 && states[currentState].overridePartialHue_) {
        return states[currentState].getPartialHue();
    } else {
        return states[0].getPartialHue();
    }
}

ImageState* ScrollBar::getIncrementNormal()  {
    return &incrementImageStates_[STATE_INDEX_UP];
}

ImageState* ScrollBar::getIncrementMouseOver()  {
    return &incrementImageStates_[STATE_INDEX_MOUSEOVER];
}

ImageState* ScrollBar::getIncrementMouseDown()  {
    return &incrementImageStates_[STATE_INDEX_DOWN];
}

ImageState* ScrollBar::getDecrementNormal()  {
    return &decrementImageStates_[STATE_INDEX_UP];
}

ImageState* ScrollBar::getDecrementMouseOver()  {
    return &decrementImageStates_[STATE_INDEX_MOUSEOVER];
}

ImageState* ScrollBar::getDecrementMouseDown()  {
    return &decrementImageStates_[STATE_INDEX_DOWN];
}

ImageState* ScrollBar::getThumbNormal()  {
    return &thumbImageStates_[STATE_INDEX_UP];
}

ImageState* ScrollBar::getThumbMouseOver() {
    return &thumbImageStates_[STATE_INDEX_MOUSEOVER];
}

ImageState* ScrollBar::getThumbMouseDown()  {
    return &thumbImageStates_[STATE_INDEX_DOWN];
}

ImageState* ScrollBar::getTrack()  {
    return &trackImageState_;
}

// clanlib stuff

bool ScrollBar::is_vertical() const {
    return vertical;
}

bool ScrollBar::is_horizontal() const {
    return !vertical;
}

int ScrollBar::get_min() const {
    return scroll_min;
}

int ScrollBar::get_max() const {
    return scroll_max;
}

int ScrollBar::get_line_step() const {
    return line_step;
}

int ScrollBar::get_page_step() const {
    return page_step;
}

int ScrollBar::get_position() const {
    return position;
}

void ScrollBar::set_vertical() {
    vertical = true;
    if(update_part_positions())
        request_repaint();
}

void ScrollBar::set_horizontal() {
    vertical = false;
    if(update_part_positions())
        request_repaint();
}

void ScrollBar::set_min(int new_scroll_min) {
    set_ranges(new_scroll_min, scroll_max, line_step, page_step);
}

void ScrollBar::set_max(int new_scroll_max) {
    set_ranges(scroll_min, new_scroll_max, line_step, page_step);
}

void ScrollBar::set_line_step(int step) {
    set_ranges(scroll_min, scroll_max, step, page_step);
}

void ScrollBar::set_page_step(int step) {
    set_ranges(scroll_min, scroll_max, line_step, step);
}

void ScrollBar::set_ranges(int min, int max, int lstep, int pstep) {
    if (scroll_min >= scroll_max || line_step <= 0 || page_step <= 0)
        throw CL_Exception("Scrollbar ranges out of bounds!");
    scroll_min = min;
    scroll_max = max;
    line_step = lstep;
    page_step = pstep;
    if (position >= scroll_max)
        position = scroll_max-1;
    if (position < scroll_min)
        position = scroll_min;
    if(update_part_positions())
        request_repaint();
}

void ScrollBar::calculate_ranges(int view_size, int total_size) {
    if (view_size <= 0 || total_size <= 0) {
        set_ranges(0, 1, 1, 1);
    } else {
        int scroll_max = cl_max(1, total_size - view_size + 1);
        int page_step = cl_max(1, view_size);
        set_ranges(0, scroll_max, page_step / 10, page_step);
    }
}

void ScrollBar::set_position(int pos) {
    position = pos;
    if (pos >= scroll_max)
        position = scroll_max-1;
    if (pos < scroll_min)
        position = scroll_min;

    if(update_part_positions())
        request_repaint();
}

void ScrollBar::on_process_message(CL_GUIMessage &msg) {
    if (!is_enabled())
        return;

    if (msg.is_type(CL_GUIMessage_Input::get_type_name())) {
        CL_GUIMessage_Input input = msg;
        CL_InputEvent input_event = input.get_event();
        if (input_event.type == CL_InputEvent::pointer_moved)
            on_mouse_move(input, input_event);
        else if (input_event.type == CL_InputEvent::pressed && input_event.id == CL_MOUSE_LEFT)
            on_mouse_lbutton_down(input, input_event);
        else if (input_event.type == CL_InputEvent::released && input_event.id == CL_MOUSE_LEFT)
            on_mouse_lbutton_up(input, input_event);
    } else if (msg.is_type(CL_GUIMessage_Pointer::get_type_name())) {
        CL_GUIMessage_Pointer pointer = msg;
        if (pointer.get_pointer_type() == CL_GUIMessage_Pointer::pointer_leave)
            on_mouse_leave();
    }
}

void ScrollBar::on_mouse_move(CL_GUIMessage_Input &input, CL_InputEvent &input_event) {
    CL_Point pos = input_event.mouse_pos;

    updateTextureStates(pos);

    if (mouse_down_mode == mouse_down_thumb_drag) {
        CL_Rect geom = get_window_geometry();

        int last_position = position;

        if (pos.x < -100 || pos.x > geom.get_width() + 100 || pos.y < -100 || pos.y > geom.get_height() + 100) {
            position = thumb_start_position;
        } else {
            int delta = vertical ? (pos.y - mouse_drag_start_pos.y) : (pos.x - mouse_drag_start_pos.x);
            int position_pixels = thumb_start_pixel_position + delta;

            int track_height = 0;
            if (vertical)
                track_height = rect_track_decrement.get_height()+rect_track_increment.get_height();
            else
                track_height = rect_track_decrement.get_width()+rect_track_increment.get_width();

            if (track_height != 0)
                position = scroll_min + position_pixels*(scroll_max-scroll_min)/track_height;
            else
                position = 0;

            if (position >= scroll_max)
                position = scroll_max-1;
            if (position < scroll_min)
                position = scroll_min;

        }

        if (position != last_position) {
            invoke_scroll_event();

            update_part_positions();
        }
    }

    request_repaint();

    input.set_consumed();
}

void ScrollBar::on_mouse_lbutton_down(CL_GUIMessage_Input &input, CL_InputEvent &input_event) {
    CL_Point pos = input_event.mouse_pos;
    mouse_drag_start_pos = pos;

    if (rect_button_decrement.contains(pos)) {
        mouse_down_mode = mouse_down_button_decr;

        int last_position = position;

        position -= line_step;
        last_step_size = -line_step;
        if (position >= scroll_max)
            position = scroll_max-1;
        if (position < scroll_min)
            position = scroll_min;

        if (last_position != position)
            invoke_scroll_event();
    } else if (rect_button_increment.contains(pos)) {
        mouse_down_mode = mouse_down_button_incr;

        int last_position = position;

        position += line_step;
        last_step_size = line_step;
        if (position >= scroll_max)
            position = scroll_max-1;
        if (position < scroll_min)
            position = scroll_min;

        if (last_position != position)
            invoke_scroll_event();
    } else if (rect_thumb.contains(pos)) {
        mouse_down_mode = mouse_down_thumb_drag;
        thumb_start_position = position;
        thumb_start_pixel_position = vertical ? (rect_thumb.top-rect_track_decrement.top) : (rect_thumb.left-rect_track_decrement.left);
    } else if (rect_track_decrement.contains(pos)) {
        mouse_down_mode = mouse_down_track_decr;

        int last_position = position;

        position -= page_step;
        last_step_size = -page_step;
        if (position >= scroll_max)
            position = scroll_max-1;
        if (position < scroll_min)
            position = scroll_min;

        if (last_position != position)
            invoke_scroll_event();
    } else if (rect_track_increment.contains(pos)) {
        mouse_down_mode = mouse_down_track_incr;

        int last_position = position;

        position += page_step;
        last_step_size = page_step;
        if (position >= scroll_max)
            position = scroll_max-1;
        if (position < scroll_min)
            position = scroll_min;

        if (last_position != position)
            invoke_scroll_event();
    }

    mouse_down_timer.start(100,false);

    update_part_positions();
    updateTextureStates(pos);

    request_repaint();
    capture_mouse(true);
    input.set_consumed();
}

void ScrollBar::on_mouse_lbutton_up(CL_GUIMessage_Input &input, CL_InputEvent &input_event) {
    mouse_down_mode = mouse_down_none;
    mouse_down_timer.stop();

    updateTextureStates(input_event.mouse_pos);

    request_repaint();
    capture_mouse(false);
    input.set_consumed();
}

void ScrollBar::on_mouse_leave() {
    updateTextureStates(CL_Point(-1, -1));
    request_repaint();
}

// Calculates positions of all parts. Returns true if thumb position was changed compared to previously, false otherwise.
bool ScrollBar::update_part_positions() {
    boost::shared_ptr<ui::Texture> incTex = getPartTexture(incrementImageStates_, incrementIndex_);
    boost::shared_ptr<ui::Texture> decTex = getPartTexture(decrementImageStates_, decrementIndex_);
    boost::shared_ptr<ui::Texture> thumbTex = getPartTexture(thumbImageStates_, thumbIndex_);
    boost::shared_ptr<ui::Texture> trackTex = trackImageState_.getTexture();
    // textures not set yet, or not fully loaded (size information might not be available)
    if (!incTex || !incTex->isReadComplete() ||
            !decTex || !decTex->isReadComplete() ||
            !thumbTex || !thumbTex->isReadComplete() ||
            !trackTex || !trackTex->isReadComplete()) {
        ui::Manager::getSingleton()->queueComponentRepaint(this);
        return false;
    }

    CL_Rect rect(CL_Point(0,0), get_geometry().get_size());

    int decr_height = decTex->getHeight();
    int incr_height = incTex->getHeight();
    int total_height = vertical ? rect.get_height() : rect.get_width();
    int track_height = cl_max(0, total_height - decr_height - incr_height);
    int thumb_height = thumbTex->getHeight(); //calculate_thumb_size(track_height);

    int thumb_offset = decr_height + calculate_thumb_position(thumb_height, track_height);

    CL_Rect previous_rect_thumb = rect_thumb;

    rect_button_decrement = create_rect(rect, 0, decr_height);
    rect_track_decrement = create_rect(rect, decr_height, thumb_offset);
    rect_thumb = create_rect(rect, thumb_offset, thumb_offset+thumb_height);
    rect_track_increment = create_rect(rect, thumb_offset+thumb_height, decr_height+track_height);
    rect_button_increment = create_rect(rect, decr_height+track_height, decr_height+track_height+incr_height);

    return (previous_rect_thumb != rect_thumb);
}

int ScrollBar::calculate_thumb_size(int track_size) {
    int minimum_thumb_size = 20;
    int range = scroll_max-scroll_min;
    int length = range+page_step-1;
    int thumb_size = page_step*track_size/length;
    if (thumb_size < minimum_thumb_size)
        thumb_size = minimum_thumb_size;
    if (thumb_size > track_size)
        thumb_size = track_size;
    return thumb_size;
}

int ScrollBar::calculate_thumb_position(int thumb_size, int track_size) {
    int relative_pos = position-scroll_min;
    int range = scroll_max-scroll_min-1;
    if (range != 0) {
        int available_area = cl_max(0, track_size-thumb_size);
        return relative_pos*available_area/range;
    } else {
        return 0;
    }
}

CL_Rect ScrollBar::create_rect(const CL_Rect content_rect, int start, int end) {
    if (vertical)
        return CL_Rect(content_rect.left, content_rect.top+start, content_rect.right, content_rect.top+end);
    else
        return CL_Rect(content_rect.left+start, content_rect.top, content_rect.left+end, content_rect.bottom);
}

void ScrollBar::on_timer_expired() {
    if (mouse_down_mode == mouse_down_thumb_drag)
        return;

    mouse_down_timer.start(100, false);

    int last_position = position;
    position += last_step_size;
    if (position >= scroll_max)
        position = scroll_max-1;

    if (position < scroll_min)
        position = scroll_min;

    if (position != last_position) {
        invoke_scroll_event();

        if(update_part_positions())
            request_repaint();
    }
}

void ScrollBar::invoke_scroll_event() {
    if (!func_scroll_.is_null())
        func_scroll_.invoke();
}

void ScrollBar::updateTextureStates(const CL_Point& pos) {
    if (pos.x == -1 && pos.y == -1) {
        incrementIndex_ = STATE_INDEX_UP;
        decrementIndex_ = STATE_INDEX_UP;
        thumbIndex_ = STATE_INDEX_UP;
    } else {
        thumbIndex_ = rect_thumb.contains(pos) ? (mouse_down_mode == mouse_down_none ? STATE_INDEX_MOUSEOVER : STATE_INDEX_DOWN) : STATE_INDEX_UP;
        incrementIndex_ = rect_button_increment.contains(pos) ? (mouse_down_mode == mouse_down_none ? STATE_INDEX_MOUSEOVER : STATE_INDEX_DOWN) : STATE_INDEX_UP;
        decrementIndex_ = rect_button_decrement.contains(pos) ? (mouse_down_mode == mouse_down_none ? STATE_INDEX_MOUSEOVER : STATE_INDEX_DOWN) : STATE_INDEX_UP;
    }
}

CL_Callback_v0& ScrollBar::func_scroll() {
    return func_scroll_;
}

}
}
}
