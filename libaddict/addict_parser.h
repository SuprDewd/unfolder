#ifndef ADDICT_PARSER_H
#define ADDICT_PARSER_H

#include "../lib/server.h"
#include "reader.h"
#include "addict_dag.h"
#include <string>

class AddictParser {
private:
    std::pair<size_t,size_t> next_dict(Reader *s, const std::string &base_path);
    std::pair<size_t,size_t> next_expr(Reader *s, const std::string &base_path);
    AddictDAG *dag;

public:
    AddictParser(AddictDAG *dag);
    std::pair<size_t,size_t> parse_string(const std::string &s, const std::string &base_path = ".");
    std::pair<size_t,size_t> parse_file(std::string path, std::string base_path = ".");
};

#endif
