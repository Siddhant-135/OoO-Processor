#pragma once
#include "Basics.h"
#include <sstream>
#include <fstream>
#include <optional>
#include <string_view>
#include <string>
#include <vector>

class Parser
{
    public:
    std::optional<OpCode> parseOperation(std::string_view first_token);
    void parseFile(std::ifstream& file, std::vector<Instruction>& inst_memory, std::vector<int>& memory);
    std::vector<std::pair<std::string_view, int>> inst_alias; // holds PC values for jumps in the instruction.
    std::vector<std::pair<std::string_view, int>> mem_alias; // holds memory array name to location mapping
    int getValue(std::vector<std::pair<std::string_view, int>>& aliastable, std::string_view name);
};
