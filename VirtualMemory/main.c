
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


#define PAGE_SIZE 256
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define BACKING_STORE "BACKING_STORE.bin"

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

int NUMBER_OF_FRAMES;
int MEMORY_SIZE;
int page_faults = 0;
int tlb_hits = 0;
int page_table_index = 0;
int8_t *memory;
bool is_used[PAGE_SIZE];
PageTable page_table;
TLB tlb;

void add_address(unsigned, FILE *);
void init_TLB(TLB *, int);
void init_page_table(PageTable *, int);

int main(int argc, char **argv) {
    memset(is_used, false, sizeof is_used);
    tlb.current_index = 0;
    init_TLB(&tlb, TLB_SIZE);
    init_page_table(&page_table, PAGE_TABLE_SIZE);

    if (argc < 2) {
        printf("address file required\n");
        exit(1);
    }

    char *address_file = argv[1];
    FILE *virtual_addrs = fopen(address_file, "r");
    if (!virtual_addrs) {
        perror("fopen");
        return 1;
    }
    unsigned current_addr;
    FILE *backing_store = fopen(BACKING_STORE, "r");
    if (!backing_store) {
        perror("fopen");
        return 1;
    }

    int c;
    printf("Enter 1 for a memory size of 2 ^ 16 bytes or enter 2 for a memory size of 2 ^ 15 bytes: ");
    while (1) {
        c = getchar();
        if (c == '1' || c == '2') {
            break;
        }
    }
    if (c == '1') {
        NUMBER_OF_FRAMES = 256;
        MEMORY_SIZE = 65536;
    }
    else {
        NUMBER_OF_FRAMES = 128;
        MEMORY_SIZE = 32768;
    }

    memory = malloc(MEMORY_SIZE * sizeof(int8_t));

    int num_addresses = 0;
    while (fscanf(virtual_addrs, "%d", &current_addr) > 0) {
        add_address(current_addr, backing_store);
        num_addresses++;
    }
    printf("Number of Translated Addresses = %d\n", num_addresses);
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %f\n", (float)page_faults / num_addresses);
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %f\n", (float)tlb_hits / num_addresses);
    return 0;
}

void init_TLB(TLB *tlb, int length) {
    for (int i = 0; i < length; i++) {
        TableEntry entry = {
                .key = -1,
                .value = -1,
                .valid = false
        };
        tlb->entries[i] = entry;
    }
}

void init_page_table(PageTable *page_table, int length) {
    for (int i = 0; i < length; i++) {
        TableEntry entry = {
                .key = 0,
                .value = -1,
                .valid = false
        };
        page_table->entries[i] = entry;
    }
}

void set(int page_number, int base) {
    page_table.entries[page_number].value = base;
    page_table.entries[page_number].valid = true;
}

void add(int page_number, int frame_number) {
    TableEntry entry = {
            .key = page_number,
            .value = frame_number,
            .valid = true
    };
    tlb.entries[tlb.current_index] = entry;
    tlb.current_index = (tlb.current_index + 1) % TLB_SIZE;
}

int search(TableEntry entries[], int page_number, int size) {
    for (int i = 0; i < size; i++) {
        if (entries[i].key == page_number && entries[i].valid) {
            return entries[i].value;
        }
    }
    return -1;
}

void invalidate(TableEntry entries[], int index, int size) {
    for (int i = 0; i < size; i++) {
        if (entries[i].valid && entries[i].value == index) {
            entries[i].valid = false;
            break;
        }
    }
}

void add_address(unsigned virtual_address, FILE *backing_store) {
    unsigned address_lower16 = virtual_address & 0xFFFF;
    unsigned page_number = (address_lower16 >> 8);
    unsigned offset = address_lower16 & 0xFF;
    int current_base;
    int phys_addr;
    int tlb_val;
    if ((tlb_val = search(tlb.entries, page_number, TLB_SIZE)) >= 0) {
        current_base = tlb_val;
        tlb_hits++;
    }
    else {
        TableEntry page;

        if (page_table.entries[page_number].valid) {
            page = page_table.entries[page_number];
            current_base = page.value;
        }
        else {
            current_base = page_table_index;
            if (is_used[page_table_index]) {
                invalidate(page_table.entries, page_table_index, PAGE_TABLE_SIZE);
                invalidate(tlb.entries, page_table_index, NUMBER_OF_FRAMES);
            }
            else {
                is_used[page_table_index] = true;
            }
            set(page_number, current_base);
            page_table_index = (page_table_index + 1) % NUMBER_OF_FRAMES;

            page_faults++;

            fseek(backing_store, page_number * PAGE_SIZE, SEEK_SET);
            fread(&memory[current_base * PAGE_SIZE], 1, PAGE_SIZE, backing_store);
        }
        add(page_number, current_base);
    }
    phys_addr = (current_base * PAGE_SIZE) + offset;

    int8_t value = memory[phys_addr];

    printf("Virtual address: %d Physical address: %d Value: %d\n", virtual_address, phys_addr, value);
}
