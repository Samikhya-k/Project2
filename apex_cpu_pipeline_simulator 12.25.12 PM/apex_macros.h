/*
 * apex_macros.h
 * Contains APEX cpu pipeline macros
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _MACROS_H_
#define _MACROS_H_

#define FALSE 0x0
#define TRUE 0x1

/* Integers */
#define DATA_MEMORY_SIZE 4096
#define PHYSICAL_REGISTERS_SIZE 20
#define ARCHITECTURAL_REGISTERS_SIZE 16
#define ISSUE_QUEUE_SIZE 8
#define LSQ_SIZE 6
#define ROB_SIZE 16

#define SOURCE_AR 0
#define SOURCE_PR 1

#define LOAD_INS 0
#define STORE_INS 1

#define ADD_FU 0
#define MUL_FU 1
#define MEM_FU 2
#define BRANCH_FU 3


/* Numeric OPCODE identifiers for instructions */
#define OPCODE_ADD 0x0
#define OPCODE_SUB 0x1
#define OPCODE_MUL 0x2
#define OPCODE_DIV 0x3
#define OPCODE_AND 0x4
#define OPCODE_OR 0x5
#define OPCODE_XOR 0x6
#define OPCODE_MOVC 0x7
#define OPCODE_LOAD 0x8
#define OPCODE_STORE 0x9
#define OPCODE_ADDL 0x11
#define OPCODE_SUBL 0x12
#define OPCODE_JUMP 0x13
#define OPCODE_JALR 0x17
#define OPCODE_CMP 0x14
#define OPCODE_BZ 0xa
#define OPCODE_BNZ 0xb
#define OPCODE_HALT 0xc
#define OPCODE_BP 0xd
#define OPCODE_BNP 0xe
#define OPCODE_RET 0xf

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/* Set this flag to 1 to enable cycle single-step mode */
#define ENABLE_SINGLE_STEP 1

#endif
