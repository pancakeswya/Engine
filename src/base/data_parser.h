#ifndef ENGINE_SRC_BASE_DATA_PARSER_H_
#define ENGINE_SRC_BASE_DATA_PARSER_H_

#include <string_view>
#include <utility>

#include "base/data_types.h"

namespace engine::DataParser {

extern std::pair<Data*, Status> FromFile(std::string_view path);

}  // namespace objv::DataParser

#endif  // SRC_BASE_DATA_PARSER_H_
