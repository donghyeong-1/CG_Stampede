#include "core/Platform.h"

std::filesystem::path ExecutableFolder()
{
    wchar_t executable[32768]{};
    DWORD length = GetModuleFileNameW(nullptr, executable, 32768);
    if (length > 0 && length < 32768) { return std::filesystem::path(executable).parent_path(); }
    return {};
}

std::filesystem::path AssetPath(const char *name)
{
    std::filesystem::path executableFolder = ExecutableFolder();
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
