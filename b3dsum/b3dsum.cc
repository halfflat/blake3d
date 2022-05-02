// For now, let's just try to hash a single chunk.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

#include <b3d/b3d.h>
#include <tinyopt/tinyopt.h>


b3d::chaining_t run_one_chunk(const std::byte* data, std::size_t sz) {
    assert(sz<=1024);
    return b3d::process_chunk(b3d::default_key, 0, data, sz, true);
}

const char* usage_info =
    "[OPTION] [FILE]\n"
    "\n"
    "  -z, --zero=LEN          hash an all-zero document of LEN bytes\n"
    "  -h, --help              print usage information and exit\n"
    "\n"
    "Specify either a FILE or a zero document with -z.\n"
    "The maximum FILE size or zero length is 1024.\n";

int main(int argc, char** argv) {
    std::vector<std::byte> source(1024);

    try {
        auto help = [argv0 = argv[0]] { to::usage(argv0, usage_info); };

        std::string file;
        std::optional<uint64_t> zero;

        to::option opts[] = {
            { to::action(help), to::flag, to::exit, "-h", "--help" },
            { zero, "-z", "--zero" },
            { file, to::single },
        };

        if (!to::run(opts, argc, argv+1)) return 0;
        if (argv[1]) throw to::option_error("unrecogonized argument", argv[1]);
        if (!(file.empty() ^ !zero)) throw to::option_error("either a FILE or -z must be provied");

        if (zero) {
            if (zero.value()>source.size()) throw to::option_error("maximum document size is 1024");
            source.resize(zero.value());
        }
        else {
            std::FILE* f = std::fopen(file.c_str(), "rb");
            if (!f) throw to::option_error("unable to open specified file");

            std::size_t n = std::fread(source.data(), 1, source.size(), f);
            if (std::ferror(f)) throw std::runtime_error("error reading specified file");
            if (n==1024 && std::fgetc(f)!=EOF) throw std::runtime_error("maximum document size is 1024");

            if (n<source.size()) source.resize(n);
            std::fclose(f);
        }
    }
    catch (to::option_error& e) {
        to::usage_error(argv[0], usage_info, e.what());
        return 1;
    }
    catch (std::runtime_error& e) {
        std::cerr << argv[0] << ": " << e.what() << '\n';
        return 1;
    }


    b3d::chaining_t result = run_one_chunk(source.data(), source.size());
    for (auto b: b3d::as_bytes(result)) {
        std::printf("%02hhx", std::to_integer<unsigned char>(b));
    }
    std::puts("");

    return 0;
}
