/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int imm;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rd;
    int imm;
    int rs1_value;
    int rs2_value;
    int result_buffer;
    int memory_address;
    int has_insn;
} CPU_Stage;

///////////////////PHYSICAL REGISTER /////////////////////////////////
typedef struct physical_register_content
{
    int reg_valid;
    int reg_value;
} physical_register_content;

typedef struct  physical_register_file
{
    physical_register_content physical_register[PHYSICAL_REGISTERS_SIZE];
}physical_register_file;

typedef struct free_physical_registers_queue
{
    int head;
    int tail;
    int free_physical_registers[PHYSICAL_REGISTERS_SIZE];
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
}issue_queue_entry;

typedef struct issue_queue_buffer
{
    issue_queue_entry issue_queue[ISSUE_QUEUE_SIZE];
}issue_queue_buffer;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int regs[REG_FILE_SIZE];       /* Integer register file */
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction *code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int positive_flag;
    int fetch_from_next_cycle;

    
    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage decode_rename;
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage writeback;

    free_physical_registers_queue free_prf_q;
    rename_table_mapping rnt;
    issue_queue_buffer iq;



} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);

void print_prf_q(free_physical_registers_queue *a);
int issue_buffer_available_index(issue_queue_buffer *iq);
#endif
