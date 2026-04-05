#include "Parser.h"

Instruction parsefile(std::ifstream file, std::vector<Instruction> inst_memory)
{
    std::string line;
    while (std::getline(file, line)) 
    {
        std::istringstream iss(line); // could be a tag, or a operation, or a comment hash, or a memory declaration
        std::string first_word;
        iss >> first_word;

        if (mapname.find(first_word))
        {

        }
        else if (first_word[0]=='.' && first_word[-1]==':') 
        {
            break;
        }
        else if (first_word[-1]==':')
        {
            break;
        }
        else{
            throw std::runtime_error("File is corrupted in its input");
            return;
        }
    }
}
    std::string line;
    while (std::getline(file, line)) 
    {
        if (line.empty() || line[0] == '#');
        Instruction inst = lineToInst (line);
        inst_memory.push_back(inst);
    }