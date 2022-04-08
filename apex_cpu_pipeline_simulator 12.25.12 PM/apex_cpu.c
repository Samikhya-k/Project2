/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apex_cpu.h"
#include "apex_macros.h"
#include "physical_register.h"
#include  "issue_queue.h"

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}
static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_ADDL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }
        case OPCODE_SUBL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        
        case OPCODE_LOAD:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs2, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_BP:
        case OPCODE_BNP:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }
        case OPCODE_JUMP:
        {
            printf("%s,R%d ", stage->opcode_str, stage->rs1);
            break;
        }
        case OPCODE_JALR:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
        }
        case OPCODE_RET:
        {
            printf("%s,R%d", stage->opcode_str, stage->rs1);
            break;
        }
        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }
        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i,ph;

    printf("----------\n%s\n----------\n", "ARCHITECTURAL Registers:");

    for (int i = 0; i < ARCHITECTURAL_REGISTERS_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->arf.architectural_register_file[i].value);
    }

    printf("\n");

    for (i = (ARCHITECTURAL_REGISTERS_SIZE / 2); i < ARCHITECTURAL_REGISTERS_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i,cpu->arf.architectural_register_file[i].value);
    }

    printf("\n");

    printf("----------\n%s\n----------\n", "PHYSICAL Registers:");

    for (int ph = 0; ph < PHYSICAL_REGISTERS_SIZE / 2; ++ph)
    {
        printf("P%-3d[%-3d] ", ph, cpu->prf.physical_register[ph].reg_value);
    }

    printf("\n");

    for (ph = (PHYSICAL_REGISTERS_SIZE / 2); ph < PHYSICAL_REGISTERS_SIZE; ++ph)
    {
        printf("P%-3d[%-3d] ", ph,cpu->prf.physical_register[ph].reg_value);
    }

    printf("\n");

}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.imm = current_ins->imm;

        /* Update PC for next instruction */
        cpu->pc += 4;

        /* Copy data from fetch latch to decode latch*/
        cpu->decode_rename = cpu->fetch;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
}
//// first ROB---> LSQ--->Issue_queue/////
/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode_rename(APEX_CPU *cpu)
{
    if (cpu->decode_rename.has_insn)
    {
        cpu->decode_rename.is_physical_register_required=0;
        cpu->decode_rename.is_src1_register_required=0;
        cpu->decode_rename.is_src2_register_required=0;
        cpu->decode_rename.is_memory_insn=0;

        /* Read operands from register file based on the instruction type */
        switch (cpu->decode_rename.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {
                cpu->decode_rename.is_physical_register_required=1;
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.is_src2_register_required=1;
                cpu->decode_rename.fu=INT_FU;  
                if(cpu->decode_rename.opcode==OPCODE_MUL || cpu->decode_rename.opcode==OPCODE_DIV){
                    cpu->decode_rename.fu=MUL_FU;      
                }
                break;
            }

            case OPCODE_ADDL:
            {
                cpu->decode_rename.is_physical_register_required=1;
                cpu->decode_rename.is_src1_register_required=1;  
                cpu->decode_rename.fu=INT_FU;        
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->decode_rename.is_physical_register_required=1;
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.fu=INT_FU;
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->decode_rename.is_physical_register_required=1;
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.is_memory_insn=1;
                cpu->decode_rename.fu=INT_FU;
                cpu->decode_rename.memory_instruction_type=LOAD_INS;
                break;
            }
            case OPCODE_STORE:
            {
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.is_src2_register_required=1;
                cpu->decode_rename.is_memory_insn=1;
                cpu->decode_rename.fu=INT_FU;
                cpu->decode_rename.memory_instruction_type=STORE_INS;
                break;
            }
            case OPCODE_CMP:
            {
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.is_src2_register_required=1;
                break;
            }
             case OPCODE_JUMP:
            {
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.fu=BRANCH_FU;
                break;
            }
            case OPCODE_JALR:
            {
                cpu->decode_rename.is_physical_register_required=1;
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.fu=BRANCH_FU;
                break;
            }

            case OPCODE_MOVC:
            {
                cpu->decode_rename.is_physical_register_required=1;
                cpu->decode_rename.fu=INT_FU;

                break;
            }
            case OPCODE_RET:
            {
                cpu->decode_rename.is_src1_register_required=1;

                break;
            }
        }
        //checking the resources (availabilty of free physical register, iq entry and lsq entry)
        cpu->decode_rename.phy_rd=100;//default value is set to 100 for phy_rd
        if(cpu->decode_rename.is_physical_register_required){
            int temp_rd=pop_free_physical_registers(&cpu->free_prf_q);
            if( temp_rd!= -1){
                cpu->decode_rename.phy_rd =temp_rd;
                cpu->rnt.rename_table[cpu->decode_rename.rd].mapped_to_physical_register=temp_rd;
                cpu->rnt.rename_table[cpu->decode_rename.rd].register_source=1;
            }
            else  //setting the stalling variable to 1 if no free physical register available
                cpu->decode_rename.is_stage_stalled=1;
        }
    
        cpu->decode_rename.rs1_ready=1;
        cpu->decode_rename.rs2_ready=1;
        if(cpu->decode_rename.is_src1_register_required){
            int temp_physcial_src1=100;
            //if need to reaad the content from physical register
            if(cpu->rnt.rename_table[cpu->decode_rename.rs1].register_source){
                temp_physcial_src1=cpu->rnt.rename_table[cpu->decode_rename.rs1].mapped_to_physical_register;
                //if physical register content is valid then read the value 
                if(cpu->prf.physical_register[temp_physcial_src1].reg_valid){
                    cpu->decode_rename.rs1_value=cpu->prf.physical_register[cpu->decode_rename.rs1].reg_value;
                    cpu->decode_rename.phy_rs1=temp_physcial_src1;
                    cpu->decode_rename.rs1_ready=1;
                }
                //if physical register content is invalid  then read the prf  from which it  need to be read  
                else{
                    cpu->decode_rename.phy_rs1=temp_physcial_src1;
                    cpu->decode_rename.rs1_ready=0;
                }
            }
            //read from architectural register 
            else{
                 cpu->decode_rename.rs1_value= cpu->arf.architectural_register_file[cpu->decode_rename.rs1].value;
                 cpu->decode_rename.rs1_ready=1;
            }
        }
        if(cpu->decode_rename.is_src2_register_required){
            int temp_physcial_src2=100;
            if(cpu->rnt.rename_table[cpu->decode_rename.rs2].register_source){
                temp_physcial_src2=cpu->rnt.rename_table[cpu->decode_rename.rs2].mapped_to_physical_register;
                if(cpu->prf.physical_register[temp_physcial_src2].reg_valid){
                    cpu->decode_rename.rs2_value=cpu->prf.physical_register[cpu->decode_rename.rs2].reg_value;
                    cpu->decode_rename.phy_rs2=temp_physcial_src2;
                    cpu->decode_rename.rs2_ready=1;
                }
                else{
                    cpu->decode_rename.phy_rs2=temp_physcial_src2;
                    cpu->decode_rename.rs2_ready=0;
                }
            }
            else{
                 cpu->decode_rename.rs2_value= cpu->arf.architectural_register_file[cpu->decode_rename.rs2].value;
                 cpu->decode_rename.rs2_ready=1;
            }
        }



        // int temp_iq_index=issue_buffer_available_index(&cpu->iq);
        // if(temp_iq_index!=-1)
        //     cpu->decode_rename.issue_queue_index=temp_iq_index;
        // else
        //     cpu->decode_rename.is_stage_stalled=1;
        
        // //check rob availabolity

        // //check lsq availability if it is memry instruction






        /* Copy data from decode latch to execute latch*/
        cpu->rename_dispatch = cpu->decode_rename;
        cpu->decode_rename.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode_Rename", &cpu->decode_rename);
        }
    }
}

