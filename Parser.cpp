#include "Parser.h"
#include <optional>

std::optional<OpCode> Parser::parseOperation(std::string_view first_token)
{
    if (first_token == "add") return OpCode::ADD;
    else if (first_token == "sub") return OpCode::SUB;
    else if (first_token == "addi") return OpCode::ADDI;
    else if (first_token == "mul") return OpCode::MUL;
    else if (first_token == "div") return OpCode::DIV;
    else if (first_token == "rem") return OpCode::REM;
    else if (first_token == "lw") return OpCode::LW;
    else if (first_token == "sw") return OpCode::SW;
    else if (first_token == "beq") return OpCode::BEQ;
    else if (first_token == "bne") return OpCode::BNE;
    else if (first_token == "blt") return OpCode::BLT;
    else if (first_token == "ble") return OpCode::BLE;
    else if (first_token == "j") return OpCode::J;
    else if (first_token == "slt") return OpCode::SLT;
    else if (first_token == "slti") return OpCode::SLTI;
    else if (first_token == "and") return OpCode::AND;
    else if (first_token == "or") return OpCode::OR;
    else if (first_token == "xor") return OpCode::XOR;
    else if (first_token == "andi") return OpCode::ANDI;
    else if (first_token == "ori") return OpCode::ORI;
    else if (first_token == "xori") return OpCode::XORI;
    else return std::nullopt;
};


Instruction Parser::parseFile(std::ifstream& file, std::vector<Instruction>& inst_memory, std::vector<int>& memory)
{
    std::string line;
    int inst_number = 0;
    int mem_location = 0;
    while (std::getline(file, line)) 
    {
        std::istringstream iss(line); // could be a tag, or a operation, or a comment hash, or a memory declaration
        std::string first_word;
        iss >> first_word;

        if (parseOperation(first_word).has_value())
        {
            OpCode op = parseOperation(first_word).value();
            Instruction inst;
            inst.op = op;
            inst.pc = inst_number;

            if(op==OpCode::LW) // Instruction type lw x5 A(x4)  to be interpreted as src1 = x4, imm = A, dest = x5. sw is different and separately handled in the sw case below. 
            {
                std::string dest, source;
                iss >> dest >> source;
                int dest_reg = std::stoi(dest.substr(1)); // works for double digit as well. note that dest[1] wouldn't work coz thats a pointer
                inst.dest = dest_reg;
                auto l = source.find('(');
                auto r = source.find(')');
                std::string mem_loc = source.substr(0, l);
                std::string reg = source.substr(l+1, r-l-1); // syntax is start, length not start, end.
                inst.src1 = std::stoi(reg.substr(1));
                inst.imm = Parser::getValue(mem_alias, mem_loc); // Implementation: store memory[imm + value from src1] into dest.
            }
            else if (op==OpCode::SW) // Instruction type sw x5 A(x4)  to be interpreted as src1 = x5, imm = A, dest = x4. lw is different and separately handled in the lw case above.
            {
                std::string source, dest;
                iss >> source >> dest;
                int src_reg = std::stoi(source.substr(1)); 
                inst.src1 = src_reg; // value from src1 will be stored into memory[imm + value from dest]
                auto l = dest.find('(');
                auto r = dest.find(')');
                std::string mem_loc = dest.substr(0, l);
                std::string reg = dest.substr(l+1, r-l-1);
                inst.dest = std::stoi(reg.substr(1));
                inst.imm = Parser::getValue(mem_alias, mem_loc);
            }
            else if (op==OpCode::BEQ || op==OpCode::BNE || op==OpCode::BLT || op==OpCode::BLE)
            {
                std::string src1, src2, label;
                iss >> src1 >> src2 >> label;
                inst.src1 = std::stoi(src1.substr(1));
                inst.src2 = std::stoi(src2.substr(1));
                inst.imm = Parser::getValue(inst_alias, label); // Implementation: if condition holds on src1 and src2, otherwise pc++ as normal. 
            }
            else if (op==OpCode::J)
            {
                std::string label;
                iss >> label;
                inst.imm = Parser::getValue(inst_alias, label); // Implementation: set pc = imm. 
            }
            else if (op==OpCode::ADDI || op==OpCode::SLTI || op==OpCode::ANDI || op==OpCode::ORI || op==OpCode::XORI)
            {
                std::string dest, source, imm;
                iss >> dest >> source >> imm;
                int dest_reg = std::stoi(dest.substr(1)); 
                inst.dest = dest_reg;
                inst.src1 = std::stoi(source.substr(1));
                inst.imm = std::stoi(imm); // Implementation: perform the operation on value from src1 and imm, store into dest.
            }
            else
            {
                // Oh no.
            }
            inst_memory.push_back(inst);
            inst_number++;
        }

        else if (first_word[0]=='.' && first_word[first_word.length() - 1]==':') // Memory declaration of the form .A: 1 2 4 6, to be interpreted as wherever A starts, there is 1, then 2 then 4. Location of A itself is determined by addition assuming all memory contiguous.
        {
            std::string mem_name = first_word.substr(1, first_word.length()-2); // remove the dot and the colon 
            mem_alias.push_back({mem_name, mem_location});
            int val;
            while (iss >> val)
            {
                memory.push_back(val);
                mem_location++;
            }
            break;
        }

        else if (first_word[first_word.length() - 1]==':')
        {
            std::string label_name = first_word.substr(0, first_word.length()-1);
            inst_alias.push_back({label_name, (inst_number+1)}); // label points to the instruction after it so + 1. Everything is 0 indexed btw.
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


int Parser::findLocation(std::vector<std::pair<std::string_view, int>>& aliastable, std::string_view name)
{
    for (auto& pair : aliastable)
    {
        if (pair.first == name) return pair.second;
    }
    return -1; // not found
}