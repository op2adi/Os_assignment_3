/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include <inttypes.h>
#include <stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/mman.h>

/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
#define PAGE_SIZE 4096
typedef struct main_node{
    
    struct main_node* prev ;
    int pages ; 
    unsigned long virtual_start ; 
    unsigned long virtual_end ;    
    void* phy_addr ;
    struct subchain_node* subchain ;
    struct main_node* next ;

}main_node;

typedef struct subchain_node{
    struct subchain_node* prev ;
    int type ; // 0 for hole 1 for process .. 
    unsigned long virtual_start ; 
    unsigned long virtual_end ;
    void* phys_addr ;    
    size_t size ; 
    struct subchain_node* next ;
      
}subchain_node; 



unsigned long vir_address ;
struct main_node* head = NULL;
struct main_node* current_pointer = NULL;
unsigned long  main_node_size = sizeof(struct main_node) ;
unsigned long subchain_size = sizeof(struct subchain_node) ;
struct subchain_node* current_pointer_subchain = NULL;
struct main_node* init_main = NULL;
struct subchain_node* init_sub = NULL;
void* segments[200000000] ;
int no_of_seg = 0 ;
int segments_size[200000000] ;
int chain_count = 1;
int subchain_count_ls = 1 ;

// helper functions 


int rounded_val(int x){
    if(x%4096 == 0){
        return x ;
    }
    else{
        return ((x/4096) + 1) *4096 ; 
    }


}


subchain_node* create_new_subchain(subchain_node* new_sub){
    //subchain_node* new_sub = NULL ;
    if((unsigned long)current_pointer_subchain + subchain_size <= (unsigned long)init_sub + PAGE_SIZE && subchain_count_ls%(rounded_val(PAGE_SIZE)/subchain_size  - 1 ) != 0){
        new_sub =(subchain_node* )((unsigned char*)current_pointer_subchain+subchain_size) ;
        current_pointer_subchain = new_sub ;
        subchain_count_ls ++ ;
        //printf("%d\n",subchain_count_ls) ;
        return new_sub ;
    }
    else {
        //printf("here\n") ;
        new_sub =(subchain_node* )mmap(NULL,PAGE_SIZE*1,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE ,-1,0) ;
        if(new_sub == MAP_FAILED){
            perror("mmap failed");
        }
        subchain_count_ls++ ;
       // printf("%d\n",subchain_count_ls);
        segments[no_of_seg] = (void*)new_sub ;
        segments_size[no_of_seg++] = PAGE_SIZE ;
        init_sub = new_sub ;
        current_pointer_subchain = new_sub ;
        new_sub =(subchain_node* )((unsigned char*)current_pointer_subchain+subchain_size) ;
        current_pointer_subchain = new_sub ; 
        return new_sub ;
    }
}

main_node* create_new_main_node(main_node* node){
    
   // printf("this is first term: %u  \n",(unsigned long)current_pointer+main_node_size) ;
   // printf("this is second term: %u  \n",(unsigned long)init_main+PAGE_SIZE) ;
    //main_node* node = NULL ;
    if((unsigned long)current_pointer+main_node_size < (unsigned long)init_main+(unsigned long)PAGE_SIZE && chain_count%(rounded_val(PAGE_SIZE)/main_node_size -1)!=0 ){
        node = (main_node*)((unsigned char*)current_pointer + main_node_size);
        current_pointer = node;
        chain_count ++ ; 
       // printf("%d\n",chain_count) ;
        //printf("gg\n") ;
        return node ;
    }
    else{
      //printf("here\n") ;
        node = (main_node*)mmap(NULL, PAGE_SIZE*1, PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE ,-1,0) ;
        if(node == MAP_FAILED){
            perror("mmap failed");
        }
        chain_count++ ;
    //    printf("%d" , chain_count) ;
        segments[no_of_seg] = (void*)node ;
        segments_size[no_of_seg++] = PAGE_SIZE ;
        init_main = node; 
        current_pointer = node; 
        node = (main_node*)((unsigned char*)current_pointer + main_node_size);
        current_pointer = node ;
        return node ;        
    }
}


void merge_holes(subchain_node* subchain){
   
    if(subchain->next != NULL){
        while(subchain->next->type == 0){
            subchain->size = subchain->size + subchain->next->size ;

            subchain->virtual_end = subchain->next->virtual_end ;
            
            subchain->next = subchain->next->next ;
            if(subchain->next == NULL){
                break;
            }
        }
    }

}

