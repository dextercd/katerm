#include <cstdint>
#include <katerm/terminal.hpp>
#include <katerm/terminal_decoder.hpp>

extern "C" int LLVMFuzzerTestOneInput(
        std::uint8_t const* const bytes,
        std::size_t const size)
{
    katerm::decoder decoder;
    katerm::terminal term{{132, 80}};
    katerm::terminal_instructee instructee{&term};
    decoder.decode(
        reinterpret_cast<const char*>(bytes),
        size,
        instructee);

    return 0;
}