static void
APEX_rename_dispatch(APEX_CPU *cpu)
{
    //check availabity of iq_entry
    if(cpu->rename_dispatch.has_insn){
        int temp_iq_index=issue_buffer_index_available(&cpu->iq);
        printf("%d",temp_iq_index);
        int temp_lsq_index=100;
        if(cpu->rename_dispatch.is_memory_insn){
            temp_lsq_index=lsq_index_available(&cpu->lsq);
        }
        int temp_rob_index=reorder_buffer_available(&cpu->rob);
        if(temp_iq_index!=-1 && temp_lsq_index!=-1 && temp_rob_index!=-1){
           //temp lsq entry , rob entry and iq entry are available
            cpu->rename_dispatch.temp_iq_entry.dest_tag=cpu->rename_dispatch.phy_rd;
            cpu->rename_dispatch.temp_iq_entry.src1_tag=cpu->rename_dispatch.rs1;
            cpu->rename_dispatch.temp_iq_entry.src2_tag=cpu->rename_dispatch.rs2;
            cpu->rename_dispatch.temp_iq_entry.src1_valid=cpu->rename_dispatch.rs1_ready;
            cpu->rename_dispatch.temp_iq_entry.src2_valid=cpu->rename_dispatch.rs2_ready;
            cpu->rename_dispatch.temp_iq_entry.src1_value=cpu->rename_dispatch.rs1_value;
            cpu->rename_dispatch.temp_iq_entry.src2_value=cpu->rename_dispatch.rs2_value;
            cpu->rename_dispatch.temp_iq_entry.FU=cpu->rename_dispatch.fu;
            cpu->rename_dispatch.temp_iq_entry.immediate_literal=cpu->rename_dispatch.imm;
            cpu->rename_dispatch.temp_iq_entry.is_allocated=1;
            cpu->rename_dispatch.temp_iq_entry.rob_index=temp_rob_index;
            cpu->rename_dispatch.temp_iq_entry.lsq_index=temp_lsq_index;
            cpu->rename_dispatch.temp_iq_entry.opcode=cpu->rename_dispatch.opcode;
            cpu->rename_dispatch.temp_iq_entry.pc_value=cpu->rename_dispatch.pc;
            cpu->rename_dispatch.temp_iq_entry.counter=0;
            cpu->rename_dispatch.issue_queue_index=temp_iq_index;

            if(temp_lsq_index!=100 && temp_lsq_index != -1){
                cpu->rename_dispatch.temp_lsq_entry.allocate=1;
                cpu->rename_dispatch.temp_lsq_entry.instruction_type=cpu->rename_dispatch.memory_instruction_type;
                cpu->rename_dispatch.temp_lsq_entry.address_valid=0;
                cpu->rename_dispatch.temp_lsq_entry.data_ready=0;
                cpu->rename_dispatch.temp_lsq_entry.OPCODE=cpu->rename_dispatch.opcode;
            }
            cpu->rename_dispatch.temp_rob_entry.insn_type=cpu->rename_dispatch.fu;
            //check the pc value later
            cpu->rename_dispatch.temp_rob_entry.pc_value=cpu->rename_dispatch.pc;
            cpu->rename_dispatch.temp_rob_entry.destination_address=cpu->rename_dispatch.rd;
            cpu->rename_dispatch.temp_rob_entry.status_bit=0;
            cpu->rename_dispatch.temp_rob_entry.store_value_valid=0;
            //
        }
        else{
            cpu->decode_rename.is_stage_stalled=1;
        } 
        print_iq_indexes(&cpu->iq);

    cpu->queue_entry=cpu->rename_dispatch;
    cpu->rename_dispatch.has_insn = FALSE;
    if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Rename_Dispatch", &cpu->rename_dispatch);
        }
    }
}


