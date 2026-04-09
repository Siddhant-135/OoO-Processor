#pragma once
#include "Basics.h"
#include <vector>
#include <optional>

class RS {
    private:
    struct RS_triple{
        bool valid;
        RSEntry rs_entry;
        int stage;
    };

    std::vector <RS_triple> RS_stage_vector;// bool for validity, RSEntry for the entry, int for pipeline stage.
    int size;
    int pipeline_size;
    int counter;
    int stage_lat;

    public:
    RS(int size, int pipeline_size, int stage_lat){
        this->size=size;
        RS_stage_vector.resize(size);
        for (int i = 0; i < size; i++) {
            RS_stage_vector[i].valid = false;
            RS_stage_vector[i].stage = -1;
        }
        this->pipeline_size=pipeline_size;
        this->stage_lat=stage_lat;
        this->counter=0;
    }

    bool isFull();

    void push(RSEntry temp);//an O(n) search.

    int get_valid_entry(); //method to iterate linearly and find the ready entry: optional result? returns index of ready entry. -1 if none yet. Also puts the ready rnetry in pipleine.

    int update_rs(); // update_rs(), returns index.

    void invalidate_entry(int idx);

    RSEntry& get_entry(int idx){ return RS_stage_vector[idx].rs_entry; } // much better: single point access to everything

    void rs_capture(int tag, int value);
    bool hasPendingWork(){
        for (int i = 0; i < size; i++) {
            if (RS_stage_vector[i].valid) {
                return true;
            }
        } return false;
    }
};
