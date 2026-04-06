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


Instruction Parser::parseFile(std::ifstream& file, std::vector<Instruction>& inst_memory)
{
    std::string line;
    int inst_number = 0;
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
            }
            else if (op==OpCode::SW) // Instruction type sw x5 A(x4)  to be interpreted as src1 = x5, imm = A, dest = x4. lw is different and separately handled in the lw case above.
            {
                std::string mem_loc;
                iss >> mem_loc;
                inst.src1 = findLocation(mem_alias);
                std::string reg;
                iss >> reg;
                inst.dest = findLocation(inst_alias);
            }
            else if (op==OpCode::BEQ || op==OpCode::BNE || op==OpCode::BLT || op==OpCode::BLE)
            {
                std::string reg1, reg2, label;
                iss >> reg1 >> reg2 >> label;
                inst.src1 = findLocation(inst_alias);
                inst.src2 = findLocation(inst_alias);
                inst.imm = findLocation(inst_alias);
            }
            else if (op==OpCode::J)
            {
                std::string label;
                iss >> label;
                inst.imm = findLocation(inst_alias);
            }
            else if (op==OpCode::ADDI || op==OpCode::SLTI || op==OpCode::ANDI || op==OpCode::ORI || op==OpCode::XORI)
            {
                std::string reg1, reg2;
                int imm;
                iss >> reg1 >> reg2 >> imm;
                inst.src1 = findLocation(inst_alias);
                inst.dest = findLocation(inst_alias);
                inst.imm = imm;
            }
            else
            {
                std::string reg1, reg2, reg3;
                iss >> reg1 >> reg2 >> reg3;
                inst.src1 = findLocation(inst_alias);
                inst.src2 = findLocation(inst_alias);
                inst.dest = findLocation(inst_alias);
            }
            inst_memory.push_back(inst);
            inst_number++;
        }
        else if (first_word[0]=='.' && first_word[first_word.length() - 1]==':') 
        {
            break;
        }
        else if (first_word[first_word.length() - 1]==':')
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


int Parser::findLocation(std::vector<std::pair<std::string_view, int>>& aliastable, std::string_view name)
{
    for (auto& pair : aliastable)
    {
        if (pair.first == name) return pair.second;
    }
    return -1; // not found
}