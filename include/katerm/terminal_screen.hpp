#ifndef KATERM_TERMINAL_SCREEN_HPP
#define KATERM_TERMINAL_SCREEN_HPP

#include <vector>
#include <memory>

#include "glyph.hpp"

namespace katerm {

struct line {
    glyph* glyphs;
    bool changed;
};

class terminal_screen {
private:
    extend m_size;
    int m_scroll = 0;

public:
    std::vector<glyph> data;
    std::vector<line> lines;

    terminal_screen()
        : terminal_screen({80, 25})
    {
    }

    terminal_screen(extend screen_sz);

    int resize(extend new_size, int preserve_column, glyph fill_glyph);
    void resize(extend new_size);

    void fill_lines(int line_beg, int line_end, glyph fill_glyph);

    void scroll_up(int keep_top, int const count, glyph fill);
    void scroll_down(int keep_top, int const count, glyph fill);

    glyph* get_line(int line);
    glyph& get_glyph(position pos);

    glyph const* get_line(int line) const;
    glyph const& get_glyph(position pos) const;

    extend size() const;
    void mark_dirty(int start, int end);

    // Retrieve how much the scrolling has changed +/-
    int changed_scroll() const;

    void clear_changes();
    void move_scroll(int change);

private:
    void set_scroll(int scroll);
};

} // katerm::

#endif // header guard
