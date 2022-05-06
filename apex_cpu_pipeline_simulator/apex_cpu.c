/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
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
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
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
    if(stage->pc>=4000){
        //printf("%-15s: pc(%d) ", name, stage->pc);
        printf("%-15s: I[%d] ",name, (stage->pc-4000)/4);

        printf("\n");
    }
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

    //rename table CCR

    if(cpu->rnt.rename_table[16].register_source==1)
    {
        if(cpu->prf.physical_register[cpu->rnt.rename_table[16].mapped_to_physical_register].reg_valid){
            printf("%d", cpu->prf.physical_register[cpu->rnt.rename_table[16].mapped_to_physical_register].reg_value);
        }
        else{
            printf("%d", cpu->arf.architectural_register_file[cpu->arf.architectural_register_file[16].value].value);
        }
    }

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

        if(cpu->decode_rename.is_stage_stalled==1){
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
            // printf("has isn: %d\n", cpu->fetch.has_insn);
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
    if (cpu->decode_rename.has_insn )
    {

        if(cpu->rename_dispatch.is_stage_stalled==1){
            cpu->decode_rename.is_stage_stalled=1;
            return;
        }
        cpu->decode_rename.is_stage_stalled=0;

        cpu->decode_rename.is_physical_register_required=0;
        cpu->decode_rename.is_src1_register_required=0;
        cpu->decode_rename.is_src2_register_required=0;
        cpu->decode_rename.is_memory_insn=0;

        /* Read operands from register file based on the instruction type */
        switch (cpu->decode_rename.opcode)
        {
            case OPCODE_ADD:
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
            case OPCODE_SUB:
            {
                cpu->decode_rename.is_physical_register_required=1;
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.is_src2_register_required=1;
                cpu->decode_rename.fu=INT_FU;

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

            case OPCODE_BZ:
            {
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.fu=BRANCH_FU;
                break;
            }

            case OPCODE_BNZ:
            {
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.fu=BRANCH_FU;
                break;
            }
            
            case OPCODE_BP:
            {
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.fu=BRANCH_FU;
                break;
            }

            case OPCODE_BNP:
            {
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.fu=BRANCH_FU;
                break;
            }
            case OPCODE_CMP:
            {
                cpu->decode_rename.is_src1_register_required=1;
                cpu->decode_rename.is_src2_register_required=1;
                cpu->decode_rename.fu=BRANCH_FU;
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
                cpu->decode_rename.fu=BRANCH_FU;
                break;
            }
            case OPCODE_HALT:
            {
                cpu->decode_rename.fu=INT_FU;
                break;
            }
        }
        //checking the resources (availabilty of free physical register, iq entry and lsq entry)
    
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
        // if(cpu->rename_dispatch.opcode==OPCODE_RET){
        //     if(cpu->rnt.rename_table[cpu->rename_dispatch.rs1].register_source){
        //         if(cpu->prf.physical_register [cpu->rnt.rename_table[cpu->rename_dispatch.rs1].mapped_to_physical_register].reg_valid){
        //             cpu->pc= cpu->prf.physical_register [cpu->rnt.rename_table[cpu->rename_dispatch.rs1].mapped_to_physical_register].reg_value;
                    
        //             cpu->rename_dispatch.is_stage_stalled=0;
        //         }
        //         else{
        //             cpu->rename_dispatch.is_stage_stalled=1;
        //         }
        //     }
        //     else{
        //         cpu->pc= cpu->arf.architectural_register_file[cpu->rename_dispatch.rs1].value;
        //     }
        //      printf("RETURNED TO PC: %d\n",cpu->pc);
            
        // }
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
if(cpu->queue_entry.has_insn)
    {
    
    if(cpu->queue_entry.opcode==OPCODE_RET){
        //cpu->rename_dispatch.is_stage_stalled=1;
        if(reorder_buffer_available(&cpu->rob) ==-1){
            cpu->rename_dispatch.is_stage_stalled=1;
            return;
        }
        else{
            if(cpu->rnt.rename_table[cpu->queue_entry.rs1].register_source){
                if(cpu->prf.physical_register [cpu->rnt.rename_table[cpu->queue_entry.rs1].mapped_to_physical_register].reg_valid){
                    cpu->pc= cpu->prf.physical_register [cpu->rnt.rename_table[cpu->queue_entry.rs1].mapped_to_physical_register].reg_value;
                    cpu->decode_rename.has_insn=FALSE;
                    cpu->rename_dispatch.has_insn=FALSE;
                    cpu->queue_entry.is_stage_stalled=0;
                }
                else{
                    cpu->queue_entry.is_stage_stalled=1;
                }
            }
            else{
                cpu->pc= cpu->arf.architectural_register_file[cpu->queue_entry.rs1].value;
                cpu->decode_rename.has_insn=FALSE;
                cpu->rename_dispatch.has_insn=FALSE;
            }
             printf("RETURNED TO PC: %d\n",cpu->pc);
            

        //provide rob_entry and return
            cpu->queue_entry.temp_rob_entry.pc_value=cpu->queue_entry.pc;
            cpu->queue_entry.temp_rob_entry.destination_address=cpu->queue_entry.rd;
            //check if physical register is corectly populated
            cpu->queue_entry.temp_rob_entry.physical_register=cpu->queue_entry.phy_rd;
            cpu->queue_entry.temp_rob_entry.status_bit=1;
            cpu->queue_entry.temp_rob_entry.store_value_valid=0;
            cpu->queue_entry.temp_rob_entry.opcode=cpu->queue_entry.opcode;
            cpu->queue_entry.temp_rob_entry.insn_type=BRANCH_FU;
            reorder_buffer_entry_addition_to_queue(&cpu->rob,&cpu->queue_entry.temp_rob_entry);
            cpu->queue_entry.has_insn=FALSE;
            return;
        }
            
    }

    cpu->queue_entry.rs1_ready=1;
        cpu->queue_entry.rs2_ready=1;
        if(cpu->queue_entry.is_src1_register_required){
            int temp_physcial_src1=100;
            //if need to reaad the content from physical register
            if(cpu->rnt.rename_table[cpu->queue_entry.rs1].register_source){
                temp_physcial_src1=cpu->rnt.rename_table[cpu->queue_entry.rs1].mapped_to_physical_register;
                //if physical register content is valid then read the value 
                if(cpu->prf.physical_register[temp_physcial_src1].reg_valid){
                    cpu->queue_entry.rs1_value=cpu->prf.physical_register[temp_physcial_src1].reg_value;
                    cpu->queue_entry.phy_rs1=temp_physcial_src1;
                    cpu->queue_entry.rs1_ready=1;
                }
                //if physical register content is invalid  then read the prf  from which it  need to be read  
                else{
                    cpu->queue_entry.phy_rs1=temp_physcial_src1;
                    cpu->queue_entry.rs1_ready=0;
                }
            }
            //read from architectural register 
            else{
                 cpu->queue_entry.rs1_value= cpu->arf.architectural_register_file[cpu->queue_entry.rs1].value;
                 cpu->queue_entry.rs1_ready=1;
            }
            if (cpu->queue_entry.opcode == OPCODE_JUMP || cpu->queue_entry.opcode == OPCODE_JALR || cpu->queue_entry.opcode == OPCODE_RET)
            {
                create_mri_backup(cpu);
                create_rename_table_backup(cpu);
            }
            //if opcode is bz or bnz or bp or bnp then check the condition
            if (cpu->queue_entry.opcode == OPCODE_BZ || cpu->queue_entry.opcode == OPCODE_BNZ || cpu->queue_entry.opcode == OPCODE_BP || cpu->queue_entry.opcode == OPCODE_BNP)
            {
                //create a backup of mri and rnt
                create_mri_backup(cpu);
                create_rename_table_backup(cpu);
                //read the rename table last entry
                if(cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].register_source){
                    temp_physcial_src1=cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].mapped_to_physical_register;
                    //if physical register content is valid then read the value 
                    if(cpu->prf.physical_register[temp_physcial_src1].reg_valid){
                        cpu->queue_entry.rs1_value=cpu->prf.physical_register[temp_physcial_src1].reg_value;
                        cpu->queue_entry.phy_rs1=temp_physcial_src1;
                        cpu->queue_entry.rs1_ready=1;
                    }
                    else{
                        cpu->queue_entry.phy_rs1=temp_physcial_src1;
                        cpu->queue_entry.rs1_ready=0;
                    }
                }
                else{
                        cpu->queue_entry.rs1_value=  cpu->arf.architectural_register_file[cpu->arf.architectural_register_file[ARCHITECTURAL_REGISTERS_SIZE].value].value;
                        cpu->queue_entry.rs1_ready=1;
                    }
            }

        }

        if(cpu->queue_entry.is_src2_register_required){
            int temp_physcial_src2=100;
            if(cpu->rnt.rename_table[cpu->queue_entry.rs2].register_source){
                temp_physcial_src2=cpu->rnt.rename_table[cpu->queue_entry.rs2].mapped_to_physical_register;
                if(cpu->prf.physical_register[temp_physcial_src2].reg_valid){
                    cpu->queue_entry.rs2_value=cpu->prf.physical_register[temp_physcial_src2].reg_value;
                    cpu->queue_entry.phy_rs2=temp_physcial_src2;
                    cpu->queue_entry.rs2_ready=1;
                }
                else{
                    cpu->queue_entry.phy_rs2=temp_physcial_src2;
                    cpu->queue_entry.rs2_ready=0;
                }
            }
            else{
                 cpu->queue_entry.rs2_value= cpu->arf.architectural_register_file[cpu->queue_entry.rs2].value;
                 cpu->queue_entry.rs2_ready=1;
            }
        }

        cpu->queue_entry.phy_rd=100;//default value is set to 100 for phy_rd
        //ccr update for cmp , assigned to last physical register
        if (cpu->queue_entry.opcode==OPCODE_CMP){
             cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].mapped_to_physical_register= PHYSICAL_REGISTERS_SIZE;
             cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].register_source=1;
             //check this code for cmp
             cpu->mri[ARCHITECTURAL_REGISTERS_SIZE]=PHYSICAL_REGISTERS_SIZE+2;
             printf("MRP CCR=P%d\n", cpu->mri[ARCHITECTURAL_REGISTERS_SIZE]);
        }

        if(cpu->queue_entry.is_physical_register_required){
            int temp_rd=pop_free_physical_registers(&cpu->free_prf_q);
            if( temp_rd!= -1){
                cpu->queue_entry.phy_rd =temp_rd;
                cpu->rnt.rename_table[cpu->queue_entry.rd].mapped_to_physical_register=temp_rd;
                cpu->rnt.rename_table[cpu->queue_entry.rd].register_source=1;
                cpu->mri[cpu->queue_entry.rd]=temp_rd;
                //if insn is add sub addl subl mul 
                if( cpu->queue_entry.opcode==OPCODE_ADD || 
                    cpu->queue_entry.opcode==OPCODE_ADDL || 
                    cpu->queue_entry.opcode==OPCODE_SUB || 
                    cpu->queue_entry.opcode==OPCODE_SUBL || 
                    cpu->queue_entry.opcode==OPCODE_MUL){
                       cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].mapped_to_physical_register=temp_rd;
                       cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].register_source=1;
                       cpu->mri[ARCHITECTURAL_REGISTERS_SIZE]=temp_rd;
                       printf("MRP CCR=P%d\n", cpu->mri[ARCHITECTURAL_REGISTERS_SIZE]);
                    }
                printf("Physical Reg Allocation: +P[%d]\n",cpu->queue_entry.phy_rd);
                printf("RNT change R[%d]=p[%d]\n", cpu->queue_entry.rd,cpu->queue_entry.phy_rd);
            }
            else  //setting the stalling variable to 1 if no free physical register available
                cpu->queue_entry.is_stage_stalled=1;
        }
            int temp_iq_index=issue_buffer_index_available(&cpu->iq);
        //printf("%d",temp_iq_index);
        int temp_lsq_index=100;
        if(cpu->queue_entry.is_memory_insn){
            temp_lsq_index=lsq_index_available(&cpu->lsq);
        }
        int temp_rob_index=reorder_buffer_available(&cpu->rob);
        if(temp_iq_index!=-1 && temp_lsq_index!=-1 && temp_rob_index!=-1){
           //temp lsq entry , rob entry and iq entry are available
            cpu->queue_entry.temp_iq_entry.dest_tag=cpu->queue_entry.phy_rd;
            cpu->queue_entry.temp_iq_entry.src1_tag=cpu->queue_entry.phy_rs1;
            cpu->queue_entry.temp_iq_entry.src2_tag=cpu->queue_entry.phy_rs2;
            cpu->queue_entry.temp_iq_entry.src1_valid=cpu->queue_entry.rs1_ready;
            cpu->queue_entry.temp_iq_entry.src2_valid=cpu->queue_entry.rs2_ready;
            cpu->queue_entry.temp_iq_entry.src1_value=cpu->queue_entry.rs1_value;
            cpu->queue_entry.temp_iq_entry.src2_value=cpu->queue_entry.rs2_value;
            cpu->queue_entry.temp_iq_entry.FU=cpu->queue_entry.fu;
            cpu->queue_entry.temp_iq_entry.immediate_literal=cpu->queue_entry.imm;
            cpu->queue_entry.temp_iq_entry.is_allocated=1;
            cpu->queue_entry.temp_iq_entry.rob_index=temp_rob_index;
            cpu->queue_entry.temp_iq_entry.lsq_index=temp_lsq_index;
            cpu->queue_entry.temp_iq_entry.opcode=cpu->queue_entry.opcode;
            cpu->queue_entry.temp_iq_entry.pc_value=cpu->queue_entry.pc;
            cpu->queue_entry.temp_iq_entry.counter=0;
            cpu->queue_entry.issue_queue_index=temp_iq_index;
            cpu->queue_entry.temp_rob_entry.insn_type=cpu->queue_entry.fu;

            if(temp_lsq_index!=100 && temp_lsq_index != -1){
                cpu->queue_entry.temp_lsq_entry.allocate=1;
                cpu->queue_entry.temp_lsq_entry.instruction_type=cpu->queue_entry.memory_instruction_type;
                cpu->queue_entry.temp_lsq_entry.address_valid=0;
                cpu->queue_entry.temp_lsq_entry.data_ready=0;
                cpu->queue_entry.temp_lsq_entry.OPCODE=cpu->queue_entry.opcode;
                cpu->queue_entry.temp_lsq_entry.data_ready=cpu->queue_entry.rs1_ready;
                cpu->queue_entry.temp_lsq_entry.value_to_be_stored=cpu->queue_entry.rs1_value;
                cpu->queue_entry.temp_lsq_entry.src1_store=cpu->queue_entry.phy_rs1;
                cpu->queue_entry.temp_rob_entry.insn_type=3;
                cpu->queue_entry.temp_lsq_entry.pc_value=cpu->queue_entry.pc;
                cpu->queue_entry.temp_lsq_entry.phy_destination_address_for_load=cpu->queue_entry.phy_rd;
                cpu->queue_entry.temp_lsq_entry.destination_address_for_load=cpu->queue_entry.rd;
            }
            
            //check the pc value later
            cpu->queue_entry.temp_rob_entry.pc_value=cpu->queue_entry.pc;
            cpu->queue_entry.temp_rob_entry.destination_address=cpu->queue_entry.rd;
            //check if physical register is corectly populated
            cpu->queue_entry.temp_rob_entry.physical_register=cpu->queue_entry.phy_rd;
            cpu->queue_entry.temp_rob_entry.status_bit=0;
            cpu->queue_entry.temp_rob_entry.store_value_valid=0;
            cpu->queue_entry.temp_rob_entry.opcode=cpu->queue_entry.opcode;

            //
        }
        else{
            cpu->rename_dispatch.is_stage_stalled=1;
        } 
        //print_iq_indexes(&cpu->iq);
    int rob_index,lsq_index;
    lsq_index=100;
        rob_index= reorder_buffer_entry_addition_to_queue(&cpu->rob,&cpu->queue_entry.temp_rob_entry);
        if(cpu->queue_entry.is_memory_insn){
            cpu->queue_entry.temp_lsq_entry.rob_index=rob_index;
            lsq_index=lsq_entry_addition_to_queue(&cpu->lsq,&cpu->queue_entry.temp_lsq_entry);
        }
        cpu->queue_entry.temp_iq_entry.rob_index=rob_index;
        cpu->queue_entry.temp_iq_entry.lsq_index=lsq_index;
        iq_entry_addition(&cpu->iq,&cpu->queue_entry.temp_iq_entry,cpu->queue_entry.issue_queue_index);

        printf("IQ + I[%d]\n", (cpu->queue_entry.pc-4000)/4);

        //print_rob_entries(&cpu->rob);
        //cpu->process_iq=cpu->queue_entry;
        //printf("%d",cpu->int_fu.imm);
        cpu->queue_entry.has_insn = FALSE;
    
    if(cpu->queue_entry.opcode==OPCODE_JALR){
            create_mri_backup(cpu);
            create_rename_table_backup(cpu);

    }
        
    //print_iq_entries(&cpu->iq);
    if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("All queue entry", &cpu->queue_entry);
        }
    }
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
        cpu->int_fu.lsq_index=cpu->iq.issue_queue[index].lsq_index;
        cpu->int_fu.opcode=cpu->iq.issue_queue[index].opcode;
        cpu->int_fu.has_insn=1;
        cpu->int_fu.imm=cpu->iq.issue_queue[index].immediate_literal;
        cpu->iq.issue_queue[index].is_allocated=0;
        cpu->int_fu.pc=cpu->iq.issue_queue[index].pc_value;
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
        cpu->mul1_fu.pc=cpu->iq.issue_queue[index].pc_value;

        break;
    //branch fu
    case BRANCH_FU:
        cpu->bu_fu.rs1_value=cpu->iq.issue_queue[index].src1_value;
        cpu->bu_fu.rs2_value=cpu->iq.issue_queue[index].src2_value;
        cpu->bu_fu.imm=cpu->iq.issue_queue[index].immediate_literal;
        cpu->bu_fu.phy_rd=cpu->iq.issue_queue[index].dest_tag;
        cpu->bu_fu.rob_index=cpu->iq.issue_queue[index].rob_index;
        cpu->bu_fu.lsq_index=cpu->iq.issue_queue[index].lsq_index;
        cpu->bu_fu.opcode=cpu->iq.issue_queue[index].opcode;
        cpu->bu_fu.has_insn=1;
        cpu->iq.issue_queue[index].is_allocated=0;
        cpu->bu_fu.pc=cpu->iq.issue_queue[index].pc_value;
        break;
    default:
        break;
    }
    printf("IQ - I[%d]\n", (cpu->iq.issue_queue[index].pc_value-4000)/4);

}