static void APEX_queue_entry_addition(APEX_CPU *cpu)
{
    int rob_index,lsq_index;
    lsq_index=100;
    if(cpu->queue_entry.has_insn)
    {
        rob_index= reorder_buffer_entry_addition_to_queue(&cpu->rob,&cpu->queue_entry.temp_rob_entry);
        if(cpu->queue_entry.is_memory_insn){
            cpu->queue_entry.temp_lsq_entry.rob_index=rob_index;
            lsq_index=lsq_entry_addition_to_queue(&cpu->lsq,&cpu->queue_entry.temp_lsq_entry);
            print_lsq_entries(&cpu->lsq);
        }
        cpu->queue_entry.temp_iq_entry.rob_index=rob_index;
        cpu->queue_entry.temp_iq_entry.lsq_index=lsq_index;
        iq_entry_addition(&cpu->iq,&cpu->queue_entry.temp_iq_entry,cpu->queue_entry.issue_queue_index);

        print_iq_entries(&cpu->iq);
        //print_rob_entries(&cpu->rob);
    }
    cpu->process_iq=cpu->queue_entry;
    
    //printf("%d",cpu->int_fu.imm);
    cpu->queue_entry.has_insn = FALSE;
}

void push_information_to_fu(APEX_CPU *cpu, int index, int fu){
    switch (fu)
    {
    //addition fu
    case INT_FU:
        cpu->int_fu.rs1_value=cpu->iq.issue_queue[index].src1_value;
        cpu->int_fu.rs2_value=cpu->iq.issue_queue[index].src2_value;
        cpu->int_fu.phy_rd=cpu->iq.issue_queue[index].dest_tag;
        cpu->int_fu.rob_index=cpu->iq.issue_queue[index].rob_index;
        cpu->int_fu.lsq_index=cpu->iq.issue_queue[index].rob_index;
        cpu->int_fu.opcode=cpu->iq.issue_queue[index].opcode;
        cpu->int_fu.has_insn=1;
        cpu->int_fu.imm=cpu->iq.issue_queue[index].immediate_literal;
        cpu->iq.issue_queue[index].is_allocated=0;
        printf("in push");
        break;
    //multiplication fu
    case MUL_FU:
        cpu->mul1_fu.rs1_value=cpu->iq.issue_queue[index].src1_value;
        cpu->mul1_fu.rs2_value=cpu->iq.issue_queue[index].src2_value;
        cpu->mul1_fu.phy_rd=cpu->iq.issue_queue[index].dest_tag;
        cpu->mul1_fu.rob_index=cpu->iq.issue_queue[index].rob_index;
        cpu->mul1_fu.lsq_index=cpu->iq.issue_queue[index].lsq_index;
        cpu->mul1_fu.opcode=cpu->iq.issue_queue[index].opcode;
        cpu->mul1_fu.has_insn=1;
        cpu->iq.issue_queue[index].is_allocated=0;
        break;
    //branch fu
    case BRANCH_FU:
        cpu->branch_fu.rs1_value=cpu->iq.issue_queue[index].src1_value;
        cpu->branch_fu.rs2_value=cpu->iq.issue_queue[index].src2_value;
        cpu->branch_fu.phy_rd=cpu->iq.issue_queue[index].dest_tag;
        cpu->branch_fu.rob_index=cpu->iq.issue_queue[index].rob_index;
        cpu->branch_fu.lsq_index=cpu->iq.issue_queue[index].lsq_index;
        cpu->branch_fu.opcode=cpu->iq.issue_queue[index].opcode;
        cpu->branch_fu.has_insn=1;
        cpu->iq.issue_queue[index].is_allocated=0;
        break;
    default:
        break;
    }
}

