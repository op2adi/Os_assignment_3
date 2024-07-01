# MeMS: Memory Management System [CSE231 OS Assignment 3]
[Documentation](https://docs.google.com/document/d/1Gs9kC3187lLrinvK1SueTc8dHCJ0QP43eRlrCRlXiCY/edit?usp=sharing)
---


This implementation provides a custom memory management system (MeMS) that mimics the functionalities of a typical memory management subsystem in an operating system. It handles memory allocation, deallocation, and maintains the state of memory using a linked list structure. The key features include the ability to allocate memory, free memory, and display memory statistics.

## Overview

The system uses a main chain of memory blocks, each of which contains a sub-chain of segments. Each segment can either be a hole (unused memory) or a process segment (used memory). The memory is managed in units of pages, defined by `PAGE_SIZE`.

### Key Structures

1. **`mems_subchain_node`**: Represents a segment within a main chain block. It can either be a hole or a process segment.
2. **`mems_mainchain_node`**: Represents a block of memory in the main chain. Each block contains a sub-chain of segments and metadata about the block.

### Key Global Variables

- **`mems_free_list`**: Points to the head of the free list.
- **`mems_heap_start`**: Marks the starting address of the heap managed by MeMS.

## Functions

### `mems_init()`

Initializes the MeMS system by setting up necessary parameters and allocating an initial block of memory using `mmap`.

### `mems_finish()`

Cleans up the MeMS system by unmapping all allocated memory using the `munmap` system call.

### `mems_malloc(size_t size)`

Allocates memory of the specified size by reusing a segment from the free list if available. If not, it allocates more memory using `mmap` and updates the free list accordingly.

### `mems_print_stats()`

Prints statistics of the MeMS system, including the number of utilized pages, unused memory, and details about each node in the main chain and each segment in the sub-chain.

### `mems_get(void* v_ptr)`

Returns the MeMS physical address mapped to a given MeMS virtual address (`v_ptr`).

### `mems_free(void* ptr)`

Frees up memory pointed to by a virtual address (`ptr`) and adds it back to the free list.

---


### How to run the exmaple.c
After implementing functions in mems.h follow the below steps to run example.c file
```
$ make
$ ./example
```

