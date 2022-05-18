// For now, let's just try to hash a single chunk.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <optional>
#include <string>
#include <vector>

#include <b3d/b3d.h>
#include <tinyopt/tinyopt.h>


b3d::chaining_t run_one_chunk(const std::byte* data, std::size_t sz) {
    assert(sz<=1024);
    return b3d::hash_chunk(b3d::default_key, 0, data, sz, true);
}

const char* usage_info =
    "[OPTION] [FILE]\n"
    "\n"
    "  -z, --zero=LEN          hash an all-zero document of LEN bytes\n"
    "  -h, --help              print usage information and exit\n"
    "\n"
    "Specify either a FILE or a zero document with -z.\n"
    "The maximum FILE size or zero length is 1024.\n"
    "\n"
    "Debug modes ignore other options and take special parameters:\n"
    "  --run-compress [h=UINT32,...,UINT32] m=UINT32,...,UINT32 t=UINT64 b=UINT32 d=UINT32\n"
    "                          Run compression function on a block m of 16\n"
    "                          4-byte words. The chaining key h is given\n"
    "                          by 8 4-byte wotrds, defaulting to the IV\n"
    "                          values. t is the chunk counter, b the block length\n"
    "                          in bytes (default 64), and d the domain flag word.\n";

struct options {
    std::string file;
    std::optional<std::uint64_t> zero;

    bool run_compress = false;
    // parameters for --run-compress:
    b3d::chaining_t param_h = b3d::IV;
    b3d::block_t param_m;
    std::uint64_t param_t = 0;
    std::uint32_t param_b = 64;
    std::uint32_t param_d = 0;
};

// Custom integer parser accepts octal and hex.

template <typename I>
to::maybe<I> int_parser(const char* text) {
    using namespace to;

    if (!text) return nothing;
    I v;
    std::istringstream stream(text);
    if (!(stream >> std::setbase(0) >> v)) return nothing;
    if (!stream.eof()) stream >> std::ws;
    return stream.eof()? maybe<I>(v): nothing;
}

// Return false for early exit.
bool parse_args(int& argc, char** argv, options& O) {
    using namespace to::literals;
    using namespace to;

    std::vector<std::uint32_t> param_h_vec, param_m_vec;
    auto help = [argv0 = argv[0]] { to::usage(argv0, usage_info); };

    auto parse_u32 = int_parser<std::uint32_t>;
    auto parse_u64 = int_parser<std::uint64_t>;

    option opts[] = {
        { action(help), flag, to::exit, "-h", "--help" },
        { O.zero, "-z", "--zero", when(0) },
        { O.file, single, when(0) },
        { set(O.run_compress), flag, "--run-compress", then(1) },
        // --run-compress mode:
        { {param_h_vec, delimited(',', parse_u32)}, "h"_long, when(1) },
        { {param_m_vec, delimited(',', parse_u32)}, "m"_long, when(1) },
        { {O.param_t, parse_u64}, "t"_long, when(1) },
        { {O.param_b, parse_u32}, "b"_long, when(1) },
        { {O.param_d, parse_u32}, "d"_long, when(1) }
    };

    if (!run(opts, argc, argv+1)) return false;
    if (argv[1]) throw option_error("unrecognized argument", argv[1]);

    if (O.run_compress) {
        if (!param_h_vec.empty()) {
            if (param_h_vec.size()>8) throw option_error("chaining key too long");
            else {
                std::copy(param_h_vec.begin(), param_h_vec.end(), O.param_h.begin());
                std::fill(O.param_h.begin()+param_h_vec.size(), O.param_h.end(), 0);
            }
        }

        if (param_m_vec.size()>16) throw option_error("message block too long");
        else {
            std::copy(param_m_vec.begin(), param_m_vec.end(), O.param_m.begin());
            std::fill(O.param_m.begin()+param_m_vec.size(), O.param_m.end(), 0);
        }
    }
    else {
        if (O.file.empty() ^ !!O.zero) throw option_error("either a FILE or -z must be provied");
    }

    return true;
};


int main(int argc, char** argv) {
    try {
        options O;
        if (!parse_args(argc, argv, O)) return 0;

        if (O.run_compress) {
            b3d::chaining_t h = b3d::compress(O.param_h, O.param_m, O.param_t, O.param_b, O.param_d);
            for (auto word: h) std::printf("0x%08x\n", word);
            return 0;
        }

        std::vector<std::byte> message;
        if (O.zero) message.resize(O.zero.value());
        else {
            std::ifstream in(O.file.c_str(), std::ios::binary|std::ios::ate);
            if (!in) throw std::runtime_error("unable to open file for reading");

            in.exceptions(std::ifstream::failbit);
            std::streamoff sz = in.tellg();
            if (sz<0) throw std::runtime_error("error reading file");

            message.resize(sz);
            in.seekg(0);
            in.read((char *)message.data(), sz);
        }

        b3d::chaining_t h = b3d::hash(b3d::IV, message.data(), message.size());

        char repn[65];
        repn[64] = 0;
        b3d::to_hex_chars(repn, repn+64, b3d::as_bytes(h));
        std::puts(repn);
    }
    catch (to::option_error& e) {
        to::usage_error(argv[0], usage_info, e.what());
        return 1;
    }
    catch (std::runtime_error& e) {
        std::cerr << argv[0] << ": " << e.what() << '\n';
        return 1;
    }

    return 0;
}