void APEX_bu_fu(APEX_CPU *cpu){
    if(cpu->bu_fu.has_insn){
        cpu->bu_fu.need_to_flush=0;
        switch (cpu->bu_fu.opcode)
        {
            case OPCODE_BZ:
                if(cpu->bu_fu.rs1_value==0){
                    cpu->bu_fu.pc_value_to_be_taken=cpu->bu_fu.pc+cpu->bu_fu.imm;
                    cpu->bu_fu.need_to_flush=1;
                    cpu->fetch_from_next_cycle=TRUE;
                    // cpu->fetch.has_insn=TRUE;
                    //flush_instructions(cpu,cpu->bu_fu.rob_index);
                }
                break;
            case OPCODE_BNZ:
                if(cpu->bu_fu.rs1_value!=0){
                    cpu->bu_fu.pc_value_to_be_taken=cpu->bu_fu.pc+cpu->bu_fu.imm;
                    //flush_instructions(cpu,cpu->bu_fu.rob_index);
                    cpu->bu_fu.need_to_flush=1;
                    //cpu->fetch_from_next_cycle=TRUE;
                    //cpu->fetch.has_insn=TRUE;
                }
                break;
            case OPCODE_BP:
                if(cpu->bu_fu.rs1_value>0){
                    cpu->bu_fu.pc_value_to_be_taken=cpu->bu_fu.pc+cpu->bu_fu.imm;
                    cpu->bu_fu.need_to_flush=1;
                    //cpu->fetch_from_next_cycle=TRUE;
                    //cpu->fetch.has_insn=TRUE;
                }
                break;
            case OPCODE_BNP:
                if(cpu->bu_fu.rs1_value<0){
                    cpu->bu_fu.pc_value_to_be_taken=cpu->bu_fu.pc+cpu->bu_fu.imm;
                    cpu->bu_fu.need_to_flush=1;
                    //cpu->fetch_from_next_cycle=TRUE;
                    //cpu->fetch.has_insn=TRUE;
                }
                break;
            case OPCODE_JUMP:
            {
                cpu->bu_fu.pc_value_to_be_taken=cpu->bu_fu.rs1_value+cpu->bu_fu.imm;
                cpu->bu_fu.need_to_flush=1;
                //cpu->fetch_from_next_cycle=TRUE;
                //cpu->fetch.has_insn=TRUE;
                break;
            }
            case OPCODE_JALR:
            {
                cpu->bu_fu.result_buffer=cpu->bu_fu.pc+4;
                cpu->bu_fu.pc_value_to_be_taken=cpu->bu_fu.rs1_value+cpu->bu_fu.imm;
                cpu->bu_fu.need_to_flush=1;
                //cpu->fetch_from_next_cycle=TRUE;
                //cpu->fetch.has_insn=TRUE;
                break;
            }
            default:
                break;
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
                print_stage_content("BU FU", &cpu->bu_fu);
        }
        cpu->bu_fwd=cpu->bu_fu;
        cpu->bu_fu.has_insn=FALSE;
    }
}



