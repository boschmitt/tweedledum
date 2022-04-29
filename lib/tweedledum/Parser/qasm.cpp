/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Utils/SourceManager.h"

#include "QASM/Parser.h"
#include <string_view>

namespace tweedledum::qasm {

Circuit parse_source_buffer(std::string_view buffer)
{
    Circuit circuit;
    SourceManager source_manager;
    source_manager.add_buffer(buffer);
    Parser parser(source_manager);
    parser.parse(circuit);
    return circuit;
}

Circuit parse_source_file(std::string_view path)
{
    Circuit circuit;
    SourceManager source_manager;
    source_manager.add_file(path);
    Parser parser(source_manager);
    parser.parse(circuit);
    return circuit;
}

} // namespace tweedledum::qasm
