/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _XXYZ_PHY_REG_
#define _XXYZ_PHY_REG_

#ifndef _MACROS_H_
#include "apex_macros.h"
#endif


///////////////////PHYSICAL REGISTER /////////////////////////////////
typedef struct physical_register_content
{
    int reg_valid;
    int reg_value;
    int zero_flag;
    int positive_flag;
    int architectural_register;
} physical_register_content;

typedef struct  physical_register_file
{
    physical_register_content physical_register[PHYSICAL_REGISTERS_SIZE+1];
}physical_register_file;

typedef struct free_physical_registers_queue
{
    int head;
    int tail;
    int free_physical_registers[PHYSICAL_REGISTERS_SIZE];
    int is_empty;
}free_physical_registers_queue;


/////////////////// REGISTER RENAME /////////////////////////////////
typedef struct rename_table_content
{
    int mapped_to_physical_register;
    int register_source;
}rename_table_content;

typedef struct rename_table_mapping 
{
    rename_table_content rename_table[ARCHITECTURAL_REGISTERS_SIZE+1];
}rename_table_mapping;

void print_prf_q(free_physical_registers_queue *a);
int pop_free_physical_registers(free_physical_registers_queue *fpq);
void push_free_physical_registers(free_physical_registers_queue *fpq, int physical_register);
#endif