void APEX_int_fu(APEX_CPU *cpu){
    if(cpu->int_fu.has_insn){
        switch (cpu->int_fu.opcode)
        {
        case OPCODE_ADD:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value+cpu->int_fu.rs2_value;
            break;
        case OPCODE_ADDL:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value+cpu->int_fu.imm;
            break;
        case OPCODE_SUB:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value-cpu->int_fu.rs2_value;
            break;
        case OPCODE_SUBL:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value-cpu->int_fu.imm;
            break; 
        case OPCODE_AND:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value & cpu->int_fu.rs2_value;
            break;
        case OPCODE_OR:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value | cpu->int_fu.rs2_value;
            break;
        case OPCODE_XOR:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value ^ cpu->int_fu.rs2_value;
            break;   
        case OPCODE_MOVC:
            cpu->int_fu.result_buffer=cpu->int_fu.imm ;
            break; 
        case OPCODE_LOAD:
            cpu->int_fu.memory_address=cpu->int_fu.rs1_value+cpu->int_fu.imm;
            cpu->lsq.load_store_queue[cpu->int_fu.lsq_index].destination_address_for_load=cpu->int_fu.memory_address;
            cpu->lsq.load_store_queue[cpu->int_fu.lsq_index].address_valid=1;
            break;
        case OPCODE_STORE:
            cpu->int_fu.memory_address=cpu->int_fu.rs1_value+cpu->int_fu.imm;
            cpu->lsq.load_store_queue[cpu->int_fu.lsq_index].mem_address=cpu->int_fu.memory_address;
            cpu->lsq.load_store_queue[cpu->int_fu.lsq_index].address_valid=1;
            break;
        default:
            break;
        }
        cpu->int_fwd=cpu->int_fu;
        cpu->int_fu.has_insn=FALSE;
    }
}

