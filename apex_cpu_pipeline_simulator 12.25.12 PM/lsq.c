#include "lsq.h"
#include  <stdio.h>

int lsq_index_available(load_store_queue *lsq){
    if(lsq->is_full)
        return -1;
    else
        return lsq->tail;
}

void lsq_entry_addition_to_queue(load_store_queue *lsq, load_store_queue_entry * lsq_entry){
    lsq->load_store_queue[lsq->tail].mem_address = lsq_entry->mem_address;
    lsq->load_store_queue[lsq->tail].address_valid = lsq_entry->address_valid;
    lsq->load_store_queue[lsq->tail].allocate = lsq_entry->allocate;
    lsq->load_store_queue[lsq->tail].instruction_type = lsq_entry->instruction_type;
    lsq->load_store_queue[lsq->tail].destination_address_for_load = lsq_entry->destination_address_for_load;
    lsq->load_store_queue[lsq->tail].data_ready = lsq_entry->data_ready;
    lsq->load_store_queue[lsq->tail].src1_store = lsq_entry->src1_store;
    lsq->load_store_queue[lsq->tail].value_to_be_stored = lsq_entry->value_to_be_stored;
    lsq->tail = (lsq->tail + 1) % LSQ_SIZE;
    if(lsq->tail == lsq->head)
        lsq->is_full = 1;

}


void print_lsq_entries(load_store_queue *lsq){
    int temp = lsq->head;
    int temp_tail = lsq->tail;
    while(temp!=temp_tail){
        printf("mem_address: %d |", lsq->load_store_queue[temp].mem_address);
        printf("address_valid: %d |", lsq->load_store_queue[temp].address_valid);
        printf("allocate: %d |", lsq->load_store_queue[temp].allocate);
        printf("instruction_type: %d |", lsq->load_store_queue[temp].instruction_type);
        printf("destination_address_for_load: %d |", lsq->load_store_queue[temp].destination_address_for_load);
        printf("data_ready: %d |", lsq->load_store_queue[temp].data_ready);
        printf("src1_store: %d |", lsq->load_store_queue[temp].src1_store);
        printf("value_to_be_stored: %d \n", lsq->load_store_queue[temp].value_to_be_stored);
        temp = (temp + 1) % LSQ_SIZE;
    }
}


