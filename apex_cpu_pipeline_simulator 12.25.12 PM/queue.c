#include<stdio.h>

typedef struct free_physical_registers_queue
{
    int head;
    int tail;
    int free_physical_registers[10];
    int is_empty;
}free_physical_registers_queue;

void print_prf_q(free_physical_registers_queue *a){
    int temp_head=a->head;
    int temp_tail=a->tail;
    int i=temp_head;
    while(i!=temp_tail){
        printf("%d\t,",a->free_physical_registers[i]);
        i=(i+1)%10;
    }
    printf("%d",a->free_physical_registers[temp_tail]);
    printf("\n");
}


int pop_free_physical_registers(free_physical_registers_queue *fpq){
    if(fpq->is_empty){
        return -1;
    }
    else{
        int temp= fpq->head;
        fpq->head=(fpq->head+1)%10;
        return temp;
    }
}

void push_free_physical_registers(free_physical_registers_queue *fpq, int physical_register){
    fpq->tail=(fpq->tail+1)%10;
    fpq->free_physical_registers[fpq->tail]=physical_register;
    return;
}

int main(){
    free_physical_registers_queue a;
    a.head=0;
    a.tail=9;
    for(int i=0;i<10;i++){
        a.free_physical_registers[i]=i;
    }
    a.is_empty=0;


    printf("elemnt poppped =%d \n",pop_free_physical_registers(&a));
    print_prf_q(&a);
    push_free_physical_registers(&a,10);
    print_prf_q(&a);
    printf("elemnt poppped =%d \n",pop_free_physical_registers(&a));
    push_free_physical_registers(&a,10);
    print_prf_q(&a);  
}