void APEX_bu_fwd(APEX_CPU *cpu){
    if(cpu->bu_fwd.has_insn){
        if(cpu->bu_fwd.need_to_flush){
            flush_instructions(cpu,cpu->bu_fwd.rob_index);
            cpu->pc=cpu->bu_fwd.pc_value_to_be_taken;
            cpu->fetch.has_insn=TRUE;
        }
        cpu->branch_writeback=cpu->bu_fwd;
        cpu->bu_fwd.has_insn=FALSE;
    if (ENABLE_DEBUG_MESSAGES)
    {
            print_stage_content("BU Fwd", &cpu->bu_fwd);
    }
    }


}

void APEX_branch_writeback(APEX_CPU *cpu){
    if(cpu->branch_writeback.has_insn){
        if(cpu->branch_writeback.opcode==OPCODE_JALR){
            cpu->prf.physical_register[cpu->branch_writeback.phy_rd].reg_value=cpu->branch_writeback.result_buffer;
            cpu->prf.physical_register[cpu->branch_writeback.phy_rd].reg_valid=1;
            printf("PRF updated for P[%d]\n",cpu->branch_writeback.phy_rd);

            
            for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
            if(cpu->iq.issue_queue[i].is_allocated==1){
                if(!cpu->iq.issue_queue[i].src1_valid){
                    if(cpu->iq.issue_queue[i].src1_tag==cpu->branch_writeback.phy_rd){
                        cpu->iq.issue_queue[i].src1_value=cpu->branch_writeback.result_buffer;
                        cpu->iq.issue_queue[i].src1_valid=1;
                    }
                }
                if(!cpu->iq.issue_queue[i].src2_valid){
                    if(cpu->iq.issue_queue[i].src2_tag==cpu->branch_writeback.phy_rd){
                        cpu->iq.issue_queue[i].src2_value=cpu->branch_writeback.result_buffer;
                        cpu->iq.issue_queue[i].src2_valid=1;
                    }
                }
            }
        }
        //update lsq instruction for which phys_rd is matched
        for(int i=cpu->lsq.head;i!=cpu->lsq.tail;i=(i+1)%LSQ_SIZE){
            //if instn is store
            if(cpu->lsq.load_store_queue[i].OPCODE==OPCODE_STORE){
                if(!cpu->lsq.load_store_queue[i].data_ready){
                    if(cpu->lsq.load_store_queue[i].src1_store==cpu->branch_writeback.phy_rd){
                        cpu->lsq.load_store_queue[i].data_ready=1;
                        cpu->lsq.load_store_queue[i].value_to_be_stored=cpu->branch_writeback.result_buffer;
                    }
                }
            }
        }
        }
        cpu->rob.reorder_buffer_queue[cpu->branch_writeback.rob_index].status_bit=1;
        cpu->branch_writeback.has_insn=FALSE;
    if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Branch WB", &cpu->branch_writeback);
        }
    }
}


