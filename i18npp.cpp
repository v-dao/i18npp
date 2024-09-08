#include "i18npp.hpp"

#include <string.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <fstream>
#include <unordered_map>

#include <iostream>

namespace i18npp
{
    static std::shared_ptr<translator> g_translator = std::make_shared<translator>();

    std::shared_ptr<translator> default_translator()
    {
        return g_translator;
    }

    void set_default_translator(std::shared_ptr<translator> p_translator)
    {
        g_translator = p_translator;
    }

    struct set_bool
    {
        bool &m_val;
        bool  m_old;

        set_bool(bool& val, bool tmp):m_val(val), m_old(val)
        {
            val = tmp;
        }

        ~set_bool() { m_val = m_old; }
    };

    struct translator::detail
    {
        int callback_index = 0;
        std::string error_string;
        std::unordered_map<std::string_view, std::string_view> id_to_str;

        std::vector<char> raw_buffer;
    };

    translator::translator(): d(std::make_unique<detail>()){}

    int translator::open_raw_mo_data(const char *data, int length)
    {
        return _open_raw_mo_data(data, length, true);
    }

    int translator::open_mo_file(std::filesystem::path filepath)
    {
        std::ifstream ifs(filepath);
        if (!ifs.is_open())
            return -1;

        ifs.seekg(0, std::fstream::end);
        auto filesize = ifs.tellg();
        ifs.seekg(0, std::fstream::beg);

        d->raw_buffer.resize(filesize);
        ifs.read(d->raw_buffer.data(), filesize);
        return _open_raw_mo_data(d->raw_buffer.data(), static_cast<int>(filesize), false);
    }

    std::string_view translator::ttr(std::string_view str) const
    {
        auto findit = d->id_to_str.find(str);
        if (findit == d->id_to_str.end())
            return str;
        else
            return findit->second;
    }

    std::string_view translator::get_last_error() const
    {
        return d->error_string;
    }

    int translator::_open_raw_mo_data(const char *data, int length, bool save)
    {
        if (save)
        {
            d->raw_buffer.resize(length);
            memcpy(d->raw_buffer.data(), data, length);
            data = d->raw_buffer.data();
        }

        uint32_t magic;
        uint32_t file_format_revision;
        uint32_t number_of_strings;
        uint32_t offset_of_table_with_original_strings;
        uint32_t offset_of_table_with_translation_strings;
        uint32_t size_of_hashing_table;
        uint32_t offset_of_hashing_table;

        if (length < 28)
            return -1;

        memcpy(&magic,                                    data + 0, 4);
        memcpy(&file_format_revision,                     data + 4, 4);
        memcpy(&number_of_strings,                        data + 8, 4);
        memcpy(&offset_of_table_with_original_strings,    data + 12, 4);
        memcpy(&offset_of_table_with_translation_strings, data + 16, 4);
        memcpy(&size_of_hashing_table,                    data + 20, 4);
        memcpy(&offset_of_hashing_table,                  data + 24, 4);

        if (magic != 0x950412DE)
            return -1;

        printf("magic %X\n", magic);
        printf("file format revision = %d\n", file_format_revision);
        printf("number of strings %d\n", number_of_strings);
        printf("offset of table with original strings %d\n", offset_of_table_with_original_strings);
        printf("offset of table with translation strings %d\n", offset_of_table_with_translation_strings);
        printf("size of hashing table %d\n", size_of_hashing_table);
        printf("offset of hashing table %d\n", offset_of_hashing_table);

        struct LengthAndOffset
        {
            uint32_t length = 0;
            uint32_t offset = 0;
        };

        std::vector<LengthAndOffset> original_lo(number_of_strings);
        memcpy(original_lo.data(), data + offset_of_table_with_original_strings, 8 * number_of_strings);
        std::vector<LengthAndOffset> translation_lo(number_of_strings);
        memcpy(translation_lo.data(), data + offset_of_table_with_translation_strings, 8 * number_of_strings);

        for (int i = 0; i < number_of_strings; i++)
        {
            LengthAndOffset& original = original_lo[i];
            LengthAndOffset& translation = translation_lo[i];

            std::string_view original_str{data + original.offset, original.length};
            std::string_view translation_str{data + translation.offset, translation.length};

            d->id_to_str[original_str] = translation_str;
            //std::cout << ">>> " << original_str << std::endl;
            //std::cout << ">>> " << translation_str << std::endl;
            //std::cout << "========================"  << std::endl;
        }

        return number_of_strings;
    }

}
