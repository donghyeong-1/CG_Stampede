#include "core/Platform.h"

std::filesystem::path AssetPath(const char *name)
{
    wchar_t executable[32768]{};
    DWORD length = GetModuleFileNameW(nullptr, executable, 32768);
    std::filesystem::path executableFolder;
    if (length > 0 && length < 32768)
    {
        executableFolder = std::filesystem::path(executable).parent_path();
    }
    const std::filesystem::path paths[]{executableFolder / name, std::filesystem::path(name)};
    for (const auto &path : paths)
    {
        if (std::filesystem::is_regular_file(path)) { return path; }
    }
    std::cerr << "Asset file not found: " << name << '\n';
    return executableFolder / name;
}

std::string ReadTextFile(const char *name)
{
    std::ifstream file(AssetPath(name), std::ios::binary);
    if (!file) { return {}; }
    return std::string(std::istreambuf_iterator<char>(file), {});
}
