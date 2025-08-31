/*
Copyright (C) 2025 NoneBot

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <https://www.gnu.org/licenses/>.
*/

#include "cairo_wrapper.h"

#include <cmath>

using namespace cairo_wrapper;

void border::cairo_add_path_arc(cairo_t* cr, double x, double y, double rx, double ry, double a1, double a2, bool neg) {
    if (rx > 0 && ry > 0) {
        cairo_save(cr);

        cairo_translate(cr, x, y);
        cairo_scale(cr, 1.0, ry / rx);
        cairo_translate(cr, -x, -y);

        if (neg) {
            cairo_arc_negative(cr, x, y, rx, a1, a2);
        }
        else {
            cairo_arc(cr, x, y, rx, a1, a2);
        }

        cairo_restore(cr);
    }
    else {
        cairo_move_to(cr, x, y);
    }
}

void border::cairo_set_color(cairo_t* cr, const litehtml::web_color& color) {
    cairo_set_source_rgba(cr, color.red / 255.0, color.green / 255.0, color.blue / 255.0, color.alpha / 255.0);
}

void border::draw_border() {
    cairo_save(cr);

    if (radius_top_x && radius_top_y) {
        double start_angle = M_PI;
        double end_angle = start_angle + M_PI / 2.0 / (top_border_width / (double)border_width + 1);

        cairo_add_path_arc(cr,
                           left + radius_top_x,
                           top + radius_top_y,
                           radius_top_x - border_width,
                           radius_top_y - border_width + (border_width - top_border_width),
                           start_angle,
                           end_angle, false);

        cairo_add_path_arc(cr,
                           left + radius_top_x,
                           top + radius_top_y,
                           radius_top_x,
                           radius_top_y,
                           end_angle,
                           start_angle, true);
    }
    else {
        cairo_move_to(cr, left + border_width, top + top_border_width);
        cairo_line_to(cr, left, top);
    }

    if (radius_bottom_x && radius_bottom_y) {
        cairo_line_to(cr, left, bottom - radius_bottom_y);

        double end_angle = M_PI;
        double start_angle = end_angle - M_PI / 2.0 / ((double)bottom_border_width / (double)border_width + 1);

        cairo_add_path_arc(cr,
                           left + radius_bottom_x,
                           bottom - radius_bottom_y,
                           radius_bottom_x,
                           radius_bottom_y,
                           end_angle,
                           start_angle, true);

        cairo_add_path_arc(cr,
                           left + radius_bottom_x,
                           bottom - radius_bottom_y,
                           radius_bottom_x - border_width,
                           radius_bottom_y - border_width + (border_width - bottom_border_width),
                           start_angle,
                           end_angle, false);
    }
    else {
        cairo_line_to(cr, left, bottom);
        cairo_line_to(cr, left + border_width, bottom - bottom_border_width);
    }
    cairo_clip(cr);

    switch (style) {
    case litehtml::border_style_dotted:
        draw_dotted();
        break;
    case litehtml::border_style_dashed:
        draw_dashed();
        break;
    case litehtml::border_style_double:
        draw_double();
        break;
    case litehtml::border_style_inset:
        draw_inset_outset(true);
        break;
    case litehtml::border_style_outset:
        draw_inset_outset(false);
        break;
    case litehtml::border_style_groove:
        draw_groove_ridge(true);
        break;
    case litehtml::border_style_ridge:
        draw_groove_ridge(false);
        break;
    default:
        draw_solid();
        break;
    }

    cairo_restore(cr);
}

void border::draw_line(double line_offset, double top_line_offset, double bottom_line_offset) {
    if (radius_top_x && radius_top_y) {
        double end_angle = M_PI;
        double start_angle = end_angle + M_PI / 2.0 / ((double)top_border_width / (double)border_width + 1);

        cairo_add_path_arc(cr,
                           left + radius_top_x,
                           top + radius_top_y,
                           radius_top_x - line_offset,
                           radius_top_y - line_offset + (line_offset - top_line_offset),
                           start_angle,
                           end_angle, true);
    }
    else {
        cairo_move_to(cr, left + line_offset, top);
    }

    if (radius_bottom_x && radius_bottom_y) {
        cairo_line_to(cr, left + line_offset, bottom - radius_bottom_y);

        double start_angle = M_PI;
        double end_angle = start_angle - M_PI / 2.0 / ((double)bottom_border_width / (double)border_width + 1);

        cairo_add_path_arc(cr,
                           left + radius_bottom_x,
                           bottom - radius_bottom_y,
                           radius_bottom_x - line_offset,
                           radius_bottom_y - line_offset + (line_offset - bottom_line_offset),
                           start_angle,
                           end_angle, true);
    }
    else {
        cairo_line_to(cr, left + line_offset, bottom);
    }
}

