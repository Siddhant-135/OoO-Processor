#pragma once
#include "ReservationStation.h"

// class LoadStoreQueue {
// public:
//     // LSQ reservation station
//     int latency;
    
//     bool has_result = false; // result flag
//     bool has_exception = false; // exception flag
//     int store_data = 0;
    
//     void lsq_capture (int tag, int val) {};
//     void executeCycle(std::vector<int>& Memory) {};
// };

class LoadStoreQueue: public RS{
private:
    /* data */
    int youngest_entry = 0  ;
    int oldest_entry = 1;

public:
    LoadStoreQueue(int size, int pipeline_size, int stage_lat): RS(size, pipeline_size, stage_lat) {};
    ~LoadStoreQueue();
    void push(RSEntry temp) override;
    int get_valid_entry() override;
        // void lsq_capture (int tag, int val) override;
        // void executeCycle(std::vector<int>& Memory);
};


LoadStoreQueue::~LoadStoreQueue()
{
}
