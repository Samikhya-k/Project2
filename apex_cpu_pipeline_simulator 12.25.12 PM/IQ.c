#include<stdio.h>
#define ISSUE_QUEUE_SIZE 6
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





int main()
{

    issue_queue_buffer iq;
    for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
        iq.issue_queue[i].is_allocated=0;
    }
    iq.issue_queue[0].is_allocated=1;
    iq.issue_queue[1].is_allocated=1;
    
    for(int i=0;i<ISSUE_QUEUE_SIZE;i++){
        printf("inside\n");
        if(!iq.issue_queue[i].is_allocated){
            printf("Issue buffer is free at index %d\n",i);
            return i;
        }
    }


}
