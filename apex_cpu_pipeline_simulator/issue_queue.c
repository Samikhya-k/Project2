/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
////////////////////////ISSUE_QUEUE////////////////////////////////////
#include  "issue_queue.h"

int issue_buffer_index_available(issue_queue_buffer *iq){
     for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
        if(iq->issue_queue[i].is_allocated==0){
            return i;
        }
    }
    return -1;
}

void iq_entry_addition(issue_queue_buffer *iq,issue_queue_entry *iq_entry,int iq_index){
    iq->issue_queue[iq_index].is_allocated=1;
    iq->issue_queue[iq_index].dest_tag=iq_entry->dest_tag;
    iq->issue_queue[iq_index].FU=iq_entry->FU;
    iq->issue_queue[iq_index].immediate_literal=iq_entry->immediate_literal;
    iq->issue_queue[iq_index].src1_tag=iq_entry->src1_tag;
    iq->issue_queue[iq_index].src1_valid=iq_entry->src1_valid;
    iq->issue_queue[iq_index].src1_value=iq_entry->src1_value;
    iq->issue_queue[iq_index].src2_tag=iq_entry->src2_tag;
    iq->issue_queue[iq_index].src2_valid=iq_entry->src2_valid;
    iq->issue_queue[iq_index].src2_value=iq_entry->src2_value;
    iq->issue_queue[iq_index].lsq_index=iq_entry->lsq_index;
    iq->issue_queue[iq_index].rob_index=iq_entry->rob_index;
    iq->issue_queue[iq_index].pc_value=iq_entry->pc_value;
    iq->issue_queue[iq_index].counter=iq_entry->counter;
    iq->issue_queue[iq_index].opcode=iq_entry->opcode;
}


void print_iq_indexes(issue_queue_buffer *iq){
    issue_queue_entry *temp_iq= iq->issue_queue;
    printf("allocated indexes are:");
    for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
        if(temp_iq[i].is_allocated){
            printf("%d\t",i);
        }
    }
     printf("\n");
}

void print_iq_entries(issue_queue_buffer *iq){
    issue_queue_entry *temp_iq= iq->issue_queue;
    int count=0;
    printf("************************\n");
    for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
        if(temp_iq[i].is_allocated){
            count=count+1;;
        }
    }
    printf("No.of issue queue entries:%d",count);
    printf("IQ contents are as below \n:");
    for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
        //print content of iq
        if(temp_iq[i].is_allocated){
            printf("index:%d\t |",i);
            printf("allocate:%d\t |",temp_iq[i].is_allocated);
            printf("FU:%d\t |",temp_iq[i].FU);
            printf("src1_tag:%d\t |",temp_iq[i].src1_tag);
            printf("src1_value:%d\t |",temp_iq[i].src1_value);
            printf("src1_valid:%d\t |",temp_iq[i].src1_valid);
            printf("src2_tag:%d\t |",temp_iq[i].src2_tag);
            printf("src2_value:%d\t |",temp_iq[i].src2_value);
            printf("src2_valid:%d\t |",temp_iq[i].src2_valid);
            printf("immediate_literal:%d\t|",temp_iq[i].immediate_literal);
            printf("dest_tag:%d\n",temp_iq[i].dest_tag);
            printf("counter:%d\n",temp_iq[i].counter);
        }
    }
    printf("************************\n");
}




int get_iq_index_fu(issue_queue_buffer *iq, int fu){
    int temp_index=-1;
    int counter=-1;
    for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
        if(iq->issue_queue[i].is_allocated ==1 && iq->issue_queue[i].FU==fu){
            iq->issue_queue[i].counter++;
            if (iq->issue_queue[i].src2_valid ==1 && iq->issue_queue[i].src1_valid==1)
            {   
                if(iq->issue_queue[i].counter>counter){
                    temp_index=i;
                    counter=iq->issue_queue[i].counter;
                }
            }
        }
    }
    return temp_index;
}