void finding_subchain(void*v_ptr){
    main_node* main_start = head->next ;
    while(main_start!=NULL){
        subchain_node* subchain_start = main_start->subchain ;
        while(subchain_start!=NULL){
           // printf("%u %u \n",(unsigned long)v_ptr ,subchain_start->virtual_start ) ;
            if((unsigned long)v_ptr == subchain_start->virtual_start){
                subchain_start->type = 0 ;
                return ;
            }
            subchain_start = subchain_start->next ;
        }
        main_start = main_start->next ;
    }

}




/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_init(){
    vir_address = 1000 ;
    head = (main_node*)mmap(NULL, PAGE_SIZE*1, PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE , -1, 0);
    if(head == MAP_FAILED){
        perror("mmap failed");
    }
    segments[no_of_seg] = (void*)head ;
    segments_size[no_of_seg++] = PAGE_SIZE ;
    head->next = NULL ;
    current_pointer=head ;
    init_main = head ;
   // printf("%d\n" , rounded_val(PAGE_SIZE)) ;
    //printf("%d\n",rounded_val(PAGE_SIZE)/main_node_size) ;
    //printf("%d\n",rounded_val(PAGE_SIZE)/main_node_size  - 1 ) ;
//    printf("%lu",(unsigned long)current_pointer) ;    

   // printf("%lu\n",sizeof(subchain_node));
  //  printf("%lu\n",sizeof(main_node));
}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish(){
    for(int i = 0;i<no_of_seg ;i++){
        if(munmap(segments[i],segments_size[i])==-1) 
            perror("munmap failed");
    }
}


