#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
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


//#define FRAME_SIZE 256
#define FRAME_SIZE 128
#define PAGE_SIZE 256
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
//#define MEMORY_SIZE 65536
#define MEMORY_SIZE 32768

int page_faults = 0;
int tlb_hits = 0;
int current = 0;
int8_t memory[MEMORY_SIZE];
bool is_used[PAGE_SIZE];

typedef struct {
    int key;
    int value;
    bool valid;

} TableEntry;

typedef struct {
    TableEntry entries[TLB_SIZE];
    int current_index;
} TLB;

typedef struct {
    TableEntry entries[PAGE_TABLE_SIZE];
} PageTable;

void init_TLB(TLB *tlb, int length) {
    for (int i = 0; i < length; i++) {
        tlb->entries[i].key = -1;
        tlb->entries[i].value = -1;
        tlb->entries[i].valid = false;
    }
}

void init_page_table(PageTable *page_table, int length) {
    for (int i = 0; i < length; i++) {
        page_table->entries[i].key = 0;
        page_table->entries[i].value = -1;
        page_table->entries[i].valid = false;
    }
}

TableEntry get(PageTable page_table, int page_number) {
    return page_table.entries[page_number];
}

void set(PageTable *page_table, int page_number, int base) {
    page_table->entries[page_number].value = base;
}

void add(TLB *tlb, int page_number, int frame_number) {
    TableEntry entry = {
            .key = page_number,
            .value = frame_number,
            .valid = false
    };
    tlb->entries[tlb->current_index] = entry;
    tlb->current_index = (tlb->current_index + 1) % TLB_SIZE;
}

int search(TableEntry entries[], int page_number, int size) {
    for (int i = 0; i < size; i++) {
        if (entries[i].key == page_number) {
            return entries[i].value;
        }
    }
    return -1;
}

void add_address(unsigned, FILE *, PageTable *, TLB *);
int main() {
    //memset(page_table, -1, sizeof page_table);
    memset(is_used, false, sizeof is_used);
    TLB tlb;
    PageTable page_table;
    tlb.current_index = 0;
    init_TLB(&tlb, TLB_SIZE);
    init_page_table(&page_table, PAGE_TABLE_SIZE);

    FILE *virtual_addrs = fopen("addresses.txt", "r");
    if (!virtual_addrs) {
        perror("fopen");
        return 1;
    }
    unsigned current_addr;
    FILE *backing_store = fopen("BACKING_STORE.bin", "r");
    while (fscanf(virtual_addrs, "%d", &current_addr) > 0) {
        add_address(current_addr, backing_store, &page_table, &tlb);
    }
    printf("%d %d\n", page_faults, tlb_hits);
    return 0;
}

void add_address(unsigned virtual_address, FILE *backing_store, PageTable *page_table, TLB *tlb) {
    unsigned address_lower16 = virtual_address & 0xFFFF;
    unsigned page_number = (address_lower16 >> 8);
    unsigned offset = address_lower16 & 0xFF;
    int current_base;
    int phys_addr;
    int tlb_val;
    if ((tlb_val = search(tlb->entries, page_number, TLB_SIZE)) >= 0) {
        current_base = tlb_val;
        tlb_hits++;
    }
    else {
        TableEntry page;
        if ((page = get(*page_table, page_number)).value >= 0) {
            current_base = page.value;
        }
        else {
            current_base = current;
//            if (current_base > 128) {
//                printf("%d\n", current_base);
//            }
//
//            if (is_used[current_base]) {
//                printf("test\n");
//            }
            set(page_table, page_number, current);
            //current += FRAME_SIZE;
            //current++;
            current = (current + 1) % FRAME_SIZE;
            page_faults++;
            is_used[current_base] = true;
            fseek(backing_store, page_number * PAGE_SIZE, SEEK_SET);
            fread(&memory[current_base * FRAME_SIZE], 1, PAGE_SIZE, backing_store);
        }
        add(tlb, page_number, current_base);
    }
    phys_addr = (current_base * FRAME_SIZE) + offset;
//    if (phys_addr >= MEMORY_SIZE) {
//        printf("test\n");
//    }
    int8_t value = memory[phys_addr];
//    for (int i = 0; i < TLB_SIZE; i++) {
//        printf("%d %d | ", tlb->page_number[i], tlb->frame_number[i]);
//    }
//    printf("\n");
    printf("%d %d %d\n", virtual_address, phys_addr, value);
    //printf("%d %d %d %d %d\n", virtual_address, current_base + offset, current_base, offset, page_number);
}
