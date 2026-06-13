#include "ui.h"

#include <libgen.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

std::string dirname_of(const char* path) {
    if (!path || !*path) return ".";
    std::vector<char> buffer(path, path + std::strlen(path) + 1);
    return std::string(dirname(buffer.data()));
}

std::string detect_app_dir(const char* argv0) {
    const char* env = std::getenv("KINDLESUDOKU_HOME");
    if (env && *env) return std::string(env);

    char resolved[4096];
    if (realpath(argv0, resolved)) {
        std::string bin_dir = dirname_of(resolved);
        const std::string suffix = "/bin";
        if (bin_dir.size() > suffix.size() && bin_dir.compare(bin_dir.size() - suffix.size(), suffix.size(), suffix) == 0) {
            return bin_dir.substr(0, bin_dir.size() - suffix.size());
        }
        return bin_dir;
    }
    return ".";
}

} // namespace

int main(int argc, char** argv) {
    const std::string app_dir = detect_app_dir(argc > 0 ? argv[0] : "kindlesudoku");
    ksudoku::SudokuApp app(argc, argv, app_dir);
    return app.run();
}