/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 
void* mems_malloc(size_t size){
    
    main_node* curr_main = head->next ;
    while(curr_main != NULL){
        subchain_node*  curr_sub = curr_main->subchain ;
        while(curr_sub!=NULL){
            curr_sub->virtual_end = curr_sub->virtual_start + curr_sub->size - 1;
            curr_sub = curr_sub->next ;
        }
        curr_main = curr_main->next ;
    }
    
    
    
    int flag = 0 ; 
    int n = 1 ; 
    while(n*PAGE_SIZE < size){
        n++ ;
    }
    
    //printf("mems_mall\n");

    if(size == 0){
        printf("please give a size\n") ;
        return NULL;
    }
    else{
        //printf("etre\n");
        if(head->next != NULL){
            //printf("head != NULL\n") ;
            main_node* main_chain = head ;
            //printf("%lu\n" , main_chain->phy_addr) ;
            while (main_chain!=NULL) {
                subchain_node* sub_chain = main_chain->subchain;
                //need to increase the current_pointer // 
                while (sub_chain!=NULL) {
                    if(sub_chain->type == 0 && sub_chain->size > size){
                        subchain_node* new_sub = NULL ;
                        new_sub = create_new_subchain(new_sub) ;
                        unsigned long virt_end = sub_chain->virtual_end ;
                        size_t sub_size = sub_chain->size; 
                        sub_chain->virtual_end = sub_chain->virtual_start + size - 1 ;
                        sub_chain->size = size ;
                        sub_chain->type = 1 ;
                        sub_chain->next = new_sub ;
                        new_sub->prev = sub_chain ;
                        new_sub->type = 0 ;
                        new_sub->phys_addr = sub_chain->phys_addr + sub_chain->size ;
                        new_sub->size = sub_size - size ;
                        new_sub->virtual_start = sub_chain->virtual_start + size ;
                        new_sub->virtual_end = virt_end ;
                        new_sub->next = NULL;
                        flag = 1 ;
                        return (void*) sub_chain->virtual_start ;

                    }
                    else if (sub_chain->type == 0 && sub_chain->size==size){
                        //printf("dqwadwq") ;
                        sub_chain->type = 1 ;
                        flag = 1 ;
                        return (void*)sub_chain->virtual_start;
                    }
                    sub_chain= sub_chain->next ;
                }
                main_chain = main_chain->next ;
            }
            if(flag == 0){
                main_node* new_main = NULL ;
                new_main = create_new_main_node(new_main);
                main_node* main_chain = head ;
                while(main_chain->next != NULL){
                    main_chain = main_chain->next ;
                }    
                
                void* temp = mmap(NULL,n*PAGE_SIZE,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE , -1, 0);
                new_main->pages = n ;
                new_main->phy_addr = temp;
                if(new_main->phy_addr == MAP_FAILED){
                    perror("mmap failed") ;
                }
                segments[no_of_seg] = new_main->phy_addr ;
                segments_size[no_of_seg++] = n*PAGE_SIZE ;
                //printf("%zu\n",new_main->phy_addr);
                 // reach the last main node and then add a new main node and do the things  .. 
                main_chain->next = new_main ;
                new_main->prev = main_chain ;
                new_main->virtual_start = new_main->prev->virtual_end + 1 ;
                new_main->virtual_end = new_main->virtual_start + n*PAGE_SIZE - 1 ;
                subchain_node* new_subchain = NULL ;
                new_subchain = create_new_subchain(new_subchain) ;
                new_main->subchain = new_subchain ;
                new_main->next = NULL;
                new_subchain->size = n*PAGE_SIZE ;
                new_subchain->phys_addr = new_main->phy_addr  ;
                new_subchain->type = 0;
                new_subchain->size = n*PAGE_SIZE ;
                new_subchain->virtual_start = new_main->virtual_start ;
                new_subchain->virtual_end = new_main->virtual_end ;
                
                if(new_subchain->size > size){
                    subchain_node* newest_subchain = NULL;
                    newest_subchain = create_new_subchain(newest_subchain) ;
                    new_subchain->next = newest_subchain ;
                    newest_subchain->prev = new_subchain ; 
                    new_subchain->virtual_end = new_subchain->virtual_start + size - 1 ;
                    new_subchain->size = size ; 
                    new_subchain->type = 1 ;
                    newest_subchain->phys_addr = newest_subchain->prev->phys_addr + size  ;
                    newest_subchain->virtual_start = newest_subchain->prev->virtual_start + size ;
                    newest_subchain->size = new_main->pages*PAGE_SIZE - size ; 
                    newest_subchain->type = 0 ;
                    newest_subchain->next = NULL;
                    return (void*) new_subchain->virtual_start ;
                } 
                else if( new_subchain->size == size){
                    new_subchain->type = 1 ;
                    return (void*)new_subchain->virtual_start ;
                }
            }
        }
        else{
            //printf("dfkoaed\n");
           // printf("%lu %lu\n",current_pointer, main_node_size) ;
           //printf("%c" , current_pointer) ;
            main_node* first_node = NULL ;
            
            first_node = (main_node* )((unsigned char*)current_pointer+main_node_size);
            
            // else{
            //     first_node = mmap(NULL,PAGE_SIZE ,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE ,-1,0) ;
            //     init_main = first_node ;
            //     current_pointer = first_node ;  // will do this for the adding of main node also when flag = 0 ;
            // }
           // printf("%lu\n",first_node) ;
            current_pointer = first_node ;
            first_node->phy_addr = mmap(NULL,n*PAGE_SIZE, PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE ,-1,0) ;
            if(first_node->phy_addr == MAP_FAILED){
                perror("mmap failed") ;
            }
            segments[no_of_seg] = first_node->phy_addr ;
            segments_size[no_of_seg++] = n*PAGE_SIZE ;
            //printf("%zu\n",first_node->phy_addr) ;
            //printf("fefe");     
            head->next = first_node ;
            first_node->prev = head ;
            first_node->pages = n ;
            first_node->virtual_start = vir_address;    
            first_node->virtual_end = first_node-> virtual_start+n*PAGE_SIZE - 1 ;
            vir_address+=n*PAGE_SIZE ;
            subchain_node* subchain = (subchain_node*)mmap(NULL,PAGE_SIZE*1,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE ,-1,0) ;
            if(subchain == MAP_FAILED){
                perror("mmap failed") ;
            }
            segments[no_of_seg] = (void*)subchain ;
            segments_size[no_of_seg++] = PAGE_SIZE ;
            first_node->next = NULL;
            first_node->subchain = subchain ;
            subchain->phys_addr = first_node->phy_addr  ;
            subchain->size = n*PAGE_SIZE ;
            subchain->type = 0 ;
            subchain->virtual_start = first_node->virtual_start ;
            subchain->virtual_end = first_node->virtual_end ; 
            subchain->next = NULL;
          //  printf("%lu\n",subchain->virtual_start);
          //  printf("%lu\n",subchain->virtual_end) ;
            current_pointer_subchain = subchain ;
            init_sub = subchain ; 
            if(subchain->size > size){
                subchain_node* new_sub = NULL ;
                new_sub = create_new_subchain(new_sub) ;
                unsigned long virt_end = subchain->virtual_end ;
                subchain->virtual_end = subchain->virtual_start+ size - 1 ;  
                subchain->size = size ;
                subchain->type = 1;
                subchain->next = new_sub ;
                new_sub->prev = subchain ;
                new_sub->size = (first_node->pages * PAGE_SIZE) - size ;
                new_sub->virtual_start = subchain->virtual_start + size ;
                new_sub->virtual_end = virt_end ;
                new_sub->type = 0;
                new_sub->next = NULL;
                new_sub->phys_addr = subchain->phys_addr + subchain->size ;
                return (void*)subchain->virtual_start ;
            }
            else if(subchain->size == size){
                subchain->type = 1 ;
                return (void*)subchain->virtual_start ;
            }
        }   
    }
}


