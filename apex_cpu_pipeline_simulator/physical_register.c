/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */

#include "physical_register.h"
#include<stdio.h>


void print_prf_q(free_physical_registers_queue *a){
    int temp_head=a->head;
    int temp_tail=a->tail;
    int i=temp_head;
    while(i!=temp_tail){
        printf("%d\t,",a->free_physical_registers[i]);
        i=(i+1)%PHYSICAL_REGISTERS_SIZE;
    }
    printf("%d\n",a->free_physical_registers[temp_tail]);
}

int pop_free_physical_registers(free_physical_registers_queue *fpq){
    if(fpq->is_empty){
        return -1;
    }
    else{
        int temp= fpq->head;
        if (fpq->head==fpq->tail)
            fpq->is_empty=1;
        fpq->head=(fpq->head+1)%PHYSICAL_REGISTERS_SIZE;
        return temp;
    }
}

void push_free_physical_registers(free_physical_registers_queue *fpq, int physical_register){
    printf("PRF reg Freed: P[%d]\n",physical_register);
    fpq->tail=(fpq->tail+1)%PHYSICAL_REGISTERS_SIZE;
    fpq->free_physical_registers[fpq->tail]=physical_register;
    return;
}