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
int is_allocated;
int pc_value;
 //address of architectural register if any
int destination_address;
int physical_register;
//value or address
int result_value;
int store_value;
int store_value_valid;
//valid address or valid value in result value
int status_bit;
int insn_type;
int opcode;
//register to regster 0
//load 1
//store 2
//branch 3
int positive_flag;
int zero_flag;
}reorder_buffer_entry;

typedef struct reorder_buffer
{
    reorder_buffer_entry reorder_buffer_queue[ROB_SIZE];
    int head;
    int tail;
    int is_full;
}reorder_buffer;

int reorder_buffer_available(reorder_buffer *rob);
int reorder_buffer_entry_addition_to_queue(reorder_buffer *rob, reorder_buffer_entry * rob_entry);
void print_rob_entries(reorder_buffer *rob);
int is_rob_full(reorder_buffer *rob);
#endif
