#pragma once

#include <unordered_map>
#include <string>
#include <boost/filesystem/path.hpp>

#include "cog/script.h"
#include "cog/verbs/table.h"
#include "cog/vm/value.h"
#include "cog/message_id.h"
#include "base/io/read_only_file.h"
#include "base/diagnostics/report.h"

namespace gorc {
namespace cog {

class compiler {
private:
    void add_message_id(const std::string& name, message_id value);
    void populate_tables();

    std::unordered_map<std::string, vm::value> ConstantTable;
    std::unordered_map<std::string, message_id> MessageTable;
    verbs::verb_table& verb_table;

public:
    compiler(verbs::verb_table& verbTable);

    void compile(const boost::filesystem::path& filename, io::read_only_file& file,
            script& output, diagnostics::report& report) const;
};

}
}