void APEX_int_fu(APEX_CPU *cpu){
    if(cpu->int_fu.has_insn){
        switch (cpu->int_fu.opcode)
        {
        case OPCODE_ADD:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value+cpu->int_fu.rs2_value;
            cpu->int_fu.positive_flag= (cpu->int_fu.result_buffer>0)?1:0;
            cpu->int_fu.zero_flag= (cpu->int_fu.result_buffer==0)?1:0;
            break;
        case OPCODE_ADDL:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value+cpu->int_fu.imm;
            cpu->int_fu.positive_flag= (cpu->int_fu.result_buffer>0)?1:0;
            cpu->int_fu.zero_flag= (cpu->int_fu.result_buffer==0)?1:0;
            break;
        case OPCODE_SUB:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value-cpu->int_fu.rs2_value;
            cpu->int_fu.positive_flag= (cpu->int_fu.result_buffer>0)?1:0;
            cpu->int_fu.zero_flag= (cpu->int_fu.result_buffer==0)?1:0;
            break;
        case OPCODE_SUBL:
            cpu->int_fu.result_buffer=cpu->int_fu.rs1_value-cpu->int_fu.imm;
            cpu->int_fu.positive_flag= (cpu->int_fu.result_buffer>0)?1:0;
            cpu->int_fu.zero_flag= (cpu->int_fu.result_buffer==0)?1:0;
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
            break;
        case OPCODE_STORE:
            cpu->int_fu.memory_address=cpu->int_fu.rs2_value+cpu->int_fu.imm;
            break;
        default:
            break;
        }
        cpu->int_fwd=cpu->int_fu;
        cpu->int_fu.has_insn=FALSE;
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Integer Functional Unit", &cpu->int_fu);
        }
    }
    }


void APEX_int_fwd(APEX_CPU *cpu){
    if(cpu->int_fwd.has_insn){


        if(cpu->int_fwd.opcode==OPCODE_STORE || cpu->int_fwd.opcode==OPCODE_LOAD){
            cpu->lsq.load_store_queue[cpu->int_fwd.lsq_index].mem_address  = cpu->int_fwd.memory_address;
            cpu->lsq.load_store_queue[cpu->int_fwd.lsq_index].address_valid = 1;
            printf("LSQ I[%d] memory address calculated \n",(cpu->int_fwd.pc -4000)/4);
            printf("calculated address is %d \n",cpu->lsq.load_store_queue[cpu->int_fwd.lsq_index].mem_address);
        }

        if(cpu->int_fwd.opcode!=OPCODE_STORE && cpu->int_fwd.opcode!=OPCODE_LOAD){
            cpu->int_writeback=cpu->int_fwd;
        }
        cpu->int_fwd.has_insn=FALSE;

    if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Integer forward Bus", &cpu->int_fwd);
        }
    }

}

void APEX_memory_fwd(APEX_CPU *cpu){
    if(cpu->memory_fwd.has_insn){
        // for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
        //     if(cpu->iq.issue_queue[i].is_allocated==1){
        //         if(!cpu->iq.issue_queue[i].src1_valid){
        //             if(cpu->iq.issue_queue[i].src1_tag==cpu->memory_fwd.phy_rd){
        //               cpu->iq.issue_queue[i].src1_value=cpu->memory_fwd.result_buffer;
        //                 cpu->iq.issue_queue[i].src1_valid=1;
        //             }
        //         }
        //         if(!cpu->iq.issue_queue[i].src2_valid){
        //             if(cpu->iq.issue_queue[i].src2_tag==cpu->memory_fwd.phy_rd){
        //                 cpu->iq.issue_queue[i].src2_value=cpu->memory_fwd.result_buffer;
        //                 cpu->iq.issue_queue[i].src2_valid=1;
        //             }
        //         }
        //     }
        // }
        // //update lsq instruction for which phys_rd is matched
        // for(int i=cpu->lsq.head;i!=cpu->lsq.tail;i=(i+1)%LSQ_SIZE){
        //     //if instn is store
        //     if(cpu->lsq.load_store_queue[i].OPCODE==OPCODE_STORE){
        //         if(!cpu->lsq.load_store_queue[i].data_ready){
        //             if(cpu->lsq.load_store_queue[i].src1_store==cpu->memory_fwd.phy_rd){
        //                 cpu->lsq.load_store_queue[i].data_ready=1;
        //                 cpu->lsq.load_store_queue[i].value_to_be_stored=cpu->memory_fwd.result_buffer;
        //             }
        //         }
        //     }
        // }
        if(cpu->memory_fwd.opcode==OPCODE_STORE){
            cpu->data_memory[cpu->memory_fwd.memory_address]=cpu->memory_fwd.result_buffer;
            printf("data[%d]=%d\n", cpu->memory_fwd.memory_address,cpu->data_memory[cpu->memory_fwd.memory_address]);
            cpu->rob.reorder_buffer_queue[cpu->memory_fwd.rob_index].status_bit=1;
            printf("ROB I[%d] status bit updated\n",(cpu->memory_fwd.pc -4000)/4);
            //cpu->lsq.head=(cpu->lsq.head+1)%LSQ_SIZE;
        }
        if(cpu->memory_fwd.opcode==OPCODE_LOAD){
            cpu->mem_writeback=cpu->memory_fwd;
        }
        cpu->memory_fwd.has_insn=FALSE;
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory forward Bus", &cpu->memory_fwd);
        }
    }

}

