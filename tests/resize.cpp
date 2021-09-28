#include <cstring>

#include <catch2/catch.hpp>

#include <katerm/terminal.hpp>
#include <katerm/terminal_decoder.hpp>

TEST_CASE("Content is moved up", "[resize]") {
    katerm::terminal term{{12, 8}};
    katerm::decoder decoder{};
    katerm::terminal_instructee instructee{&term};

    auto text = "\r\n\r\n\r\n\r\nHello\r\nWorld!\r";
    decoder.decode(text, std::strlen(text), instructee);

    SECTION("Before resize") {
        REQUIRE(term.screen.get_glyph(term.cursor.pos).code == 'W');
    }

    term.resize({12, 2});

    SECTION("After resize") {
        REQUIRE(term.cursor.pos == katerm::position{0, 1});
        REQUIRE(term.screen.get_glyph(term.cursor.pos).code == 'W');
        REQUIRE(term.screen.get_glyph({0, 0}).code == 'H');
        REQUIRE(term.screen.get_glyph({4, 0}).code == 'o');
    }
}

TEST_CASE("Increasing is stable", "[resize]") {
    katerm::terminal term{{12, 8}};
    katerm::decoder decoder{};
    katerm::terminal_instructee instructee{&term};

    auto text = "\r\n\r\n\r\n\r\nHello\r\nWorld!\r";
    decoder.decode(text, std::strlen(text), instructee);

    SECTION("Before resize") {
        REQUIRE(term.screen.get_glyph(term.cursor.pos).code == 'W');
        REQUIRE(term.screen.get_glyph({1, 4}).code == 'e');
        REQUIRE(term.screen.get_glyph({2, 4}).code == 'l');
        REQUIRE(term.screen.get_glyph({5, 5}).code == '!');
    }

    auto before_resize = term.cursor.pos;
    term.resize({12, 16});

    SECTION("After resize") {
        REQUIRE(term.cursor.pos == before_resize);

        REQUIRE(term.screen.get_glyph({1, 4}).code == 'e');
        REQUIRE(term.screen.get_glyph({2, 4}).code == 'l');
        REQUIRE(term.screen.get_glyph({5, 5}).code == '!');
    }
}
