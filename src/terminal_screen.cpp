#include <type_traits>
#include <algorithm>

#include <katerm/terminal_screen.hpp>

namespace katerm {

static_assert(std::is_copy_assignable_v<terminal_screen>);

terminal_screen::terminal_screen(extend screen_sz)
    : m_size{screen_sz}
    , data(m_size.width * m_size.height, glyph{})
{
    lines.resize(m_size.height);
    int index = 0;
    for(auto& line : lines) {
        line.glyphs = data.data() + index * m_size.width;
        line.changed = false;
        ++index;
    }
}

int terminal_screen::resize(
        extend const new_size,
        int preserve_column,
        glyph const fill_glyph)
{
    if (preserve_column >= new_size.height) {
        auto diff = preserve_column - new_size.height;
        scroll_up(0, diff + 1, fill_glyph);
        preserve_column = new_size.height - 1;
    }

    resize(new_size);

    return preserve_column;
}

void terminal_screen::resize(extend new_size)
{
    auto replacement = terminal_screen{m_size};
    for (auto line = 0; line < new_size.height && line < size().height; ++line) {
        for (auto x = 0; x < new_size.width && x < size().width; ++x) {
            replacement.lines[line].glyphs[x] = lines[line].glyphs[x];
        }
    }

    *this = std::move(replacement);
    mark_dirty(0, size().height);
}

void terminal_screen::fill_lines(int line_beg, int line_end, glyph fill_glyph)
{
    line_end = std::clamp(line_end, 0, size().height);
    line_beg = std::clamp(line_beg, 0, line_end);

    for(auto line_it{line_beg}; line_it != line_end; ++line_it) {
        std::fill(
            get_line(line_it),
            get_line(line_it) + size().width,
            fill_glyph);
    }

    mark_dirty(line_beg, line_end);
}

void terminal_screen::scroll_up(int keep_top, int const count, glyph fill)
{
    auto const height = size().height;

    keep_top = std::clamp(keep_top, 0, height);
    auto const move_end = height;
    auto const move_to = std::clamp(keep_top + count, 0, height);

    std::rotate(
        lines.begin() + keep_top,
        lines.begin() + move_to,
        lines.begin() + move_end);

    fill_lines(move_end - count, move_end, fill);

    if (keep_top < height / 2) {
        mark_dirty(0, keep_top);
        move_scroll(count);
    } else {
        mark_dirty(keep_top, move_end);
    }
}

void terminal_screen::scroll_down(int keep_top, int const count, glyph fill)
{
    auto const height = size().height;

    keep_top = std::clamp(keep_top, 0, height);
    auto const move_end = height;
    auto const move_to = std::clamp(move_end - count, keep_top, height);

    std::rotate(
        lines.begin() + keep_top,
        lines.begin() + move_to,
        lines.begin() + move_end);

    fill_lines(move_end - count, move_end, fill);

    if (keep_top < height / 2) {
        mark_dirty(0, keep_top);
        move_scroll(-count);
    } else {
        mark_dirty(keep_top, move_end);
    }
}

glyph* terminal_screen::get_line(int line)
{
    return lines[line].glyphs;
}

glyph& terminal_screen::get_glyph(position pos)
{
    return get_line(pos.y)[pos.x];
}

const glyph* terminal_screen::get_line(int line) const
{
    return lines[line].glyphs;
}

const glyph& terminal_screen::get_glyph(position pos) const
{
    return get_line(pos.y)[pos.x];
}

extend terminal_screen::size() const
{
    return m_size;
}

void terminal_screen::mark_dirty(int start, int end)
{
    auto const height = size().height;
    start = std::clamp(start, 0, height);
    end = std::clamp(end, 0, height);

    while(start < end)
        lines[start++].changed = true;
}

int terminal_screen::changed_scroll() const
{
    return m_scroll;
}

void terminal_screen::clear_changes()
{
    set_scroll(0);
    for (auto& line : lines)
        line.changed = false;
}

void terminal_screen::move_scroll(int change)
{
    set_scroll(m_scroll + change);
}

void terminal_screen::set_scroll(int const scroll)
{
    auto const height = size().height;
    auto const bounded = scroll % height;
    if (bounded >= 0)
        m_scroll = bounded;
    else
        m_scroll = height + bounded;
}

} // katerm::