static int APEX_int_writeback(APEX_CPU *cpu){
    if(cpu->int_writeback.has_insn){

        if(cpu->int_writeback.opcode==OPCODE_HALT){
            cpu->rob.reorder_buffer_queue[cpu->int_writeback.rob_index].status_bit=1;
            printf("Halting the CPU\n");
            goto last;
        }

        if(cpu->int_writeback.opcode!=OPCODE_STORE && cpu->int_writeback.opcode!=OPCODE_LOAD){
            cpu->prf.physical_register[cpu->int_writeback.phy_rd].reg_value=cpu->int_writeback.result_buffer;
            cpu->prf.physical_register[cpu->int_writeback.phy_rd].positive_flag=cpu->int_writeback.positive_flag;
            cpu->prf.physical_register[cpu->int_writeback.phy_rd].zero_flag=cpu->int_writeback.zero_flag;
            cpu->prf.physical_register[cpu->int_writeback.phy_rd].reg_valid=1;

            printf("PRF updated for P[%d]\n",cpu->int_writeback.phy_rd);
        }
        if(cpu->int_writeback.opcode==OPCODE_STORE || cpu->int_writeback.opcode==OPCODE_LOAD){
            cpu->lsq.load_store_queue[cpu->int_writeback.lsq_index].mem_address=cpu->int_writeback.memory_address;
            cpu->lsq.load_store_queue[cpu->int_writeback.lsq_index].address_valid=1;
        }
        //if instn is add addl sub subl
        if(cpu->int_writeback.opcode==OPCODE_ADDL || cpu->int_writeback.opcode==OPCODE_SUBL || cpu->int_writeback.opcode==OPCODE_SUB || cpu->int_writeback.opcode==OPCODE_ADD){
            printf("after the result zero flag is %d\n",cpu->prf.physical_register[cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].mapped_to_physical_register].positive_flag);
            printf("after the result positive flag is %d\n",cpu->prf.physical_register[cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].mapped_to_physical_register].zero_flag);
        }


        for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
            if(cpu->iq.issue_queue[i].is_allocated==1){
                if(!cpu->iq.issue_queue[i].src1_valid){
                    if(cpu->iq.issue_queue[i].src1_tag==cpu->int_writeback.phy_rd){
                        cpu->iq.issue_queue[i].src1_value=cpu->int_writeback.result_buffer;
                        cpu->iq.issue_queue[i].src1_valid=1;
                    }
                }
                if(!cpu->iq.issue_queue[i].src2_valid){
                    if(cpu->iq.issue_queue[i].src2_tag==cpu->int_writeback.phy_rd){
                        cpu->iq.issue_queue[i].src2_value=cpu->int_writeback.result_buffer;
                        cpu->iq.issue_queue[i].src2_valid=1;
                    }
                }
            }
        }
        //update lsq instruction for which phys_rd is matched
        for(int i=cpu->lsq.head;i!=cpu->lsq.tail;i=(i+1)%LSQ_SIZE){
            //if instn is store
            if(cpu->lsq.load_store_queue[i].OPCODE==OPCODE_STORE){
                if(!cpu->lsq.load_store_queue[i].data_ready){
                    if(cpu->lsq.load_store_queue[i].src1_store==cpu->int_writeback.phy_rd){
                        cpu->lsq.load_store_queue[i].data_ready=1;
                        cpu->lsq.load_store_queue[i].value_to_be_stored=cpu->int_writeback.result_buffer;
                    }
                }
            }
        }
    cpu->rob_commit=cpu->int_writeback;
    if(cpu->int_writeback.opcode!=OPCODE_STORE && cpu->int_writeback.opcode!=OPCODE_LOAD){
        cpu->rob.reorder_buffer_queue[cpu->int_writeback.rob_index].status_bit=1;
        cpu->rob.reorder_buffer_queue[cpu->int_writeback.rob_index].result_value=cpu->int_writeback.result_buffer;
        cpu->rob.reorder_buffer_queue[cpu->int_writeback.rob_index].positive_flag=cpu->int_writeback.positive_flag;
        cpu->rob.reorder_buffer_queue[cpu->int_writeback.rob_index].zero_flag=cpu->int_writeback.zero_flag;

    }
last:
    cpu->int_writeback.has_insn=FALSE;
    if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Integer WB", &cpu->int_writeback);
        }
    }
    return 0;
}

void APEX_mul_writeback(APEX_CPU *cpu){
    if(cpu->mul_writeback.has_insn){
        cpu->prf.physical_register[cpu->mul_writeback.phy_rd].reg_value=cpu->mul_writeback.result_buffer;
        cpu->prf.physical_register[cpu->mul_writeback.phy_rd].positive_flag=cpu->mul_writeback.positive_flag;
        cpu->prf.physical_register[cpu->mul_writeback.phy_rd].zero_flag=cpu->mul_writeback.zero_flag;
        cpu->prf.physical_register[cpu->mul_writeback.phy_rd].reg_valid=1;
        printf("PRF updated for P[%d]\n",cpu->mul_writeback.phy_rd);


        for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
            if(cpu->iq.issue_queue[i].is_allocated==1){
                if(!cpu->iq.issue_queue[i].src1_valid){
                    if(cpu->iq.issue_queue[i].src1_tag==cpu->mul_writeback.phy_rd){
                        cpu->iq.issue_queue[i].src1_value=cpu->mul_writeback.result_buffer;
                        cpu->iq.issue_queue[i].src1_valid=1;
                    }
                }
                if(!cpu->iq.issue_queue[i].src2_valid){
                    if(cpu->iq.issue_queue[i].src2_tag==cpu->mul_writeback.phy_rd){
                        cpu->iq.issue_queue[i].src2_value=cpu->mul_writeback.result_buffer;
                        cpu->iq.issue_queue[i].src2_valid=1;
                    }
                }
            }
        }
        //update lsq instruction for which phys_rd is matched
        for(int i=cpu->lsq.head;i!=cpu->lsq.tail;i=(i+1)%LSQ_SIZE){
            //if instn is store
            if(cpu->lsq.load_store_queue[i].OPCODE==OPCODE_STORE){
                if(!cpu->lsq.load_store_queue[i].data_ready){
                    if(cpu->lsq.load_store_queue[i].src1_store==cpu->mul_writeback.phy_rd){
                        cpu->lsq.load_store_queue[i].data_ready=1;
                        cpu->lsq.load_store_queue[i].value_to_be_stored=cpu->mul_writeback.result_buffer;
                    }
                }
            }
        }
    cpu->rob.reorder_buffer_queue[cpu->mul_writeback.rob_index].status_bit=1;
    cpu->rob.reorder_buffer_queue[cpu->mul_writeback.rob_index].result_value=cpu->mul_writeback.result_buffer;
    cpu->rob.reorder_buffer_queue[cpu->mul_writeback.rob_index].positive_flag=cpu->mul_writeback.positive_flag;
    cpu->rob.reorder_buffer_queue[cpu->mul_writeback.rob_index].zero_flag=cpu->mul_writeback.zero_flag;
    cpu->mul_writeback.has_insn=FALSE;
    if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Multiplication WB", &cpu->mul_writeback);
        }
    }
}

void APEX_mem_writeback(APEX_CPU *cpu){
    if(cpu->mem_writeback.has_insn){
        cpu->prf.physical_register[cpu->mem_writeback.phy_rd].reg_value=cpu->mem_writeback.result_buffer;
        printf("read from memory data[]= %d\n",cpu->mem_writeback.result_buffer);
        cpu->prf.physical_register[cpu->mem_writeback.phy_rd].reg_valid=1;
        printf("PRF updated for P[%d]\n",cpu->mem_writeback.phy_rd);


        for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
            if(cpu->iq.issue_queue[i].is_allocated==1){
                if(!cpu->iq.issue_queue[i].src1_valid){
                    if(cpu->iq.issue_queue[i].src1_tag==cpu->mem_writeback.phy_rd){
                        cpu->iq.issue_queue[i].src1_value=cpu->mem_writeback.result_buffer;
                        cpu->iq.issue_queue[i].src1_valid=1;
                    }
                }
                if(!cpu->iq.issue_queue[i].src2_valid){
                    if(cpu->iq.issue_queue[i].src2_tag==cpu->mem_writeback.phy_rd){
                        cpu->iq.issue_queue[i].src2_value=cpu->mem_writeback.result_buffer;
                        cpu->iq.issue_queue[i].src2_valid=1;
                    }
                }
            }
        }
        cpu->rob.reorder_buffer_queue[cpu->mem_writeback.rob_index].status_bit=1;
        cpu->rob.reorder_buffer_queue[cpu->mem_writeback.rob_index].result_value=cpu->mem_writeback.result_buffer;
        cpu->lsq.head=(cpu->lsq.head+1)%LSQ_SIZE;
        cpu->mem_writeback.has_insn=FALSE;
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory WB", &cpu->mem_writeback);
        }
    }

}

void APEX_mul_fu_1(APEX_CPU *cpu){
    if(cpu->mul1_fu.has_insn){
        cpu->mul2_fu=cpu->mul1_fu;
        cpu->mul1_fu.has_insn=FALSE;
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("MUL FU1", &cpu->mul1_fu);
        }
    }
}

