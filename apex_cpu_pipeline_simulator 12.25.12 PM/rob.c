/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
 #include"rob.h"
 #include<stdio.h>



int reorder_buffer_available(reorder_buffer *rob){
    if(rob->is_full){
        return -1;
    }
    else{
        return rob->tail;
    }

}
int reorder_buffer_entry_addition_to_queue(reorder_buffer *rob, reorder_buffer_entry * rob_entry){
    rob->reorder_buffer_queue[rob->tail].pc_value=rob_entry->pc_value;
    rob->reorder_buffer_queue[rob->tail].destination_address=rob_entry->destination_address;
    rob->reorder_buffer_queue[rob->tail].result_value=rob_entry->result_value;
    rob->reorder_buffer_queue[rob->tail].store_value=rob_entry->store_value;
    rob->reorder_buffer_queue[rob->tail].store_value_valid=rob_entry->store_value_valid;
    rob->reorder_buffer_queue[rob->tail].status_bit=rob_entry->status_bit;
    rob->reorder_buffer_queue[rob->tail].insn_type=rob_entry->insn_type;
    rob->reorder_buffer_queue[rob->tail].pc_value=rob_entry->pc_value;
    rob->tail=(rob->tail+1)%ROB_SIZE;
    int rob_index=rob->tail;
    rob->tail++;
    if(rob->tail==ROB_SIZE){
        rob->tail=0;
        rob->is_full=1;
    }
    return rob_index;
}


void print_rob_entries(reorder_buffer *rob){
    int i;
    i=rob->head;
    printf("ROB contents are as below:\n");
    printf("***********************\n");
    while(i!=rob->tail){
        printf("pc_value: %d |",rob->reorder_buffer_queue[i].pc_value);
        printf("destination_address: %d |",rob->reorder_buffer_queue[i].destination_address);
        printf("result_value: %d |",rob->reorder_buffer_queue[i].result_value);
        printf("store_value: %d |",rob->reorder_buffer_queue[i].store_value);
        printf("store_value_valid: %d\n",rob->reorder_buffer_queue[i].store_value_valid);
        printf("status_bit: %d|",rob->reorder_buffer_queue[i].status_bit);
        printf("insn_type: %d\n",rob->reorder_buffer_queue[i].insn_type);
        i=(i+1)%ROB_SIZE;
    }
    printf("***********************\n");
}
