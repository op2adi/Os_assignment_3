#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096  // This value might differ on some systems

// MeMS data structures
struct mems_subchain_node {
    size_t size;
    int is_hole;
    struct mems_subchain_node* next;
    struct mems_subchain_node* prev;
};

struct mems_mainchain_node {
    size_t total_size;
    struct mems_subchain_node* subchain_head;
    struct mems_mainchain_node* next;
    struct mems_mainchain_node* prev;
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
        initial_block->subchain_head = (struct mems_subchain_node*)((char*)initial_block + sizeof(struct mems_mainchain_node));
        initial_block->subchain_head->size = total_size - sizeof(struct mems_subchain_node);
        initial_block->subchain_head->is_hole = 1;
        initial_block->subchain_head->next = NULL;
        initial_block->subchain_head->prev = NULL;
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
                // Allocate memory from this segment
                segment->is_hole = 0; // Mark it as a process
                return (void*)((char*)block + sizeof(struct mems_mainchain_node) + (size_t)((char*)segment - (char*)mems_heap_start));
            }
            segment = segment->next;
        }
        
        block = block->next;
    }

    // If no suitable segment is found, request more memory from the OS
    size_t requested_size = size + sizeof(struct mems_subchain_node);
    size_t num_pages = (requested_size + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t total_size = num_pages * PAGE_SIZE;

    // Use mmap to allocate more memory
    struct mems_mainchain_node* new_block = (struct mems_mainchain_node*)mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (new_block != MAP_FAILED) {
        new_block->total_size = total_size;
        new_block->subchain_head = (struct mems_subchain_node*)((char*)new_block + sizeof(struct mems_mainchain_node));
        new_block->subchain_head->size = total_size - sizeof(struct mems_subchain_node);
        new_block->subchain_head->is_hole = 1;
        new_block->subchain_head->next = NULL;
        new_block->subchain_head->prev = NULL;
        
        // Add the new block to the free list
        new_block->next = mems_free_list;
        new_block->prev = NULL;
        if (mems_free_list != NULL) {
            mems_free_list->prev = new_block;
        }
        mems_free_list = new_block;

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
        if ((v_ptr >= (void*)block) && (v_ptr < (void*)((char*)block + block->total_size))) {
            // Calculate the offset within the block
            size_t offset = (size_t)((char*)v_ptr - (char*)block - sizeof(struct mems_mainchain_node));
            
            // Calculate the physical address
            return (void*)((char*)mems_heap_start + offset);
        }
        
        block = block->next;
    }
    
    return NULL;  // Address not found
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
        
        while (segment != NULL) {
            if (segment->is_hole) {
                hole_count++;
            } else {
                process_count++;
            }
            segment = segment->next;
        }
        
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
