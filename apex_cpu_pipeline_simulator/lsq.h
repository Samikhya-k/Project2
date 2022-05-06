/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _XXYZ_LOAD_STORE_QUEUE_
#define _XXYZ_LOAD_STORE_QUEUE_

#ifndef _MACROS_H_
#include "apex_macros.h"
#endif

////////////////////////LOAD_STORE_QUEUE////////////////////////////////////

typedef struct load_store_queue_entry
{
    int mem_address;
    int address_valid;
    int allocate;
    int instruction_type;
    int destination_address_for_load;
    int phy_destination_address_for_load;
    int data_ready;
    int src1_store;
    int value_to_be_stored;
    int rob_index;
    int OPCODE;
    int pc_value;
}load_store_queue_entry;

typedef struct load_store_queue
{
    load_store_queue_entry load_store_queue[LSQ_SIZE];
    int head;
    int tail;
    int is_full;
}load_store_queue;

int lsq_index_available(load_store_queue *lsq);
int lsq_entry_addition_to_queue(load_store_queue *lsq, load_store_queue_entry * lsq_entry);
void print_lsq_entries(load_store_queue *lsq);
#endif
