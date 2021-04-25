#include <catch2/catch.hpp>

#include <gd100/terminal.hpp>

gd100::terminal test_term()
{
    // non square so that mixups with width/height will be caught.
    return gd100::terminal{{5, 4}};
}

TEST_CASE("Terminal writing", "[write]") {
    auto t = test_term();

    SECTION("After initialisation") {
        REQUIRE(t.cursor.pos == gd100::position{0, 0});
        REQUIRE(t.screen.get_line(3)[4].code == 0);
    }

    SECTION("Writing sets a character and moves the cursor") {
        t.write_char('A');

        REQUIRE(t.screen.get_glyph({0, 0}).code == 'A');
        REQUIRE(t.cursor.pos == gd100::position{1, 0});
    }

    SECTION("Cursor only goes to the next line when placing character there") {
        // Write five times to put the cursor to the end
        t.write_char('A'); t.write_char('B'); t.write_char('C');
        t.write_char('D'); t.write_char('E');

        REQUIRE(t.screen.get_glyph({4, 0}).code == 'E');
        REQUIRE(t.cursor.pos == gd100::position{4, 0});

        t.write_char('F');

        REQUIRE(t.screen.get_glyph({0, 1}).code == 'F');
        REQUIRE(t.cursor.pos == gd100::position{1, 1});
    }
}

TEST_CASE("Terminal scrolling", "[scroll]") {
    auto t = test_term();

    auto write_full_line = [&] {
        t.write_char('A'); t.write_char('B'); t.write_char('C');
        t.write_char('D'); t.write_char('E');
    };

    SECTION("Should start scrolling when no space left") {
        write_full_line();
        write_full_line();
        write_full_line();
        write_full_line();

        REQUIRE(t.cursor.pos == gd100::position{4, 3});

        t.write_char('a');

        REQUIRE(t.cursor.pos == gd100::position{1, 3});
    }
}

TEST_CASE("Terminal driven via decode", "[terminal-decode]") {
    auto t = test_term();
    SECTION("Basic write") {
        char hw[] = "Hello world!";
        auto processed = t.process_bytes(hw, sizeof(hw) - 1);

        REQUIRE(processed == sizeof(hw) - 1);

        REQUIRE(t.screen.get_glyph({0, 0}).code == 'H');
        REQUIRE(t.screen.get_glyph({4, 0}).code == 'o');
        REQUIRE(t.screen.get_glyph({0, 1}).code == ' ');
        REQUIRE(t.screen.get_glyph({4, 1}).code == 'l');
        REQUIRE(t.screen.get_glyph({0, 2}).code == 'd');
        REQUIRE(t.screen.get_glyph({1, 2}).code == '!');

        REQUIRE(t.cursor.pos == gd100::position{2, 2});
    }
}
