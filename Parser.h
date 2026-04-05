#pragma once
#include "Basics.h"

class Parser {
public:
	Instruction lineToInst(std::string line);
};