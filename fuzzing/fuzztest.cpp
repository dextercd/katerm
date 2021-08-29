#include <cstdint>
#include <katerm/terminal.hpp>
#include <katerm/terminal_decoder.hpp>

extern "C" int LLVMFuzzerTestOneInput(
        std::uint8_t const* const bytes,
        std::size_t const size)
{
    gd100::decoder decoder;
    gd100::terminal term{{132, 80}};
    gd100::terminal_instructee instructee{&term};
    decoder.decode(
        reinterpret_cast<const char*>(bytes),
        size,
        instructee);

    return 0;
}
