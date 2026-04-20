#pragma once
#include "../Basics.h"
#include <vector>
#include <iostream>

struct RATEntry{
    //default mapping: validity true, hence Rob column -1
    bool valid = true;
    int ROBId = -1;
};

class RAT{
    //validity bit, index is inherent to vector, ROBentrynumber. (put in a struct)4
    private:
    std::vector <RATEntry> RAT_vector;

    public:
    //functions: modify ROB at index: addEntry
    RAT(int num_regs){
        RAT_vector.resize(num_regs);
    }
    void add_to_RAT (int idx, int ROBId);
    //removeEntry: return it to valid and -1
    void rem_from_RAT (int idx);
    //return the ROB entry corresponding to the index.
    int get_alias (int idx);
    //return whether the register is in a valid state at that time.
    bool reg_valid (int idx);
    void reset();
    void print() const;
    //NO NOTION OF FULLNESS AS A REG CAN ONLY BE ALIASED TO ONE OTHER AT A GIVEN TIME.
};
