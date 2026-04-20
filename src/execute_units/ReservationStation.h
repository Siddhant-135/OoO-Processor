#pragma once
#include "../Basics.h"
#include <vector>
#include <optional>
#include <iostream>

class RS {
    protected:
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
    bool verbose = false;

    public:
    int PipelineCounter;
    virtual ~RS() = default;
    RS(int size, int pipeline_size, int stage_lat, bool verbose = false){
        this->size=size;
        RS_stage_vector.resize(size);
        for (int i = 0; i < size; i++) {
            RS_stage_vector[i].valid = false;
            RS_stage_vector[i].stage = -1;
        }
        this->pipeline_size=pipeline_size;
        this->stage_lat=stage_lat;
        this->counter=0;
        this->PipelineCounter=0;
        this->verbose = verbose;
    }  
    bool isFull();

    virtual void push(RSEntry temp);//an O(n) search.

    virtual int get_valid_entry(); //method to iterate linearly and find the ready entry: optional result? returns index of ready entry. -1 if none yet.

    int update_rs(); // update_rs(), returns index.
    virtual void reset();
    void print() const;

    virtual void invalidate_entry(int idx);

    void pushToPipeline(int idx); // pushes the ready entry to the pipeline. Called by the execution unit when it is ready to accept a new entry. Also updates the pipeline stage of the entry.

    RSEntry& get_entry(int idx){ return RS_stage_vector[idx].rs_entry; } // much better: single point access to everything

    void rs_capture(int tag, int value);

    virtual void update_mem_addr_val(){};
    
    virtual void ls_fwd(){};

    bool hasPendingWork(){
        for (int i = 0; i < size; i++) {
            if (RS_stage_vector[i].valid) {
                return true;
            }
        } return false;
    }
};