void APEX_int_fwd(APEX_CPU *cpu){
    if(cpu->int_fwd.has_insn){
        for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
            if(!cpu->iq.issue_queue[i].src1_valid){
                if(cpu->iq.issue_queue[i].src1_tag==cpu->int_fwd.phy_rd){
                    cpu->iq.issue_queue[i].src1_value=cpu->int_fwd.result_buffer;
                    cpu->iq.issue_queue[i].src1_valid=1;
                }
            }
            if(!cpu->iq.issue_queue[i].src2_valid){
                if(cpu->iq.issue_queue[i].src2_tag==cpu->int_fwd.phy_rd){
                    cpu->iq.issue_queue[i].src2_value=cpu->int_fwd.result_buffer;
                    cpu->iq.issue_queue[i].src2_valid=1;
                }
            }
        }
    }
    cpu->phy_writeback=cpu->int_fwd;
    cpu->int_fwd.has_insn=FALSE;
}

void APEX_phy_writeback(APEX_CPU *cpu){
    if(cpu->phy_writeback.has_insn){
        cpu->prf.physical_register[cpu->phy_writeback.phy_rd].reg_value=cpu->phy_writeback.result_buffer;
    }
    cpu->phy_writeback.has_insn=FALSE;
}

void APEX_mul_fu_1(APEX_CPU *cpu){
    if(cpu->mul1_fu.has_insn){
        cpu->mul2_fu=cpu->mul1_fu;
        cpu->mul1_fu.has_insn=FALSE;
    }
}

void APEX_mul_fu_2(APEX_CPU *cpu){
    if(cpu->mul2_fu.has_insn){
        cpu->mul3_fu=cpu->mul2_fu;
        cpu->mul2_fu.has_insn=FALSE;
    }
}

void APEX_mul_fu_3(APEX_CPU *cpu){
    if(cpu->mul3_fu.has_insn){
        cpu->mul4_fu=cpu->mul3_fu;
        cpu->mul3_fu.has_insn=FALSE;
    }
}


void APEX_mul_fu_4(APEX_CPU *cpu){
    if(cpu->mul4_fu.has_insn){
        if(cpu->mul4_fu.opcode==OPCODE_MUL)
            cpu->mul4_fu.result_buffer=cpu->mul4_fu.rs1_value*cpu->mul4_fu.rs1_value;
        else
            cpu->mul4_fu.result_buffer=cpu->mul4_fu.rs1_value/cpu->mul4_fu.rs1_value;
        cpu->mul_fwd=cpu->mul4_fu;
        cpu->mul3_fu.has_insn=FALSE;
    }
}

//identify iq index and push information 
void APEX_process_iq(APEX_CPU *cpu){
    if(cpu->process_iq.has_insn){
        int int_iq_index=-1;
        int mul_iq_index=-1;
        int bu_iq_index=-1;
        if(cpu->process_iq.fu==0)
            int_iq_index = get_iq_index_fu(&cpu->iq, 0);//0 for add
        if(cpu->process_iq.fu==1)
            mul_iq_index = get_iq_index_fu(&cpu->iq, 1);//1  for mul
        if(cpu->process_iq.fu==2)
            bu_iq_index  = get_iq_index_fu(&cpu->iq, 2);//2 for mem
        if(int_iq_index>=0){
            push_information_to_fu(cpu, int_iq_index, 0);
        }
        else if(mul_iq_index>=0){
            push_information_to_fu(cpu, mul_iq_index, 1);
        }
        else if(bu_iq_index>=0){
            push_information_to_fu(cpu, bu_iq_index, 2);
        }
    }
}







void APEX_memory(APEX_CPU *cpu){
    if(cpu->memory.has_insn){
        //for load operation
        if(cpu->memory.cycles==0){
            cpu->memory.cycles++;
            cpu->memory.is_stage_stalled=1;
        }
        else if(cpu->memory.cycles==1){
            if(cpu->memory.opcode==OPCODE_LOAD)
            {
                cpu->memory.result_buffer=cpu->data_memory[cpu->memory.memory_address];
                cpu->memory.cycles=0;
                cpu->memory_fwd=cpu->memory;
                cpu->memory.has_insn=FALSE;
                cpu->memory.is_stage_stalled=0;
            }
            else if(cpu->memory.opcode==OPCODE_STORE)
            {
                cpu->data_memory[cpu->memory.memory_address]=cpu->memory.rs1_value;
                cpu->memory.cycles=0;
                cpu->memory.has_insn=FALSE;
                cpu->memory.is_stage_stalled=0;
            }
        }
    }
}