void APEX_mul_fu_2(APEX_CPU *cpu){
    if(cpu->mul2_fu.has_insn){
        cpu->mul3_fu=cpu->mul2_fu;
        cpu->mul2_fu.has_insn=FALSE;
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("MUL FU2", &cpu->mul2_fu);
        }
    }
}

void APEX_mul_fu_3(APEX_CPU *cpu){
    if(cpu->mul3_fu.has_insn){
        cpu->mul4_fu=cpu->mul3_fu;
        cpu->mul3_fu.has_insn=FALSE;
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("MUL FU3", &cpu->mul3_fu);
        }
    }
}


void APEX_mul_fu_4(APEX_CPU *cpu){
    if(cpu->mul4_fu.has_insn){
        if(cpu->mul4_fu.opcode==OPCODE_MUL){
            cpu->mul4_fu.result_buffer=cpu->mul4_fu.rs1_value*cpu->mul4_fu.rs2_value;
            cpu->mul4_fu.positive_flag=(cpu->mul4_fu.result_buffer>0)?1:0;
            cpu->mul4_fu.zero_flag=(cpu->mul4_fu.result_buffer==0)?1:0;
        }
        else{
            cpu->mul4_fu.result_buffer=cpu->mul4_fu.rs1_value/cpu->mul4_fu.rs2_value;
            cpu->mul4_fu.positive_flag=(cpu->mul4_fu.result_buffer>0)?1:0;
            cpu->mul4_fu.zero_flag=(cpu->mul4_fu.result_buffer==0)?1:0;
        }
        cpu->mul_fwd=cpu->mul4_fu;
        cpu->mul4_fu.has_insn=FALSE;
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("MUL FU4", &cpu->mul4_fu);
        }
    }
}

void APEX_mul_fwd(APEX_CPU *cpu){
    if(cpu->mul_fwd.has_insn){

        if(cpu->mul_fwd.opcode!=OPCODE_STORE && cpu->mul_fwd.opcode!=OPCODE_LOAD){
            cpu->mul_writeback=cpu->mul_fwd;
        }
        
        cpu->mul_fwd.has_insn=FALSE;
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Mul fwd bus", &cpu->mul_fwd);
        }
    }

}


//identify iq index and push information 
void APEX_process_iq(APEX_CPU *cpu){
    
        int int_iq_index=-1;
        int mul_iq_index=-1;
        int bu_iq_index=-1;
        
        int_iq_index = get_iq_index_fu(&cpu->iq, 0);//0 for add
        mul_iq_index = get_iq_index_fu(&cpu->iq, 1);//1  for mul
        bu_iq_index  = get_iq_index_fu(&cpu->iq, 2);//2 for branch
            
        if(int_iq_index>=0){
            push_information_to_fu(cpu, int_iq_index, 0);
        }
        if(mul_iq_index>=0){
            push_information_to_fu(cpu, mul_iq_index, 1);
        }
        if(bu_iq_index>=0){
            push_information_to_fu(cpu, bu_iq_index, 2);
        }
}







void  APEX_memory(APEX_CPU *cpu){
    if(cpu->memory.has_insn){
        //for load operation
        if(cpu->memory.cycles==0){

            cpu->memory.cycles++;
            cpu->memory.is_stage_stalled=1;
            printf("Memory I[%d] in progress\n", (cpu->memory.pc-4000)/4);
        }
        else if(cpu->memory.cycles==1){
            if(cpu->memory.opcode==OPCODE_LOAD)
            {
                cpu->memory.result_buffer=cpu->data_memory[cpu->memory.memory_address];
                cpu->memory.cycles=0;
                cpu->memory_fwd=cpu->memory;
                cpu->memory.has_insn=FALSE;
                cpu->memory.is_stage_stalled=0;

                //update rob
                // cpu->rob.reorder_buffer_queue[cpu->memory.rob_index].status_bit=1;
                cpu->rob.reorder_buffer_queue[cpu->memory.rob_index].result_value=cpu->memory.result_buffer;
            }
            else if(cpu->memory.opcode==OPCODE_STORE)
            {
                cpu->memory.result_buffer=cpu->memory.rs1_value;
                cpu->memory.cycles=0;
                cpu->memory_fwd=cpu->memory;
                cpu->memory.has_insn=FALSE;
                cpu->memory.is_stage_stalled=0;

                // cpu->rob.reorder_buffer_queue[cpu->memory.rob_index].status_bit=1;
            }
            printf("Memory I[%d] completed\n", (cpu->memory.pc-4000)/4);
            cpu->memory.has_insn=FALSE;
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
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
                    cpu->memory.phy_rd=lsq.load_store_queue[lsq.head].phy_destination_address_for_load;
                    cpu->memory.rd=lsq.load_store_queue[lsq.head].destination_address_for_load;
                    cpu->memory.rob_index=lsq.load_store_queue[lsq.head].rob_index;
                    printf("**************************************\n");
                    printf("ROB index %d\n", cpu->memory.rob_index);
                    printf("**************************************\n");
                    cpu->lsq.load_store_queue[lsq.head].allocate=0;
                    cpu->memory.pc=lsq.load_store_queue[lsq.head].pc_value;
                    cpu->lsq.head=(cpu->lsq.head+1)%LSQ_SIZE;

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
                    cpu->memory.rob_index=lsq.load_store_queue[lsq.head].rob_index;
                    printf("**************************************\n");
                    printf("ROB index %d\n", cpu->memory.rob_index);
                    printf("**************************************\n");
                    cpu->lsq.load_store_queue[lsq.head].allocate=0;
                    cpu->memory.pc=lsq.load_store_queue[lsq.head].pc_value;
                    cpu->lsq.head=(cpu->lsq.head+1)%LSQ_SIZE;

                }
            }
        }
    }
}


void APEX_rob_commit_writeback(APEX_CPU *cpu){
    if(cpu->rob_commit_writeback.has_insn){
 

                    //wrrite the result into the destination  architecture register
                    cpu->arf.architectural_register_file[cpu->rob_commit_writeback.rd].value=
                    cpu->prf.physical_register[cpu->rob_commit_writeback.phy_rd].reg_value;
                    
                    if( cpu->rob_commit_writeback.opcode==OPCODE_ADD  ||
                        cpu->rob_commit_writeback.opcode==OPCODE_SUB  ||
                        cpu->rob_commit_writeback.opcode==OPCODE_ADDL ||
                        cpu->rob_commit_writeback.opcode==OPCODE_SUBL ||
                        cpu->rob_commit_writeback.opcode==OPCODE_MUL  ||
                        cpu->rob_commit_writeback.opcode==OPCODE_DIV){
                        
                        //positive flag
                        cpu->arf.architectural_register_file[cpu->rob_commit_writeback.rd].positive_flag=
                        cpu->prf.physical_register[cpu->rob_commit_writeback.phy_rd].positive_flag;
                        //zero flag
                        cpu->arf.architectural_register_file[cpu->rob_commit_writeback.rd].zero_flag=
                        cpu->prf.physical_register[cpu->rob_commit_writeback.phy_rd].zero_flag;
                        //ccr
                        cpu->arf.architectural_register_file[ARCHITECTURAL_REGISTERS_SIZE].value= cpu->rob_commit_writeback.rd;
                        cpu->arf.architectural_register_file[ARCHITECTURAL_REGISTERS_SIZE].positive_flag=
                        cpu->arf.architectural_register_file[cpu->rob_commit_writeback.rd].positive_flag;
                        cpu->arf.architectural_register_file[ARCHITECTURAL_REGISTERS_SIZE].zero_flag=
                        cpu->arf.architectural_register_file[cpu->rob_commit_writeback.rd].zero_flag;


                        
                        printf("MRA CCR=R[%d]\n",cpu->rob_commit_writeback.rd);

                        if(cpu->mri[ARCHITECTURAL_REGISTERS_SIZE]==cpu->rob_commit_writeback.phy_rd ){
                            cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].register_source=0;
                            printf("Updating RNT for CCR\n");
                        }
                    }

                        //free the physical register and add to prf free queue
                        push_free_physical_registers(&cpu->free_prf_q,cpu->rob_commit_writeback.phy_rd);

                        printf("ARF updates for R[%d]\n",cpu->rob_commit_writeback.rd);

                        if(cpu->mri[cpu->rob_commit_writeback.rd]==cpu->rob_commit_writeback.phy_rd){
                            cpu->rnt.rename_table[cpu->rob_commit_writeback.rd].register_source=0;
                            printf("Updating RNT for R[%d]\n",cpu->rob_commit_writeback.rd);
                        }
                        cpu->rob_commit_writeback.has_insn=FALSE;
    }

}

