#ifndef OBJ_PARSER_H_
#define OBJ_PARSER_H_

#include "obj/types.h"

#include <string>

namespace obj {

Data ParseFromFile(const std::string& path);

} // namespace obj

#endif // OBJ_PARSER_H_