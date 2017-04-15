
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
// page number is index into page table
// page table contains base address of every page in physical memory
// page table maps page numbers to frame numbers
// base address + offset = physical address
// 2^8 entries in page table
// 16 entries in TLB
// 2^8 byte frame size
// 256 frames
// memory size: 65536 bytes (256 frames x 256 byte size)
// 128 page frames (keep track of free page frames and use FIFO page replacement)
// FIFO replacement strategy to update TLB
// TLB- contains page numbers
// if TLB hit, frame number obtained from TLB, if TLB miss, consult page table (frame number obtained from page table or
//      page fault occurs)


#define FRAME_SIZE 256
#define PAGE_SIZE 256
unsigned MEMORY_SIZE = 65536;

int page_table[256];
int current = 0;
void add_address(unsigned, FILE *);
int main() {
    memset(page_table, -1, sizeof page_table);

    FILE *virtual_addrs = fopen("addresses.txt", "r");
    if (!virtual_addrs) {
        perror("fopen");
        return 1;
    }
    unsigned current_addr;
    FILE *backing_store = fopen("BACKING_STORE.bin", "r");
    while (fscanf(virtual_addrs, "%d", &current_addr) > 0) {
        add_address(current_addr, backing_store);
    }

    return 0;
}

void add_address(unsigned virtual_address, FILE *backing_store) {
    unsigned address_lower16 = virtual_address & 0xFFFF;
    unsigned page_number = (address_lower16 >> 8);
    unsigned offset = address_lower16 & 0xFF;
    int current_base;
    int phys_addr;
    if (page_table[page_number] >= 0) {
        current_base = page_table[page_number];
    }
    else {
        current_base = current;
        page_table[page_number] = current;
        current += FRAME_SIZE;
    }
    phys_addr = current_base + offset;

    int8_t memory[256];
    fseek(backing_store, page_number * PAGE_SIZE, SEEK_SET);
    fread(&memory, 1, PAGE_SIZE, backing_store);

    int value = memory[offset];
    printf("%d %d %d\n", virtual_address, phys_addr, value);
    //printf("%d %d %d %d %d\n", virtual_address, current_base + offset, current_base, offset, page_number);
}