int  APEX_rob_commit(APEX_CPU *cpu){

        APEX_rob_commit_writeback(cpu);
        if(cpu->rob.reorder_buffer_queue[cpu->rob.head].is_allocated){
            //check the instruction type if it is register to register
            switch (cpu->rob.reorder_buffer_queue[cpu->rob.head].insn_type)
            {
            //if it is register to register
            case 1:
            case 0:
                if(cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode==OPCODE_HALT){
                    return TRUE;
                }
                else if(cpu->rob.reorder_buffer_queue[cpu->rob.head].status_bit){

                    //push the content to rob commt write back 
                    cpu->rob_commit_writeback.rd=cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address;
                    cpu->rob_commit_writeback.phy_rd=cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register;
                    cpu->rob_commit_writeback.opcode=cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode;
                    cpu->rob_commit_writeback.has_insn=TRUE;

                    printf("ROB commit: I[%d]\n", (cpu->rob.reorder_buffer_queue[cpu->rob.head].pc_value-4000)/4);
                    //free the rob entry and change the head
                    cpu->rob.reorder_buffer_queue[cpu->rob.head].is_allocated=0;
                    cpu->rob.head=(cpu->rob.head+1)%ROB_SIZE;
                }
                break;
        


                // if(cpu->rob.reorder_buffer_queue[cpu->rob.head].status_bit ){
                //     //wrrite the result into the destination  architecture register
                //     cpu->arf.architectural_register_file[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].value=
                //     cpu->prf.physical_register[cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register].reg_value;
                //     //positive flag
                //     cpu->arf.architectural_register_file[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].positive_flag=
                //     cpu->prf.physical_register[cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register].positive_flag;
                //     //zero flag
                //     cpu->arf.architectural_register_file[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].zero_flag=
                //     cpu->prf.physical_register[cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register].zero_flag;

                //     printf("ARF updates for R[%d]\n",cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address);

                //     //if insn is add addl sub subl or mul need to update the ccr register
                //     if(cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode==OPCODE_ADD ||
                //         cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode==OPCODE_ADDL ||
                //         cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode==OPCODE_SUB ||
                //         cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode==OPCODE_SUBL ||
                //         cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode==OPCODE_MUL){
                //         cpu->arf.architectural_register_file[ARCHITECTURAL_REGISTERS_SIZE].value= cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address;
                //         cpu->arf.architectural_register_file[ARCHITECTURAL_REGISTERS_SIZE].positive_flag=cpu->arf.architectural_register_file[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].positive_flag;
                //         cpu->arf.architectural_register_file[ARCHITECTURAL_REGISTERS_SIZE].zero_flag=cpu->arf.architectural_register_file[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].zero_flag;
                //         printf("MRA CCR=R%d\n",cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address);

                //         if(cpu->mri[ARCHITECTURAL_REGISTERS_SIZE]==cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register ){
                //             cpu->rnt.rename_table[ARCHITECTURAL_REGISTERS_SIZE].register_source=0;
                //             printf("Updating RNT for CCR\n");


                //         }
                //     }

                //     //rob.reorder_buffer_queue[cpu->rob.head].physical_register
                
                //     //free the physical register and add to prf free queue
                //     push_free_physical_registers(&cpu->free_prf_q,cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register);

                //     cpu->rnt.rename_table[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].register_source=0;

                //     printf("ROB commit: I[%d]\n", (cpu->rob.reorder_buffer_queue[cpu->rob.head].pc_value-4000)/4);
                //     //free the rob entry and change the head
                //     cpu->rob.reorder_buffer_queue[cpu->rob.head].is_allocated=0;
                //     cpu->rob.head=(cpu->rob.head+1)%ROB_SIZE;

                //     //update the rename table if the architectural register's most recent value is the same as the physical register
                //     if(cpu->mri[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address]==cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register){
                //         cpu->rnt.rename_table[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].register_source=0;
                //     }
                // }
                // break;
            //branch insn
            case 2:
                if(cpu->rob.reorder_buffer_queue[cpu->rob.head].status_bit ){
                    if(cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode==OPCODE_JALR){

                        cpu->rob_commit_writeback.rd=cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address;
                        cpu->rob_commit_writeback.phy_rd=cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register;
                        cpu->rob_commit_writeback.opcode=cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode;
                        cpu->rob_commit_writeback.has_insn=TRUE;
                    }



                        // //wrrite the result into the destination  architecture register
                        // cpu->arf.architectural_register_file[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].value=
                        // cpu->prf.physical_register[cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register].reg_value;
                        // //free the physical register and add to prf free queue
                        // push_free_physical_registers(&cpu->free_prf_q,cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register);
                        // cpu->rnt.rename_table[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].register_source=0;


                        //  //update the rename table if the architectural register's most recent value is the same as the physical register
                        // if(cpu->mri[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address]==cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register){
                        //     cpu->rnt.rename_table[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].register_source=0;
                        // }
                        printf("ROB commit: I[%d]\n", (cpu->rob.reorder_buffer_queue[cpu->rob.head].pc_value-4000)/4);   
                        //free the rob entry and change the head
                        cpu->rob.reorder_buffer_queue[cpu->rob.head].is_allocated=0;
                        cpu->rob.head=(cpu->rob.head+1)%ROB_SIZE;
                }
                break;
            //memory insn
            case 3:
                if(cpu->rob.reorder_buffer_queue[cpu->rob.head].status_bit){
                    //check if the memory insn is load or store
                    if(cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode==OPCODE_LOAD){

                        cpu->rob_commit_writeback.rd=cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address;
                        cpu->rob_commit_writeback.phy_rd=cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register;
                        cpu->rob_commit_writeback.opcode=cpu->rob.reorder_buffer_queue[cpu->rob.head].opcode;
                        cpu->rob_commit_writeback.has_insn=TRUE;

                        // cpu->arf.architectural_register_file[cpu->rob.reorder_buffer_queue[cpu->rob.head].destination_address].value=
                        // cpu->prf.physical_register[cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register].reg_value;
                        // cpu->free_prf_q.free_physical_registers[cpu->free_prf_q.tail]= cpu->rob.reorder_buffer_queue[cpu->rob.head].physical_register;
                    }
                    printf("ROB commit: I[%d]\n", (cpu->rob.reorder_buffer_queue[cpu->rob.head].pc_value-4000)/4);
                    cpu->rob.reorder_buffer_queue[cpu->rob.head].is_allocated=0;
                    cpu->rob.head=(cpu->rob.head+1)%ROB_SIZE;
                    }
                break;
            
            default:
                break;
            }
    }
    return 0;
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

        APEX_branch_writeback(cpu);
        APEX_int_writeback(cpu);  
        APEX_mul_writeback(cpu);  
        APEX_mem_writeback(cpu); 
         if (APEX_rob_commit(cpu))
         {
             /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
            print_reg_file(cpu);
            break;
        }

        APEX_bu_fwd(cpu);
        APEX_memory_fwd(cpu);
        APEX_int_fwd(cpu);
        APEX_mul_fwd(cpu);
        APEX_memory(cpu);
        push_lsq_instruction_to_memory_fu(cpu);
        APEX_process_iq(cpu);
        
        APEX_bu_fu(cpu);
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
        //print_lsq_entries(&cpu->lsq);
        print_reg_file(cpu);

        if(cpu->rob.reorder_buffer_queue[cpu->rob.head].is_allocated)
            printf("ROB head= I[%d] ", (cpu->rob.reorder_buffer_queue[cpu->rob.head].pc_value-4000)/4);
        int temp= (cpu->rob.tail-1+ROB_SIZE)%ROB_SIZE;
        if(cpu->rob.reorder_buffer_queue[temp].is_allocated)
            printf("ROB tail= I[%d] \n", (cpu->rob.reorder_buffer_queue[temp].pc_value-4000)/4);

        

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




//flush all instructions in the previous stages

void flush_instructions(APEX_CPU *cpu, int rob_index){

    printf("Flushing instructions\n");
    printf("---------------------\n");
    //flush all previous stages instructions

    // //flush fetch stage
    // cpu->fetch.has_insn=FALSE;
    //flush decode stage
    cpu->decode_rename.has_insn=FALSE;
    //flush rename dispatch stage
    cpu->rename_dispatch.has_insn=FALSE;
    //flush queue ebtry addition stage
    cpu->queue_entry.has_insn=FALSE;
    
    //flush rob entries from given rob_index till tail of rob entries 
    //check the condition < rob_index < rob_tail
    for (int i=rob_index+1;i!=cpu->rob.tail;i=(i+1)%ROB_SIZE){
        //issue queue entries invalidation");
       for (int j=0; j<ISSUE_QUEUE_SIZE;j++){
           if(cpu->iq.issue_queue[j].rob_index==i){
               printf("IQ- I[%d] \n,", (cpu->iq.issue_queue[j].pc_value-4000)/4);
               cpu->iq.issue_queue[j].is_allocated=0;
               break;
           }
       }
        int temp_lsq_index=-1;
       //delete lsq entry previous to given rob_index
        for (int j=cpu->lsq.head; j<cpu->lsq.tail;j=(j+1)%LSQ_SIZE){
            if(cpu->lsq.load_store_queue[j].rob_index==i){
                temp_lsq_index=j;
                break;
            }
        }
        if(temp_lsq_index>0){
            //mark all lsq entries after given temp_lsq_index as invalid
            for (int j=temp_lsq_index; j<cpu->lsq.tail;j=(j+1)%LSQ_SIZE){
                printf("LSQ- I[%d] \n,", (cpu->iq.issue_queue[j].pc_value-4000)/4);
                cpu->lsq.load_store_queue[j].allocate=0;
            }
            cpu->lsq.tail=temp_lsq_index;
        }
        if(cpu->rob.reorder_buffer_queue[i].destination_address!=100){
            //add that physical register to free list head
            cpu->free_prf_q.head=(cpu->free_prf_q.head-1+PHYSICAL_REGISTERS_SIZE)%PHYSICAL_REGISTERS_SIZE;
            cpu->free_prf_q.free_physical_registers[cpu->free_prf_q.head]=cpu->rob.reorder_buffer_queue[i].physical_register;

            printf("Physical register %d freed\n", cpu->rob.reorder_buffer_queue[i].physical_register);
            update_rename_table_with_backup(cpu,cpu->rob.reorder_buffer_queue[i].physical_register);
            set_mri_from_backup(cpu,cpu->rob.reorder_buffer_queue[i].physical_register);
        }

        //check every fu entry to chekc whether its rob index is equal to given rob_index

        if(cpu->int_fu.rob_index==i){
            cpu->int_fu.has_insn=FALSE;
        }
        if(cpu->mul1_fu.rob_index==i){
            cpu->mul1_fu.has_insn=FALSE;
        }
        if(cpu->mul2_fu.rob_index==i){
            cpu->mul2_fu.has_insn=FALSE;
        }
        if(cpu->mul3_fu.rob_index==i){
            cpu->mul3_fu.has_insn=FALSE;
        }
        if(cpu->mul4_fu.rob_index==i){
            cpu->mul4_fu.has_insn=FALSE;
        }
        if(cpu->memory.rob_index==i){
            cpu->memory.has_insn=FALSE;
        }
        //fwd buses
        if(cpu->int_fwd.rob_index==i){
            cpu->int_fwd.has_insn=FALSE;
        }
        if(cpu->mul_fwd.rob_index==i){
            cpu->mul_fwd.has_insn=FALSE;
        }
        if(cpu->memory_fwd.rob_index==i){
            cpu->memory_fwd.has_insn=FALSE;
        }
        if(cpu->int_writeback.rob_index==i){
            cpu->int_writeback.has_insn=FALSE;
        }
        if(cpu->mul_writeback.rob_index==i){
            cpu->mul_writeback.has_insn=FALSE;
        }
        if(cpu->mem_writeback.rob_index==i){
            cpu->mem_writeback.has_insn=FALSE;
        }
        //flush rob entry
        cpu->rob.reorder_buffer_queue[i].is_allocated=0;
    }
    cpu->rob.tail=rob_index+1;
    printf("---------------------\n");
}




//JUMP and JALR is branch instruction irrespective of the instruction type
//check if the given instruction is branch instruction
int is_branch_instruction(int opcode){
    if(opcode==OPCODE_BNP || opcode==OPCODE_BP || 
       opcode==OPCODE_BNZ || opcode==OPCODE_BZ || opcode==OPCODE_JUMP || opcode==OPCODE_JALR || opcode==OPCODE_RET){
        return TRUE;
    }
    return FALSE;
}



//backup for rename table
void create_rename_table_backup( APEX_CPU *cpu){
    for (int i=0;i<ARCHITECTURAL_REGISTERS_SIZE+1;i++){
        cpu->rnt_bkp.rename_table[i].mapped_to_physical_register=cpu->rnt.rename_table[i].mapped_to_physical_register;
        cpu->rnt_bkp.rename_table[i].register_source=cpu->rnt.rename_table[i].register_source;
    }
}

//backup for mri
void create_mri_backup( APEX_CPU *cpu){
    for (int i=0;i<ARCHITECTURAL_REGISTERS_SIZE;i++){    
        cpu->mri_bkp[i]=cpu->mri[i];
    }
}


//check rename table for physical register and update it with rename table backup

void update_rename_table_with_backup( APEX_CPU *cpu, int physical_register_address){
    for (int i=0;i<ARCHITECTURAL_REGISTERS_SIZE;i++){
        if(cpu->rnt.rename_table[i].register_source==1){
            if(cpu->rnt.rename_table[i].mapped_to_physical_register==physical_register_address){
                cpu->rnt.rename_table[i].mapped_to_physical_register=cpu->rnt_bkp.rename_table[i].mapped_to_physical_register;
                cpu->rnt.rename_table[i].register_source= 1;
                if(check_free_physical_register(cpu,cpu->rnt.rename_table[i].mapped_to_physical_register)){
                    cpu->rnt.rename_table[i].register_source= 0;
                }
                
                printf("Rename table updated with backup: R[%d]=P[%d]\n",i,cpu->rnt.rename_table[i].mapped_to_physical_register);
            }
        }
    }
}


void set_mri_from_backup( APEX_CPU *cpu, int physical_register_address){
    cpu->mri_bkp[physical_register_address]=cpu->mri[physical_register_address];
}



int check_free_physical_register(APEX_CPU *cpu, int physical_register_address){

    //check free physical register queue
        for (int i=cpu->free_prf_q.head;i!=cpu->free_prf_q.tail;i=(i+1)%PHYSICAL_REGISTERS_SIZE){
            if(cpu->free_prf_q.free_physical_registers[i]==physical_register_address){
                return TRUE;
            }
        }
        return FALSE;
}