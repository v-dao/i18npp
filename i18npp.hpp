#pragma once

#include <memory>
#include <filesystem>
#include <functional>
#include <string_view>

namespace i18npp
{
    class translator
    {
    public:
        translator();

        translator(const translator&) = delete;

        int open_raw_mo_data(const char *data, int length = -1);

        int open_mo_file(std::filesystem::path filepath);

        std::string_view ttr(std::string_view str) const;

        std::string_view get_last_error() const;

        std::unique_ptr<std::function<void(int)>> retranslate_callback(std::function<void()> callback);

    private:
        struct detail;
        std::unique_ptr<detail> d;

        int _open_raw_mo_data(const char *data, int length, bool save_data);
    };

    std::shared_ptr<translator> default_translator();

    void set_default_translator(std::shared_ptr<translator> p_translator);

    inline std::string_view ttr(std::string_view str) {
        return default_translator()->ttr(str);
    }
}
