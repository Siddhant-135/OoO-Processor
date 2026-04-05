#pragma once
#include "Basics.h"
#include <sstream>
#include <fstream>

std::pair<std::string, int> memory[]
Instruction parsefile(std::ifstream& file, std::vector<Instruction>& inst_memory);