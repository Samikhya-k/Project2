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

#ifndef _MACROS_H_
#include "apex_macros.h"
#endif

#ifndef _XXYZ_ISSUE_QUEUE_
#include "issue_queue.h"
#endif

#ifndef _XXYZ_REORDER_BUFFER_
#include "rob.h"
#endif

#ifndef _XXYZ_LOAD_STORE_QUEUE_
#include "lsq.h"
#endif

#ifndef _XXYZ_PHY_REG_
#include "physical_register.h"
#endif

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
    int phy_rs1;
    int phy_rs2;
    int rs2;
    int rd;
    int phy_rd; //physical register allocated from free physical list  
    int imm;
    int rs1_ready;
    int rs2_ready;
    int rs1_value;
    int rs2_value;
    int result_buffer;
    int memory_address;
    int has_insn;
    int is_physical_register_required;
    int is_src1_register_required;
    int is_src2_register_required;
    int is_memory_insn;
    int is_stage_stalled;
    int issue_queue_index;
    int memory_instruction_type;
    int fu;
    int rob_index;
    int lsq_index;


    load_store_queue_entry temp_lsq_entry;
    reorder_buffer_entry temp_rob_entry;
    issue_queue_entry temp_iq_entry;
    
} CPU_Stage;

////////ARCHECTURAL_REGISTER_FILE///////////////

typedef struct  architectural_register_content{
    int value;
    int zero_flag;
    int positive_flag;
} architectural_register_content;

typedef struct archictectural_register_file{
    architectural_register_content architectural_register_file[ARCHITECTURAL_REGISTERS_SIZE];
}archictectural_register_file;




/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction *code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int positive_flag;
    int fetch_from_next_cycle;

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode_rename;
    CPU_Stage rename_dispatch;
    CPU_Stage queue_entry;
    CPU_Stage int_fu;
    CPU_Stage mul1_fu;
    CPU_Stage mul2_fu;
    CPU_Stage mul3_fu;
    CPU_Stage mul4_fu;

    CPU_Stage branch_fu;
    CPU_Stage memory;
    CPU_Stage add_fwd;
    CPU_Stage mul_fwd;
    CPU_Stage memory_fwd;
    CPU_Stage writeback;
    
    physical_register_file prf;
    archictectural_register_file arf;
    free_physical_registers_queue free_prf_q;
    rename_table_mapping rnt;
    issue_queue_buffer iq;
    load_store_queue lsq;
    reorder_buffer rob;


} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
void push_information_to_fu(APEX_CPU *cpu, int index, int fu);
#endif
