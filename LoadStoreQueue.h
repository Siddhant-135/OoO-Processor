#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Basics.h"

class LoadStoreQueue {
public:
    // LSQ reservation station
    int latency;
    
    bool has_result = false; // result flag
    bool has_exception = false; // exception flag
    int store_data = 0;
    
    void lsq_capture (int tag, int val) {};
    void executeCycle(std::vector<int>& Memory) {};
};