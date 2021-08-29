#include <catch2/catch.hpp>

#include <katerm/terminal.hpp>
#include <katerm/terminal_decoder.hpp>

TEST_CASE("utf-8", "[utf-8]") {
    auto decode_utf8 = [](auto const& utf8) {
        auto t = katerm::terminal{{10, 10}};
        auto d = katerm::decoder{};
        auto instructee = katerm::terminal_instructee{&t};

        d.decode(utf8, sizeof(utf8) - 1, instructee);

        return t.screen.get_glyph({0, 0}).code;
    };

    SECTION("Incomplete sequence is not consumed") {
        REQUIRE(decode_utf8("\xe2\x82") == 0);
        REQUIRE(decode_utf8("\xe2\x82\xac") == U'€');
    }

    SECTION("Check decodings") {
        REQUIRE(decode_utf8("k") == U'k');
        REQUIRE(decode_utf8("ē") == U'ē');
        REQUIRE(decode_utf8("¥") == U'¥');
        REQUIRE(decode_utf8("│") == U'│');
        REQUIRE(decode_utf8("¢") == U'¢');
        REQUIRE(decode_utf8("ह") == U'ह');
        REQUIRE(decode_utf8("€") == U'€');
        REQUIRE(decode_utf8("한") == U'한');
        REQUIRE(decode_utf8("𐍈") == U'𐍈');
        REQUIRE(decode_utf8("⚡") == U'⚡');
    }
}
