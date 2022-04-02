/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _XXYZ_ISSUE_QUEUE_
#define _XXYZ_ISSUE_QUEUE_




#ifndef _MACROS_H_
#include "apex_macros.h"
#endif

////////////////////////ISSUE_QUEUE////////////////////////////////////

typedef struct issue_queue_entry
{
    int is_allocated;
    int FU;
    int src1_tag;
    int src1_value;
    int src1_valid;
    int src2_tag;
    int src2_value;
    int src2_valid;
    int immediate_literal;
    int dest_tag;
    int lsq_index;
    int rob_index;
    int counter;
    int opcode;
    int pc_value;
}issue_queue_entry;

typedef struct issue_queue_buffer
{
    issue_queue_entry issue_queue[ISSUE_QUEUE_SIZE];
}issue_queue_buffer;

void iq_entry_addition(issue_queue_buffer *iq,issue_queue_entry *iq_entry,int iq_index);
int issue_buffer_index_available(issue_queue_buffer *iq);
void print_iq_indexes(issue_queue_buffer *iq);
void print_iq_entries(issue_queue_buffer *iq);
int get_iq_index_fu(issue_queue_buffer *iq, int fu);
#endif
