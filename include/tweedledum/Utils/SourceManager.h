/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Source.h"

#include <cstdint>
#include <filesystem>
#include <fmt/format.h>
#include <map>
#include <memory>
#include <vector>

namespace tweedledum {

/*! \brief This class handles loading source files into memory.
 *
 * NOTE: the first file added is always considered the main file.
 */
class SourceManager {
public:
    SourceManager() = default;

    Source const* main_source() const
    {
        if (sources_.empty()) {
            return nullptr;
        }
        return sources_.at(0).get();
    }

    Source const* add_file(std::filesystem::path const& file_path)
    {
        std::unique_ptr<File> file = File::open(file_path, next_offset_);
        if (file != nullptr) {
            Source const* file_ptr = file.get();
            next_offset_ += file->length() + 1;
            location_map_.emplace(next_offset_, sources_.size());
            sources_.emplace_back(std::move(file));
            return file_ptr;
        }
        return nullptr;
    }

    Source const* add_buffer(std::string_view buffer)
    {
        std::unique_ptr<Source> buf = Source::create(buffer, next_offset_);
        Source const* buf_ptr = buf.get();
        next_offset_ += buf->length() + 1;
        location_map_.emplace(next_offset_, sources_.size());
        sources_.emplace_back(std::move(buf));
        return buf_ptr;
    }

    std::string location_str(uint32_t const location) const
    {
        uint32_t const source_id = location_map_.lower_bound(location)->second;
        Source const* source = sources_.at(source_id).get();
        return fmt::format("<{}:{}:{}>", source->name(), source->line(location),
          source->column(location));
    }

private:
    // Delete copy-constructor
    SourceManager(SourceManager const&) = delete;
    SourceManager& operator=(SourceManager const&) = delete;

    std::vector<std::unique_ptr<Source>> sources_;
    std::map<uint32_t, uint32_t> location_map_;
    uint32_t next_offset_ = 0u;
};

} // namespace tweedledum
