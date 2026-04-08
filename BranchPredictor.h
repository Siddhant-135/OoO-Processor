#pragma once
#include "Basics.h"
#include <iostream>
#include <vector>

class BranchPredictor {
public:
    int total_branches = 0;
    int correct_predictions = 0;
    int state = 0; // 2-bit state: State 0- Strong taken. State 1- Weak taken. State 2- Weak not taken. State 3- Strong not taken.

    int predict(int current_pc, int imm, OpCode op) 
    {
        if(state==0 || state==1) return current_pc + imm; 
        else return current_pc + 1; 
    }

    void update(int pc, int actual_target, bool taken, bool was_correct) {
        total_branches++;
        if(was_correct) correct_predictions++;
        if(taken) state = std::max(0, state - 1); 
        else state = std::min(3, state + 1); 
    }
};