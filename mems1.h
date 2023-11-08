#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096  // This value might differ on some systems

// MeMS data structures
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

// Function to initialize the MeMS system
void mems_init() {
    // Initialize any global variables or data structures
    mems_free_list = NULL;
    
    // Allocate an initial block of memory from the OS using mmap
    size_t total_size = PAGE_SIZE;
    mems_heap_start = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    
    if (mems_heap_start != MAP_FAILED) {
        struct mems_mainchain_node* initial_block = (struct mems_mainchain_node*)mems_heap_start;
        initial_block->total_size = total_size;
        initial_block->start=0;
        initial_block->end=4095;
        initial_block->subchain_head = (struct mems_subchain_node*)((char*)initial_block + sizeof(struct mems_mainchain_node));
        initial_block->address=&mems_heap_start;
        initial_block->subchain_head->size = total_size - sizeof(struct mems_subchain_node);
        initial_block->num_pages=1;
        initial_block->address=mems_heap_start;
        initial_block->subchain_head->is_hole = 1;
        initial_block->subchain_head->next = NULL;
        initial_block->subchain_head->prev = NULL;
        initial_block->subchain_head->start=0;
        initial_block->subchain_head->end=4095;
        initial_block->next = NULL;
        initial_block->prev = NULL;
        mems_free_list = initial_block;
    }
}

// Function to allocate memory using MeMS
void* mems_malloc(size_t size) {
    if (size == 0) {
        return NULL;  // Handle zero-size allocation
    }

    struct mems_mainchain_node* block = mems_free_list;

    while (block != NULL) {
        struct mems_subchain_node* segment = block->subchain_head;

        while (segment != NULL) {
            if (segment->is_hole && segment->size >= size) {
                if (segment->size == size) {
                    segment->is_hole=0;
                    return (void *)1000+segment->start;
                }
                else{
                    struct mems_subchain_node* initial_block = (struct mems_subchain_node*)((char*)mems_heap_start +size);
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
                    return (void *)1000+segment->start;
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
    struct mems_mainchain_node* new_block = (struct mems_mainchain_node*)mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
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
        new_block->end=num_pages*PAGE_SIZE-1;
        new_block->address=&new_block;
        new_block->total_size = total_size;
        new_block->subchain_head = (struct mems_subchain_node*)((char*)new_block + sizeof(struct mems_mainchain_node));
        new_block->subchain_head->size = total_size - sizeof(struct mems_subchain_node);
        // printf("1\n");
        new_block->subchain_head->is_hole = 1;
        // printf("1\n");
        new_block->subchain_head->next = NULL;
        new_block->subchain_head->prev = NULL;
        new_block->subchain_head->start=b->end+1;
        new_block->subchain_head->end=b->end+num_pages*4096;
        new_block->num_pages=num_pages;
        // Add the new block to the free list
        new_block->next = NULL;
        new_block->prev = block;
        block->next=new_block;
        mems_heap_start=&new_block;
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
// Function to deallocate memory
void mems_free(void* ptr) {
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
                if (offset >= sizeof(struct mems_subchain_node) && offset < (size_t)((char*)segment - (char*)block)) {
                    // Mark the segment as a hole
                    segment->is_hole = 1;
                    break;
                }
                segment = segment->next;
            }
            
            break;
        }
        
        block = block->next;
    }
}

// Function to convert MeMS virtual address to MeMS physical address
void* mems_get(void* v_ptr) {
    struct mems_mainchain_node* block = mems_free_list;

    while (block != NULL) {
        if (v_ptr >= (void *)block->start && v_ptr <= (void *)block->end) {
            return block->address +(unsigned long) v_ptr - sizeof(struct mems_mainchain_node);
        }
        block = block->next;
    }

    return NULL;
}


// Function to print MeMS statistics
void mems_print_stats() {
    struct mems_mainchain_node* block = mems_free_list;
    int block_count = 0;
    int hole_count = 0;
    int process_count = 0;
    
    while (block != NULL) {
        block_count++;
        
        struct mems_subchain_node* segment = block->subchain_head;
        printf("MAIN[%lu,%lu] -->",block->start,block->end);
        while (segment != NULL) {
            if (segment->is_hole) {
                printf("H[%lu:%lu] <-> ", segment->start, segment->end);
                hole_count++;

            } else {
                printf("P[%lu:%lu] <-> ", segment->start, segment->end);
                process_count++;
            }
            segment = segment->next;
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
        
        printf("NULL\n");
        
        
        block = block->next;
    }

    printf("Total Mapped Pages: %d\n", block_count);
    printf("Unused Memory (Bytes): %lu\n", (unsigned long)(hole_count * sizeof(struct mems_subchain_node)));
    printf("Number of Holes: %d\n", hole_count);
    printf("Number of Process Segments: %d\n", process_count);
}

void mems_finish() {
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