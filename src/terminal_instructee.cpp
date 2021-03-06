#include <katerm/terminal.hpp>
#include <iostream>

namespace katerm {

void terminal_instructee::tab()
{
    term->tab();
}

void terminal_instructee::line_feed(bool first_col)
{
    term->newline(first_col);
}

void terminal_instructee::carriage_return()
{
    term->move_cursor({0, term->cursor.pos.y});
}

void terminal_instructee::backspace()
{
    term->move_cursor_forward(-1);
}

void terminal_instructee::write_char(code_point code)
{
    term->write_char(code);
}

void terminal_instructee::clear_to_bottom()
{
    term->clear(
        term->cursor.pos,
        {
            term->screen.size().width - 1,
            term->screen.size().height - 1
        });
}

void terminal_instructee::clear_from_top()
{
    term->clear(
        {0, 0},
        term->cursor.pos);
}

void terminal_instructee::clear_screen()
{
    term->clear(
        {0, 0},
        {
            term->screen.size().width - 1,
            term->screen.size().height - 1
        });
}

void terminal_instructee::clear_to_end()
{
    term->clear(
        term->cursor.pos,
        {
            term->screen.size().width - 1,
            term->cursor.pos.y
        });
}

void terminal_instructee::clear_from_begin()
{
    term->clear(
        {0, term->cursor.pos.y},
        term->cursor.pos);
}

void terminal_instructee::clear_line()
{
    term->clear(
        {0, term->cursor.pos.y},
        {term->screen.size().width - 1, term->cursor.pos.y});
}

void terminal_instructee::position_cursor(position pos)
{
    term->move_cursor(pos);
}

void terminal_instructee::change_mode_bits(bool set, terminal_mode mode)
{
    term->mode.set(mode, set);
}

void terminal_instructee::move_cursor(int count, direction dir, bool first_col)
{
    switch(dir) {
        case direction::up:
            term->move_cursor(
                {term->cursor.pos.x, term->cursor.pos.y - count});
            break;
        case direction::down:
            term->move_cursor(
                {term->cursor.pos.x, term->cursor.pos.y + count});
            break;
        case direction::forward:
            term->move_cursor(
                {term->cursor.pos.x + count, term->cursor.pos.y});
            break;
        case direction::back:
            term->move_cursor(
                {term->cursor.pos.x - count, term->cursor.pos.y});
            break;
    }

    if (first_col)
        term->move_cursor({0, term->cursor.pos.y});
}

void terminal_instructee::move_to_column(int column)
{
    term->move_cursor({column, term->cursor.pos.y});
}

void terminal_instructee::move_to_row(int row)
{
    term->move_cursor({term->cursor.pos.x, row});
}

void terminal_instructee::delete_chars(int count)
{
    term->delete_chars(count);
}

void terminal_instructee::erase_chars(int count)
{
    term->clear(
        term->cursor.pos,
        {term->cursor.pos.x + count - 1, term->cursor.pos.y});
}

void terminal_instructee::delete_lines(int count)
{
    term->scroll_up(term->cursor.pos.y, count);
}

void terminal_instructee::reverse_line_feed()
{
    if (term->cursor.pos.y == 0) {
        term->scroll_down();
    } else {
        term->move_cursor({term->cursor.pos.x, term->cursor.pos.y - 1});
    }
}

void terminal_instructee::insert_blanks(int count)
{
    term->insert_blanks(count);
}

void terminal_instructee::insert_newline(int count)
{
    term->insert_newline(count);
}

void terminal_instructee::set_charset_table(int table_index, charset cs)
{
    term->translation_tables[table_index] = cs;
}

void terminal_instructee::use_charset_table(int table_index)
{
    term->using_translation_table = table_index;
}

void terminal_instructee::reset_style()
{
    term->reset_style();
}

void terminal_instructee::set_foreground(colour c)
{
    term->cursor.style.fg = c;
}

void terminal_instructee::set_background(colour c)
{
    term->cursor.style.bg = c;
}

void terminal_instructee::set_reversed(bool enable)
{
    term->cursor.style.mode.set(glyph_attr_bit::reversed, enable);
}

void terminal_instructee::default_foreground()
{
    term->cursor.style.fg = default_style.fg;
}

void terminal_instructee::default_background()
{
    term->cursor.style.bg = default_style.bg;
}

void terminal_instructee::set_bold(bool enable)
{
    term->cursor.style.mode.set(glyph_attr_bit::bold, enable);
}

void terminal_instructee::set_mouse_mode(mouse_mode mode, bool set)
{
    if (set)
        term->mouse = mode;
    else
        term->mouse = mouse_mode::none;
}

void terminal_instructee::set_mouse_mode_extended(bool set)
{
    term->mode.set(terminal_mode_bit::extended_mouse, set);
}

void terminal_instructee::set_bracketed_paste(bool set)
{
    term->mode.set(terminal_mode_bit::bracketed_paste, set);
}

} // katerm::
