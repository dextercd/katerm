#include <iostream>
#include <algorithm>
#include <charconv>

#include <katerm/terminal_decoder.hpp>
#include <katerm/colours.hpp>

namespace katerm {

namespace {

constexpr char esc = '\x1b';

constexpr int max_csi_params = 10;

constexpr bool is_csi_final(char const c)
{
    return c >= 0x40 && c <= 0x7e;
}

using decode_session_ret = std::size_t;
#define RETURN_SUCCESS return index;
#define RETURN_NOT_ENOUGH_DATA return 0;

// You can think of the COMMON_PARAMS as being the member variables.
// Except that they are copied between function calls.
// This encourages the compiler to keep this stuff in registers and allows it to
// optimize more.
#define COMMON_PARAMS                  \
    const char* const buffer_one,      \
    std::size_t const buffer_one_size, \
    const char* const buffer_two,      \
    std::size_t const buffer_two_size, \
    std::size_t index,                 \
    decoder_instructee& t

#define COMMON_PARAMS_INDEX_REF        \
    const char* const buffer_one,      \
    std::size_t const buffer_one_size, \
    const char* const buffer_two,      \
    std::size_t const buffer_two_size, \
    std::size_t& index,                \
    decoder_instructee& t

#define ARGS         \
    buffer_one,      \
    buffer_one_size, \
    buffer_two,      \
    buffer_two_size, \
    index,           \
    t

decode_session_ret decode_utf8(COMMON_PARAMS, unsigned char const first);
decode_session_ret decode_escape(COMMON_PARAMS);
decode_session_ret decode_set_charset_table(COMMON_PARAMS, int const table_index);
decode_session_ret discard_string(COMMON_PARAMS);

decode_session_ret decode_csi(COMMON_PARAMS);

decode_session_ret decode_csi_priv(
        COMMON_PARAMS,
        int const* const params,
        int const param_count,
        char const final);

decode_session_ret decode_csi_pub(
        COMMON_PARAMS,
        int const* const params,
        int const param_count,
        char const final);

decode_session_ret decode_set_graphics(
        COMMON_PARAMS,
        int const* const params,
        int const param_count);

decode_session_ret decode_private_set(
        COMMON_PARAMS,
        int const* const params,
        int const param_count,
        bool const set);

decode_session_ret decode_public_set(
        COMMON_PARAMS,
        int const* const params,
        int const param_count,
        bool const set);

std::size_t characters_left(COMMON_PARAMS)
{
    return buffer_one_size + buffer_two_size - index;
}

char peek(COMMON_PARAMS, std::size_t forward = 0)
{
    auto ix = index + forward;

    if (ix < buffer_one_size)
        return buffer_one[ix];

    ix -= buffer_one_size;

    if (ix < buffer_two_size)
        return buffer_two[ix];

    return '\0';
}

char consume(COMMON_PARAMS_INDEX_REF)
{
    auto ret = peek(ARGS);
    if (characters_left(ARGS) > 0)
        index++;

    return ret;
}

decode_session_ret decode_one(COMMON_PARAMS)
{
    if (!characters_left(ARGS))
        RETURN_NOT_ENOUGH_DATA;

    auto const first = static_cast<unsigned char>(consume(ARGS));

    if (first > 0x7f) {
        return decode_utf8(ARGS, first);
    }

    switch(first) {
        case '\t':
            t.tab();
            RETURN_SUCCESS;

        case '\f':
        case '\v':
        case '\n':
            t.line_feed(false);
            RETURN_SUCCESS;

        case '\r':
            t.carriage_return();
            RETURN_SUCCESS;

        case '\b':
            t.backspace();
            RETURN_SUCCESS;

        case '\a':
            RETURN_SUCCESS;

        case '\016': /* SO (LS1 -- Locking shift 1) */
        case '\017': /* SI (LS0 -- Locking shift 0) */
            t.use_charset_table(first - '\016');
            RETURN_SUCCESS;

        case 0x85:
            t.line_feed(true);
            RETURN_SUCCESS;

        case esc:
            return decode_escape(ARGS);
    }

    t.write_char(static_cast<code_point>(first));
    RETURN_SUCCESS;
}

decode_session_ret decode_utf8(COMMON_PARAMS, unsigned char const first)
{
    auto codepoint = std::uint32_t{0};
    auto bytes_left = int{};
    auto bits_received = int{};

    if ((first & 0b1110'0000) == 0b1100'0000) {
        bytes_left = 1;
        bits_received = 5;
    } else if ((first & 0b1111'0000) == 0b1110'0000) {
        bytes_left = 2;
        bits_received = 4;
    } else if ((first & 0b1111'1000) == 0b1111'0000) {
        bytes_left = 3;
        bits_received = 3;
    } else {
        RETURN_NOT_ENOUGH_DATA;
    }

    codepoint = (std::uint32_t{first} & 0xff >> (8 - bits_received)) << bytes_left * 6;

    while(true) {
        if (!characters_left(ARGS))
            RETURN_NOT_ENOUGH_DATA;

        auto const utf8_part = static_cast<std::uint32_t>(
                                static_cast<unsigned char>(consume(ARGS)));

        bytes_left -= 1;
        codepoint |= (utf8_part & 0b0011'1111) << bytes_left * 6;

        if (bytes_left == 0)
            break;
    }

    t.write_char(codepoint);
    RETURN_SUCCESS;
}


decode_session_ret decode_escape(COMMON_PARAMS)
{
    if (!characters_left(ARGS))
        RETURN_NOT_ENOUGH_DATA;

    auto const code = consume(ARGS);
    switch(code) {
        case 'n': /* LS2 -- Locking shift 2 */
        case 'o': /* LS3 -- Locking shift 3 */
            t.use_charset_table(code - 'n' + 2);
            RETURN_SUCCESS;

        case '\\': // string terminator
        default:
            RETURN_SUCCESS;

        case '(':
        case ')':
        case '*':
        case '+':
            return decode_set_charset_table(ARGS, code - '(');

        case 'P' : // device control string
        case ']' : // operating system command
        case 'X' : // start of string
        case '^' : // privacy message
        case '_' : // application program command
            return discard_string(ARGS);

        case 'M':
            t.reverse_line_feed();
            RETURN_SUCCESS;

        case '[':
            return decode_csi(ARGS);
    }
}

decode_session_ret decode_set_charset_table(COMMON_PARAMS, int const table_index)
{
    if (!characters_left(ARGS))
        RETURN_NOT_ENOUGH_DATA;

    switch(consume(ARGS)) {
        case '0':
            t.set_charset_table(table_index, charset::graphic0);
            RETURN_SUCCESS;

        case 'B':
            t.set_charset_table(table_index, charset::usa);
            RETURN_SUCCESS;

        default:
            RETURN_SUCCESS; // discard unknown
    }
}

decode_session_ret discard_string(COMMON_PARAMS)
{
    while(true) {
        if (!characters_left(ARGS))
            RETURN_NOT_ENOUGH_DATA;

        switch (consume(ARGS)) {
            case '\a':
                RETURN_SUCCESS;

            case esc:
                if (peek(ARGS) == '\\') {
                    consume(ARGS);
                    RETURN_SUCCESS;
                }
        }
    }
}

decode_session_ret decode_csi(COMMON_PARAMS)
{
    int params[max_csi_params]{};
    int param_count = 0;

    auto const is_private = peek(ARGS) == '?';
    if (is_private)
        consume(ARGS);

    while(true) {
        if (!characters_left(ARGS))
            RETURN_NOT_ENOUGH_DATA;

        auto const first = consume(ARGS);
        if (is_csi_final(first)) {
            if (is_private) {
                return decode_csi_priv(
                        ARGS,
                        params, param_count,
                        first);
            }

            return decode_csi_pub(
                    ARGS,
                    params, param_count,
                    first);
        }

        if (first >= '0' && first <= '9') {
            char read_buffer[] {
                first        , peek(ARGS, 0),
                peek(ARGS, 1), peek(ARGS, 2),
                peek(ARGS, 3), peek(ARGS, 4),
            };

            int value;
            auto const [num_end, ec] = std::from_chars(
                                                std::begin(read_buffer),
                                                std::end(read_buffer),
                                                value);

            if (ec == std::errc::result_out_of_range) {
                value = 0;
            } else if (num_end == std::begin(read_buffer)) {
                value = 0;
            } else {
                auto digits_parsed = num_end - std::begin(read_buffer);
                index += digits_parsed - 1; // Subtract one because we already consumed first
            }

            // If we don't have enough space we just discard it
            if (param_count < max_csi_params)
                params[param_count++] = value;
        } else {
            // not number, so just discard it
        }

        switch(peek(ARGS)) {
            case ':':
            case ';':
                consume(ARGS);
        }
    }
}

decode_session_ret decode_csi_priv(
        COMMON_PARAMS,
        int const* const params,
        int const param_count,
        char const final)
{
    switch (final) {
        case 'l':
        case 'h':
            return decode_private_set(ARGS, params, param_count, final == 'h');
    }

    RETURN_SUCCESS;
}

decode_session_ret decode_csi_pub(
        COMMON_PARAMS,
        int const* const params,
        int const param_count,
        char const final)
{
    auto get_number = [&](int index=0, int default_=0) -> int {
        if (index < param_count && params[index] != 0)
        {
            return params[index];
        }

        return default_;
    };

    switch(final) {
        default:
#if 0
            std::cout << "Unknown ESC [ ";
            for (int i = 0; i != param_count; ++i) {
                std::cout << params[i] << ' ';
            }
            std::cout << final << '\n';
#endif
            break;

        case 'J': {
            switch(get_number(0)) {
                case 0:
                    t.clear_to_bottom();
                    break;
                case 1:
                    t.clear_from_top();
                    break;

                case 2:
                default:
                    t.clear_screen();
                    break;
            }
        } break;

        case 'G':
        case '`':
            t.move_to_column(get_number(0, 1) - 1);
            break;

        case 'd':
            t.move_to_row(get_number(0, 1) - 1);
            break;

        case 'f':
        case 'H':
            t.position_cursor({
                std::max(0, get_number(1) - 1),
                std::max(0, get_number(0) - 1)
            });
            break;

        case 'K': {
            switch(get_number(0)) {
                case 0:
                    t.clear_to_end();
                    break;

                case 1:
                    t.clear_from_begin();
                    break;

                case 2:
                default:
                    t.clear_line();
                    break;

            }
        } break;

        case 'l':
        case 'h':
            return decode_public_set(ARGS, params, param_count, final == 'h');

        case 'A':
            t.move_cursor(get_number(0, 1), direction::up);
            break;

        case 'B':
        case 'e':
            t.move_cursor(get_number(0, 1), direction::down);
            break;

        case 'C':
        case 'a':
            t.move_cursor(get_number(0, 1), direction::forward);
            break;

        case 'D':
            t.move_cursor(get_number(0, 1), direction::back);
            break;

        case 'E':
            t.move_cursor(get_number(0, 1), direction::down, true);
            break;

        case 'F':
            t.move_cursor(get_number(0, 1), direction::up, true);
            break;

        case 'P':
            t.delete_chars(get_number(0, 1));
            break;

        case 'X':
            t.erase_chars(get_number(0, 1));
            break;

        case 'M':
            t.delete_lines(get_number(0, 1));
            break;

        case '@':
            t.insert_blanks(get_number(0, 1));
            break;

        case 'L':
            t.insert_newline(get_number(0, 1));
            break;

        case 'm':
            decode_set_graphics(ARGS, params, param_count);
            break;
    }

    RETURN_SUCCESS;
}

decode_session_ret decode_private_set(
        COMMON_PARAMS,
        int const* const params,
        int const param_count,
        bool const set)
{
    for (int i = 0; i != param_count; ++i) {
        switch(params[i]) {
            case 9:
                t.set_mouse_mode(mouse_mode::x10, set);
                break;

            case 1000:
                t.set_mouse_mode(mouse_mode::button, set);
                break;

            case 1002:
                t.set_mouse_mode(mouse_mode::motion, set);
                break;

            case 1003:
                t.set_mouse_mode(mouse_mode::many, set);
                break;

            case 1006:
                t.set_mouse_mode_extended(set);
                break;

            case 2004:
                t.set_bracketed_paste(set);
        }
    }

    RETURN_SUCCESS;
}

decode_session_ret decode_public_set(
        COMMON_PARAMS,
        int const* const params,
        int const param_count,
        bool const set)
{
    terminal_mode mode;
    for (int i = 0; i < param_count; ++i) {
        auto number = params[i];

        switch(number) {
            case 4:
                mode.set(terminal_mode_bit::insert);
        }
    }

    t.change_mode_bits(set, mode);

    RETURN_SUCCESS;
}

decode_session_ret decode_set_graphics(
        COMMON_PARAMS,
        int const* const params,
        int const param_count)
{
    auto get_number = [&](int index=0, int default_=0) -> int {
        if (index < param_count && params[index] != 0)
        {
            return params[index];
        }

        return default_;
    };

    for(int i{}; i == 0 || i < param_count; ++i) {
        auto num = get_number(i);
        switch(num) {
            case 0:
                t.reset_style();
                break;

            case 1:
                t.set_bold(true);
                break;

            case 22:
                t.set_bold(false);
                break;

            case 7:
                t.set_reversed(true);
                break;

            case 27:
                t.set_reversed(false);
                break;

            case 38:
            case 48: {
                auto is_foreground = num == 38;
                auto col = colour{};
                switch (get_number(i + 1)) {
                    case 5:
                        col = eight_bit_lookup(get_number(i + 2));
                        i += 2;
                        break;

                    case 2:
                        col = colour{
                            (std::uint8_t)std::clamp(get_number(i + 2), 0, 255),
                            (std::uint8_t)std::clamp(get_number(i + 3), 0, 255),
                            (std::uint8_t)std::clamp(get_number(i + 4), 0, 255),
                        };
                        i += 4;
                        break;

                    default:
                        RETURN_SUCCESS;
                }

                if (is_foreground) t.set_foreground(col);
                else               t.set_background(col);
            } break;

            case 39:
                t.default_foreground();
                break;

            case 49:
                t.default_background();
                break;

            default:
                if (num >= 30 && num <= 37)
                    t.set_foreground(sgr_colours[num - 30]);
                if (num >= 90 && num <= 97)
                    t.set_foreground(sgr_colours[num - 90 + 8]);
                if (num >= 40 && num <= 47)
                    t.set_background(sgr_colours[num - 40]);
                if (num >= 100 && num <= 107)
                    t.set_background(sgr_colours[num - 100 + 8]);
        }
    }
    RETURN_SUCCESS;
}

} // anonymous namespace

void decoder::decode(
        char const* const new_bytes,
        int const new_count,
        decoder_instructee& t)
{
    auto index = std::size_t{0};

    while(true) {
        auto new_index = decode_one(buffer.data(),
                                    buffer.size(),
                                    new_bytes,
                                    static_cast<std::size_t>(new_count),
                                    index,
                                    t);

        if (new_index <= index)
            break;

        index = new_index;
    }

    auto new_used = index - buffer.size();
    auto keep_new = std::clamp(static_cast<std::size_t>(new_count) - new_used, (std::size_t)0, static_cast<std::size_t>(new_count));

    buffer.erase(0, index);
    buffer.append(new_bytes + new_count - keep_new, keep_new);
};


} // katerm::
