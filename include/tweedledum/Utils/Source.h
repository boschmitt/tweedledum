/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace tweedledum {

/*! \brief This object owns the source content string. */
class Source {
public:
    static std::unique_ptr<Source> create(
      std::string_view content, uint32_t const offset)
    {
        return std::unique_ptr<Source>(new Source(content, offset));
    }

    static std::unique_ptr<Source> create(
      std::string&& content, uint32_t const offset)
    {
        return std::unique_ptr<Source>(new Source(content, offset));
    }

    virtual ~Source() = default;

    std::string_view content() const
    {
        return content_;
    }

    char const* cbegin() const
    {
        return &(content_.front());
    }

    uint32_t length() const
    {
        return content_.length();
    }

    uint32_t offset() const
    {
        return offset_;
    }

    uint32_t line(uint32_t location) const
    {
        if (line_map_.empty()) {
            construct_line_map();
        }
        return line_map_.lower_bound(location - offset_)->second;
    }

    uint32_t column(uint32_t location) const
    {
        uint32_t line_start = location - offset_;
        while (line_start && content_.at(line_start - 1) != '\n'
               && content_.at(line_start - 1) != '\r')
        {
            --line_start;
        }
        return location - offset_ - line_start + 1;
    }

    virtual std::string name() const
    {
        return "";
    }

    virtual std::filesystem::path parent_path() const
    {
        return "";
    }

protected:
    Source(std::string_view content, uint32_t const offset)
        : content_(content)
        , offset_(offset)
    {}

    Source(std::string&& content, uint32_t const offset)
        : content_(content)
        , offset_(offset)
    {}

private:
    // Delete copy-constructor
    Source(const Source&) = delete;
    Source& operator=(const Source&) = delete;

    void construct_line_map() const
    {
        uint32_t line_number = 0u;
        for (uint32_t i = 0u; i < content_.length(); ++i) {
            if (content_.at(i) == '\n') {
                line_map_.emplace(i, ++line_number);
            }
        }
        line_map_.emplace(content_.length(), ++line_number);
    }

    std::string const content_;
    uint32_t const offset_;
    mutable std::map<uint32_t, uint32_t> line_map_;
};

/*! \brief File source. */
class File final : public Source {
public:
    static std::unique_ptr<File> open(
      std::filesystem::path const& file_path, uint32_t const offset)
    {
        std::optional<std::string> content = load_content(file_path);
        if (content) {
            return std::unique_ptr<File>(
              new File(file_path, std::move(content.value()), offset));
        }
        return nullptr;
    }

    std::string name() const override
    {
        return file_path_.filename().string();
    }

    std::filesystem::path parent_path() const override
    {
        return file_path_.parent_path();
    }

private:
    File(std::filesystem::path const& file_path, std::string&& content,
      uint32_t const offset)
        : Source(content, offset)
        , file_path_(file_path)
    {}

    // Delete copy-constructor
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    static std::optional<std::string> load_content(
      std::filesystem::path const& file_path)
    {
        std::string content;
        std::ifstream input_file(file_path);
        if (input_file.is_open()) {
            input_file.seekg(0, input_file.end);
            uint32_t length = input_file.tellg();
            input_file.seekg(0);
            content.resize(length, '\0');
            input_file.read(&content[0], length);
            input_file.close();
            return content;
        }
        return std::nullopt;
    }

    std::filesystem::path const file_path_;
};

} // namespace tweedledum