/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats(){
    printf("---------MeMS SYSTEM STATS------------\n");
    main_node* curr_main = head->next ;
    while(curr_main != NULL){
        subchain_node*  curr_sub = curr_main->subchain ;
        while(curr_sub!=NULL){
            curr_sub->virtual_end = curr_sub->virtual_start + curr_sub->size - 1;
            curr_sub = curr_sub->next ;
        }
        curr_main = curr_main->next ;
    }
    int counter = 0  ; 
    int arr[200000000] ;
    int num = 0 ;
    int pages = 0 ;
    int mem_unused = 0;
    int main_chain_len = 0 ;
    if(head != NULL){
    main_node* curent_main_node = head->next ;
    while(curent_main_node != NULL){
        subchain_node* current_subchain_node = curent_main_node->subchain ;
        while(current_subchain_node!=NULL){
            if(current_subchain_node->type == 0){
                mem_unused += current_subchain_node->size ;
            }
            num++ ;
            current_subchain_node = current_subchain_node->next ;
        }
        pages += curent_main_node->pages ;
        arr[counter++] = num ;
        num = 0;
        curent_main_node = curent_main_node->next ;
    }

    main_node* current_main = head->next;
    while(current_main!=NULL){
        printf("MAIN[%lu:%lu]->",current_main->virtual_start,current_main->virtual_end) ;
        subchain_node* current_subchain = current_main->subchain ;
        while(current_subchain!=NULL){
            if(current_subchain->type == 0){
                printf("H[%lu:%lu]<->",current_subchain->virtual_start , current_subchain->virtual_end);
            }
            else{
                printf("P[%lu:%lu]<->",current_subchain->virtual_start , current_subchain->virtual_end);
            }
            current_subchain = current_subchain->next ;
        }
        printf("NULL\n") ;
        main_chain_len+=1 ;
        current_main = current_main->next ;
    }
    }
    printf("Pages used:    %d\n",pages) ;
    printf("Space unused:  %d\n",mem_unused) ;
    printf("Main Chain Length:    %d\n",main_chain_len) ;
    printf("Sub-Chain Length array: [") ;
    for(int i = 0 ; i < counter ; i++){
        printf("%d, ",arr[i]);
    }
    printf("]\n");
    printf("----------------------------------------\n");
}


/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed     ptr (MeMS virtual address).
*/
void *mems_get(void*v_ptr){
    
    main_node* current_main = head->next ;
    // while(current_main != NULL){
    //     printf("%zu\n" , current_main->phy_addr) ;

    //     current_main = current_main->next ;
    // }
    void* phys_addr_for_vptr = NULL ;
    //return head->next->phy_addr ;
    while(current_main!=NULL){
        if(current_main->virtual_start<=(unsigned long)v_ptr && current_main->virtual_end>=(unsigned long)v_ptr){
            phys_addr_for_vptr = (void*)((unsigned long)current_main->phy_addr + (unsigned long) v_ptr - current_main->virtual_start) ;
            break; 
        }
        current_main = current_main->next ;
    }

    return phys_addr_for_vptr;


}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/
void mems_free(void *v_ptr){
    
    finding_subchain(v_ptr);


    main_node* curr_main = head->next ;
    while(curr_main!=NULL){
        subchain_node* curr_subchain = curr_main->subchain ;
        while(curr_subchain != NULL){
            if(curr_subchain->type == 0){
                merge_holes(curr_subchain) ;

            }
            curr_subchain = curr_subchain->next ;
        }
        curr_main = curr_main->next ;
    }
}