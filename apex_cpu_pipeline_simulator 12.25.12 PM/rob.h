/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _XXYZ_REORDER_BUFFER_
#define _XXYZ_REORDER_BUFFER_


#ifndef _MACROS_H_
#include "apex_macros.h"
#endif

////////////////////////REORDER_BUFFER////////////////////////////////////

typedef struct reorder_buffer_entry
{
int pc_value;
int destination_address;
//value or address
int result_value;
int store_value;
int store_value_valid;
//valid address or valid value in result value
int status_bit;
int insn_type;
//register to regster 0
//load 1
//store 2
//branch 3
}reorder_buffer_entry;

typedef struct reorder_buffer
{
    reorder_buffer_entry reorder_buffer_queue[ROB_SIZE];
    int head;
    int tail;
    int is_full;
}reorder_buffer;

int reorder_buffer_available(reorder_buffer *rob);
void reorder_buffer_entry_addition_to_queue(reorder_buffer *rob, reorder_buffer_entry * rob_entry);
void print_rob_entries(reorder_buffer *rob);
#endif