void border::draw_inset_outset(bool is_inset) {
    litehtml::web_color line_color;
    litehtml::web_color light_color = color;
    litehtml::web_color dark_color = color.darken(0.33);
    if (color.red == 0 && color.green == 0 && color.blue == 0) {
        dark_color.red = dark_color.green = dark_color.blue = 0x4C;
        light_color.red = light_color.green = light_color.blue = 0xB2;
    }

    if (side == left_side || side == top_side) {
        line_color = is_inset ? dark_color : light_color;
    }
    else {
        line_color = is_inset ? light_color : dark_color;
    }
    draw_line(border_width / 2.0,
              top_border_width / 2.0,
              bottom_border_width / 2.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_dash(cr, nullptr, 0, 0);
    cairo_set_color(cr, line_color);
    cairo_set_line_width(cr, border_width);
    cairo_stroke(cr);
}

void border::draw_double() {
    if (border_width < 3) {
        draw_solid();
    }
    else {
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
        cairo_set_dash(cr, nullptr, 0, 0);
        cairo_set_color(cr, color);

        double line_width = border_width / 3.0;
        cairo_set_line_width(cr, line_width);
        // draw external line
        draw_line(line_width / 2.0,
                  top_border_width / 6.0,
                  bottom_border_width / 6.0);
        cairo_stroke(cr);
        // draw internal line
        draw_line(border_width - line_width / 2.0,
                  top_border_width - top_border_width / 6.0,
                  bottom_border_width - bottom_border_width / 6.0);
        cairo_stroke(cr);
    }
}

void border::draw_dashed() {
    int line_length = std::abs(bottom - top);
    if (!line_length) return;

    draw_line(border_width / 2.0,
              top_border_width / 2.0,
              bottom_border_width / 2.0);

    int segment_length = border_width * 3;
    int seg_nums = line_length / segment_length;
    if (seg_nums < 2) {
        seg_nums = 2;
    }
    if (seg_nums % 2 != 0) {
        seg_nums = seg_nums + 1;
    }
    seg_nums++;
    double seg_len = (double)line_length / (double)seg_nums;

    double dashes[2];
    dashes[0] = seg_len;
    dashes[1] = seg_len;
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_dash(cr, dashes, 2, 0);
    cairo_set_color(cr, color);
    cairo_set_line_width(cr, border_width);
    cairo_stroke(cr);
}

void border::draw_solid() {
    draw_line(border_width / 2.0,
              top_border_width / 2.0,
              bottom_border_width / 2.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_dash(cr, nullptr, 0, 0);
    cairo_set_color(cr, color);
    cairo_set_line_width(cr, border_width);
    cairo_stroke(cr);
}

void border::draw_dotted() {
    // Zero length line
    if (bottom == top) return;

    draw_line(border_width / 2.0,
              top_border_width / 2.0,
              bottom_border_width / 2.0);

    double line_length = std::abs(bottom - top);

    double dot_size = border_width;
    int num_dots = (int)std::nearbyint(line_length / (dot_size * 2.0));
    if (num_dots < 2) {
        num_dots = 2;
    }
    if (num_dots % 2 != 0) {
        num_dots = num_dots + 1;
    }
    num_dots++;
    double space_len = ((double)line_length - (double)border_width) / (num_dots - 1.0);

    double dashes[2];
    dashes[0] = 0;
    dashes[1] = space_len;
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_dash(cr, dashes, 2, -dot_size / 2.0);

    cairo_set_color(cr, color);
    cairo_set_line_width(cr, border_width);
    cairo_stroke(cr);
}

void border::draw_groove_ridge(bool is_groove) {
    if (border_width == 1) {
        draw_solid();
    }
    else {
        litehtml::web_color inner_line_color;
        litehtml::web_color outer_line_color;
        litehtml::web_color light_color = color;
        litehtml::web_color dark_color = color.darken(0.33);
        if (color.red == 0 && color.green == 0 && color.blue == 0) {
            dark_color.red = dark_color.green = dark_color.blue = 0x4C;
            light_color.red = light_color.green = light_color.blue = 0xB2;
        }

        if (side == left_side || side == top_side) {
            outer_line_color = is_groove ? dark_color : light_color;
            inner_line_color = is_groove ? light_color : dark_color;
        }
        else {
            outer_line_color = is_groove ? light_color : dark_color;
            inner_line_color = is_groove ? dark_color : light_color;
        }

        cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
        cairo_set_dash(cr, nullptr, 0, 0);

        double line_width = border_width / 2.0;
        cairo_set_line_width(cr, line_width);
        // draw external line
        draw_line(line_width / 2.0,
                  top_border_width / 4.0,
                  bottom_border_width / 4.0);
        cairo_set_color(cr, outer_line_color);
        cairo_stroke(cr);
        // draw internal line
        cairo_set_color(cr, inner_line_color);
        draw_line(border_width - line_width / 2.0,
                  top_border_width - top_border_width / 4.0,
                  bottom_border_width - bottom_border_width / 4.0);
        cairo_stroke(cr);
    }
}

void conic_gradient::sector_patch(cairo_pattern_t* pat, const double radius, const double angle_A,
                                  const litehtml::web_color& A,
                                  const double angle_B, const litehtml::web_color& B) {
    const double A_r = A.red / 255.0, A_g = A.green / 255.0, A_b = A.blue / 255.0, A_a = A.alpha / 255.0;
    const double B_r = B.red / 255.0, B_g = B.green / 255.0, B_b = B.blue / 255.0, B_a = B.alpha / 255.0;

    const double r_sin_A = radius * sin(angle_A), r_cos_A = radius * cos(angle_A);
    const double r_sin_B = radius * sin(angle_B), r_cos_B = radius * cos(angle_B);

    const double h = 4.0 / 3.0 * tan((angle_B - angle_A) / 4.0);

    const double x0 = r_cos_A, y0 = r_sin_A;
    const double x1 = r_cos_A - h * r_sin_A, y1 = r_sin_A + h * r_cos_A;
    const double x2 = r_cos_B + h * r_sin_B, y2 = r_sin_B - h * r_cos_B;
    const double x3 = r_cos_B, y3 = r_sin_B;

    cairo_mesh_pattern_begin_patch(pat);

    cairo_mesh_pattern_move_to(pat, 0, 0);
    cairo_mesh_pattern_line_to(pat, x0, y0);

    cairo_mesh_pattern_curve_to(pat, x1, y1, x2, y2, x3, y3);
    cairo_mesh_pattern_line_to(pat, 0, 0);

    cairo_mesh_pattern_set_corner_color_rgba(pat, 0, A_r, A_g, A_b, A_a);
    cairo_mesh_pattern_set_corner_color_rgba(pat, 1, A_r, A_g, A_b, A_a);
    cairo_mesh_pattern_set_corner_color_rgba(pat, 2, B_r, B_g, B_b, B_a);
    cairo_mesh_pattern_set_corner_color_rgba(pat, 3, B_r, B_g, B_b, B_a);

    cairo_mesh_pattern_end_patch(pat);
}

cairo_pattern_t* conic_gradient::create_pattern(double angle, double radius,
                                                const std::vector<bg_color_point>& color_points) {
    if (color_points.empty()) {
        return nullptr;
    }

    if (color_points.size() == 1) {
        return create_pattern(angle, radius, {color_points[0], color_points[0], color_points[0]});
    }

    if (color_points.size() == 2) {
        bg_color_point mid;
        const litehtml::web_color &a = color_points[0].color, b = color_points[1].color;
        mid.offset = 0.5;
        mid.color.red = interpolate(a.red, b.red);
        mid.color.green = interpolate(a.green, b.green);
        mid.color.blue = interpolate(a.blue, b.blue);
        mid.color.alpha = interpolate(a.alpha, b.alpha);
        return create_pattern(angle, radius, {color_points[0], mid, color_points[1]});
    }

    const double two_pi = 2.0 * M_PI;

    cairo_pattern_t* pat = cairo_pattern_create_mesh();

    for (size_t i = 0; i < color_points.size() - 1; i++) {
        const bg_color_point &cp_A = color_points[i], cp_B = color_points[i + 1];
        double angle_A = cp_A.offset * two_pi + angle;
        double angle_B = cp_B.offset * two_pi + angle;
        sector_patch(pat, radius, angle_A, cp_A.color, angle_B, cp_B.color);
    }

    return pat;
}

// closure is pointer to PyObject* that will hold the bytes object
cairo_status_t cairo_wrapper::write_to_vector(void* closure, const unsigned char* data, unsigned int length) {
    auto* vec = static_cast<std::vector<unsigned char>*>(closure);
    if (!vec) {
        return CAIRO_STATUS_DEVICE_ERROR;
    }
    try {
        vec->insert(vec->end(), data, data + length);
    }
    catch (const std::bad_alloc&) {
        return CAIRO_STATUS_NO_MEMORY;
    } catch (...) {
        return CAIRO_STATUS_DEVICE_ERROR;
    }
    return CAIRO_STATUS_SUCCESS;
}

cairo_status_t cairo_wrapper::read_from_view(void* closure, unsigned char* buffer, unsigned int length) {
    BufferView* view = static_cast<BufferView*>(closure);
    if (!view || view->size <= 0 || !view->data || view->offset < 0 || view->offset > view->size) {
        return CAIRO_STATUS_READ_ERROR;
    }
    if (view->offset + length > view->size) {
        return CAIRO_STATUS_READ_ERROR;
    }
    memcpy((void*)buffer, view->data + view->offset, length);
    view->offset += length;
    return CAIRO_STATUS_SUCCESS;
}