void push_lsq_instruction_to_memory_fu(APEX_CPU *cpu){
    load_store_queue lsq=cpu->lsq;
    if (lsq.load_store_queue[lsq.head].allocate==1 && lsq.load_store_queue[lsq.head].address_valid==1){
        //if instruction is load =0
        if(lsq.load_store_queue[lsq.head].instruction_type==0){
            if(lsq.load_store_queue[lsq.head].address_valid==1){
                //push instruction to memory function 
                if(cpu->memory.is_stage_stalled==0){
                    cpu->memory.has_insn=TRUE;
                    cpu->memory.memory_address=lsq.load_store_queue[lsq.head].mem_address;
                    cpu->memory.memory_instruction_type=0;
                    cpu->memory.opcode=OPCODE_LOAD;
                    cpu->memory.phy_rd=lsq.load_store_queue[lsq.head].destination_address_for_load;
                }
            }
        }
        //if instruction is store =1
        if(lsq.load_store_queue[lsq.head].instruction_type==1){
            if(lsq.load_store_queue[lsq.head].address_valid==1  &&
                lsq.load_store_queue[lsq.head].data_ready ==1 &&
                lsq.load_store_queue[lsq.head].rob_index == cpu->rob.head){
                 if(cpu->memory.is_stage_stalled==0){
                    cpu->memory.has_insn=TRUE;
                    cpu->memory.memory_address=lsq.load_store_queue[lsq.head].mem_address;
                    cpu->memory.memory_instruction_type=1;
                    cpu->memory.opcode=OPCODE_STORE;
                    //either need to read from physical or architectural register
                    cpu->memory.phy_rs1=lsq.load_store_queue[lsq.head].src1_store;
                    cpu->memory.rs1_value=lsq.load_store_queue[lsq.head].value_to_be_stored;
                }
            }
        }
    }
}



APEX_CPU *APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->arf.architectural_register_file,0,sizeof(architectural_register_content)*ARCHITECTURAL_REGISTERS_SIZE);
    memset(cpu->prf.physical_register,0,sizeof(physical_register_content)*PHYSICAL_REGISTERS_SIZE);

    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    memset(cpu->iq.issue_queue,0,sizeof(issue_queue_entry)*ISSUE_QUEUE_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;
    
    //Initialization of free physiical registers
    for (int i=0;i<PHYSICAL_REGISTERS_SIZE;i++){
        cpu->free_prf_q.free_physical_registers[i]=i;  
    }
    cpu->free_prf_q.head=0;
    cpu->free_prf_q.tail=PHYSICAL_REGISTERS_SIZE-1;

    for (int j=0;j<ARCHITECTURAL_REGISTERS_SIZE+1;j++){
        cpu->rnt.rename_table[j].mapped_to_physical_register=-1;
        cpu->rnt.rename_table[j].register_source=0;
    }

    for (int i=0;i<PHYSICAL_REGISTERS_SIZE;i++){
        cpu->prf.physical_register[i].reg_valid=0;
        cpu->prf.physical_register[i].reg_value=0;
    }
    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock+1);
            printf("--------------------------------------------\n");
        }

        // if (APEX_writeback(cpu))
        // {
        //     /* Halt in writeback stage */
        //     printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
        //     break;
        // }

        
        APEX_phy_writeback(cpu);
        APEX_int_fwd(cpu);
        APEX_memory(cpu);
        APEX_process_iq(cpu);
        APEX_mul_fu_4(cpu);
        APEX_mul_fu_3(cpu);
        APEX_mul_fu_2(cpu);
        APEX_mul_fu_1(cpu);
        APEX_int_fu(cpu);
        //need to add branch funcytion unit here
        APEX_queue_entry_addition(cpu);
        APEX_rename_dispatch(cpu);
        APEX_decode_rename(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}
