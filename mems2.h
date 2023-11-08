/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include <stddef.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/mman.h>


/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
#define PAGE_SIZE 4096

struct mems_subchain_node {
    size_t start;
    size_t end;
    size_t size;
    int is_hole;
    struct mems_subchain_node* next;
    struct mems_subchain_node* prev;
};

struct mems_mainchain_node {
    size_t total_size;
    size_t start;
    size_t end;
    int num_pages;
    struct mems_subchain_node* subchain_head;
    struct mems_mainchain_node* next;
    struct mems_mainchain_node* prev;
    void * address;
};
// Global variables
struct mems_mainchain_node* mems_free_list = NULL;
void* mems_heap_start = NULL;
/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_init(){
// Initialize any global variables or data structures
    mems_free_list = NULL;
    
    // Allocate an initial block of memory from the OS using mmap
    size_t total_size = PAGE_SIZE;
    mems_heap_start = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    
    if (mems_heap_start != MAP_FAILED) {
        struct mems_mainchain_node* initial_block = (struct mems_mainchain_node*)mems_heap_start;
        initial_block->total_size = total_size;
        initial_block->start=1000;
        initial_block->end=5095;
        initial_block->subchain_head = (struct mems_subchain_node*)((char*)initial_block + sizeof(struct mems_mainchain_node));
        initial_block->address=&mems_heap_start;
        initial_block->subchain_head->size = total_size - sizeof(struct mems_subchain_node);
        initial_block->num_pages=1;
        initial_block->address=mems_heap_start;
        initial_block->subchain_head->is_hole = 1;
        initial_block->subchain_head->next = NULL;
        initial_block->subchain_head->prev = NULL;
        initial_block->subchain_head->start=1000;
        initial_block->subchain_head->end=5096;
        initial_block->next = NULL;
        initial_block->prev = NULL;
        mems_free_list = initial_block;
    }
}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish(){
    struct mems_mainchain_node* block = mems_free_list;
    struct mems_mainchain_node* next_block;

    while (block != NULL) {
        next_block = block->next;
        munmap(block, block->total_size);
        block = next_block;
    }
    mems_free_list = NULL;
    mems_heap_start = NULL;
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
void* mems_malloc(size_t size){if (size == 0) {
        return NULL;  // Handle zero-size allocation
    }

    struct mems_mainchain_node* block = mems_free_list;

    while (block != NULL) {
        struct mems_subchain_node* segment = block->subchain_head;
        int bc = 0;
        while (segment != NULL) {
            bc++;
            if (segment->is_hole && segment->size >= size) {
                if (segment->size == size) {
                    segment->is_hole=0;
                    return (void *)segment->start;
                }
                else{
                    struct mems_subchain_node* initial_block = (struct mems_subchain_node*)((char*)mems_heap_start+size);
                    segment->next=initial_block;
                    initial_block->next=NULL;
                    initial_block->start=segment->start+size;
                    initial_block->size=segment->size-size;
                    initial_block->end=segment->end;
                    initial_block->is_hole=1;
                    segment->is_hole=0;
                    segment->size=size;
                    segment->end=segment->start+size;
                    mems_heap_start+=size;
                    return (void *)segment->start;
                }
    }
    segment=segment->next;
        }
        block=block->next;
        // printf("1\n");
    }


    // If no suitable segment is found, request more memory from the OS
    size_t requested_size = size ;
    size_t num_pages = 0;
    while (num_pages*PAGE_SIZE<requested_size) {
        num_pages++;
    }
    size_t total_size = num_pages * PAGE_SIZE;
    // printf("%zu\n",num_pages);
    // Use mmap to allocate more memory
    void* new_bloc = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    struct mems_mainchain_node* new_block = (struct mems_mainchain_node*)new_bloc;
    block = mems_free_list;
    while(block->next!=NULL){
        block=block->next;
    }
    struct mems_subchain_node* b = block->subchain_head;
    while (b->next!=NULL) {
        b=b->next;
    }
    if (new_block != MAP_FAILED) {
        new_block->start=b->end+1;
        new_block->end=num_pages*PAGE_SIZE+block->end;
        new_block->address=&new_block;
        new_block->total_size = total_size;
        new_block->subchain_head = (struct mems_subchain_node*)((char*)new_block + sizeof(struct mems_mainchain_node));
        new_block->subchain_head->size = total_size - sizeof(struct mems_subchain_node);
        // printf("1\n");
        new_block->subchain_head->is_hole = 1;
        // printf("1\n");
        new_block->subchain_head->next = NULL;
        new_block->subchain_head->prev = NULL;
        new_block->subchain_head->start=b->end;
        new_block->subchain_head->end=b->end+num_pages*4096;
        new_block->num_pages=num_pages;
        // Add the new block to the free list
        new_block->next = NULL;
        new_block->prev = block;
        block->next=new_block;
        mems_heap_start = (void*)((char*)new_block + sizeof(struct mems_mainchain_node) + sizeof(struct mems_subchain_node));

        // if (mems_free_list != NULL) {
        //     mems_free_list->next = block;
        // }
        // mems_free_list = new_block;

        // Try to allocate from the new block
        return mems_malloc(size);
    }
    
    // Return NULL if mmap fails
    return NULL;
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
struct mems_mainchain_node* block = mems_free_list;
    int block_count = 0;
    int hole_count = 0;
    int process_count = 0;
    int temp=0;
    int a=0;
    int b=0;
    int arr[200];
    int unused=0;
    printf("-----MeMs SYSTEM STATS -----\n");
    printf("MAIN[%lu,%lu] -->",block->start,block->end);
    while (block != NULL) {
        block_count++;
        
        struct mems_subchain_node* segment = block->subchain_head;
        int subchain_length = 0;
        if(temp!=0){
            printf("MAIN[%lu,%lu] -->",block->start-1,block->end);
        }
        temp+=1;
        while (segment != NULL) {
            if (segment->is_hole) {
                printf("H[%lu:%lu] <-> ", segment->start, segment->end-1);
                unused+= (segment->end)-segment->start;
                hole_count++;

            } else {
                printf("P[%lu:%lu] <-> ", segment->start, segment->end-1);
                process_count++;
            }
            segment = segment->next;
            subchain_length++;
        }
        
        segment = block->subchain_head;
        
        // while (segment != NULL) {
        //     if (segment->is_hole) {
        //         printf("H[%lu:%lu] <-> ", segment->start, segment->end);
            // } else {
        //         printf("P[%lu:%lu] <-> ", segment->start, segment->end);
        //     }
        //     segment = segment->next;
        // }
        
        a+=hole_count;
        b+=process_count;
        printf("NULL\n");
        // printf("Main Chain Length for Block %d: %d\n", block_count, subchain_length);
        arr[block_count]=hole_count+process_count;
        block = block->next;
        hole_count=0;
        process_count=0;

    }


    printf("Total Mapped Pages: %d\n", block_count);
    printf("space unused: %d\n", unused);
    printf("Main Chain length:%d\n", block_count);
    printf("Sub-chain length array: [");
    for(int i=1;i<=block_count;i++){
        printf("%d ,",arr[i]);
    }
    printf("]\n");

    // printf("Number of Holes: %d\n", hole_count);
    // printf("Number of Process Segments: %d\n", process_count);
    printf("-----------------------------\n");
    unused=0;
     
}


/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void*v_ptr){
    struct mems_mainchain_node* block = mems_free_list;

    while (block != NULL) {
        if (v_ptr >= (void *)block->start && v_ptr <= (void *)block->end) {
            return block->address +(unsigned long) v_ptr - sizeof(struct mems_mainchain_node);
        }
        block = block->next;
    }

    return NULL;
    
}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/
void mems_free(void * ptr){
    if (ptr == NULL) {
        return;  // Handle NULL pointer gracefully
    }

    // Find the block that contains the pointer
    struct mems_mainchain_node* block = mems_free_list;

    while (block != NULL) {
        if ((ptr >= (void*)block) && (ptr < (void*)((char*)block + block->total_size))) {
            // Calculate the offset within the block
            size_t offset = (size_t)((char*)ptr - (char*)block - sizeof(struct mems_mainchain_node));

            // Find the corresponding segment
            struct mems_subchain_node* segment = block->subchain_head;
            while (segment != NULL) {
                if (offset >= sizeof(struct mems_subchain_node) &&
                    offset < (size_t)((char*)segment - (char*)block)) {
                    // Mark the segment as a hole
                    segment->is_hole = 1;

                    // Merge with the previous segment if it's also a hole
                    if (segment->prev != NULL && segment->prev->is_hole) {
                        segment->prev->next = segment->next;
                        if (segment->next != NULL) {
                            segment->next->prev = segment->prev;
                        }
                        segment->prev->size += sizeof(struct mems_subchain_node) + segment->size;
                        segment = segment->prev;  // Move segment pointer to the merged segment
                    }

                    // Merge with the next segment if it's also a hole
                    if (segment->next != NULL && segment->next->is_hole) {
                        segment->size += sizeof(struct mems_subchain_node) + segment->next->size;
                        segment->next = segment->next->next;
                        if (segment->next != NULL) {
                            segment->next->prev = segment;
                        }
                    }

                    break;
                }
                segment = segment->next;
            }

            break;
        }
        block=block->next;
    }
    
}