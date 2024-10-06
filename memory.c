
#include "oslabs.h"
#include <stdio.h>



// Null PCB
const struct MEMORY_BLOCK NULLBLOCK = {0, 0, 0, 0};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions

void print_mb(struct MEMORY_BLOCK mb) {
    printf("Start Address: %d\n End Address: %d\n Segment Size: %d\n PID: %d\n",
        mb.start_address, mb.end_address, mb.segment_size, mb.process_id);
};

// Adjusts the memory map for adding in an entry
struct MEMORY_BLOCK adjust_memory(int fit_index, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int request_size) {
    int empty_index = fit_index+1;
    memory_map[fit_index].segment_size = request_size;
    for(int i = *map_cnt; i > fit_index; i--) {
        memory_map[i] = memory_map[i-1];
    }
    *map_cnt += 1;
    memory_map[fit_index].end_address = request_size + memory_map[fit_index].start_address - 1;
    memory_map[empty_index].process_id = 0;
    memory_map[empty_index].start_address = memory_map[fit_index].end_address + 1;
    memory_map[empty_index].segment_size = memory_map[empty_index].end_address - memory_map[empty_index].start_address + 1;
    return memory_map[fit_index];
};

// Add process to map
struct MEMORY_BLOCK add_process(int fit_index, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int request_size, int process_id, int excess_space) {
    memory_map[fit_index].process_id = process_id;
    // Checks if the fit is perfect if it is just replace the PID of the empty block with the new memory block
    if (excess_space == 0){
        return memory_map[fit_index];
    }
    // If it is not a perfect fit place an empty block after the new block
    else {
        return adjust_memory(fit_index, memory_map, map_cnt, request_size);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Best Fit Allocate

struct MEMORY_BLOCK best_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    int best_fit_index = 100000000;
    int best_fit_excess_space = 100000000;
    // Searches for best fit/the least excess space
    for(int i = 0; i < *map_cnt; i++) {
        int excess_space = memory_map[i].segment_size-request_size;
        if (memory_map[i].process_id == 0 && excess_space >= 0 && excess_space < best_fit_excess_space) {
            best_fit_index = i;
            best_fit_excess_space = excess_space;
        }
    }
    return add_process(best_fit_index, memory_map, map_cnt, request_size, process_id, best_fit_excess_space);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// First Fit Allocate

struct MEMORY_BLOCK first_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    int fit_index = -1;
    int excess_space = -1;
    // Searches for first fit
    for(int i = 0; i < *map_cnt; i++) {
        excess_space = memory_map[i].segment_size-request_size;
        if (memory_map[i].process_id == 0 && excess_space >= 0) {
            fit_index = i;
            break;
        }
    }
    return add_process(fit_index, memory_map, map_cnt, request_size, process_id, excess_space);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Worst Fit Allocate

struct MEMORY_BLOCK worst_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    int worst_fit_index = -1;
    int most_excess_space = -1;
    // Searches for worst fit/the most excess space
    for(int i = 0; i < *map_cnt; i++) {
        int excess_space = memory_map[i].segment_size-request_size;
        if (memory_map[i].process_id == 0 && excess_space >= 0 && excess_space > most_excess_space) {
            worst_fit_index = i;
            most_excess_space = excess_space;
        }
    }
    return add_process(worst_fit_index, memory_map, map_cnt, request_size, process_id, most_excess_space);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Worst Fit Allocate

struct MEMORY_BLOCK next_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id, int last_address) {
    int fit_index = -1;
    int excess_space = -1;
    // Searches for next fit
    for(int i = last_address; i < *map_cnt; i++) {
        excess_space = memory_map[i].segment_size-request_size;
        if (memory_map[i].process_id == 0 && excess_space >= 0) {
            fit_index = i;
            break;
        }
    }
    return add_process(fit_index, memory_map, map_cnt, request_size, process_id, excess_space);
}

void remove_block(struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int freed_index) {
    memory_map[freed_index].process_id = 0;
    memory_map[freed_index].end_address = memory_map[freed_index+1].end_address;
    memory_map[freed_index].segment_size = memory_map[freed_index].end_address - memory_map[freed_index].start_address + 1;
    for (int i = freed_index+1; i < *map_cnt; i++) {
        memory_map[i] = memory_map[i+1];
    }
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Release Memory

void release_memory(struct MEMORY_BLOCK freed_block, struct MEMORY_BLOCK memory_map[MAPMAX],int *map_cnt) {
    int freed_index = -1;
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == freed_block.process_id) {
            freed_index = i;
            break;
        }
    }
    if (freed_index > 0 && memory_map[freed_index-1].process_id == 0) {
        *map_cnt -= 1;
        freed_index -= 1;
        remove_block(memory_map, map_cnt, freed_index);
    }
    if (freed_index < *map_cnt-1 && memory_map[freed_index+1].process_id == 0) {
        *map_cnt -= 1;
        remove_block(memory_map, map_cnt, freed_index);
    }
}





// int main() {
//     struct MEMORY_BLOCK a = {0, 1023, 1024, 0};
//     struct MEMORY_BLOCK memory_map[MAPMAX];
//     memory_map[0] = a;
//     int cnt = 1;
//     int *map_cnt = &cnt;
//     int request_size = 10;
//     int process_id = 32;
//     struct MEMORY_BLOCK cur = best_fit_allocate(request_size, memory_map, map_cnt, process_id);
//     printf("\nmemory_map: \n");
//     for (int i = 0; i < *map_cnt; i++) {
//         print_mb(memory_map[i]);
//     }
//     printf("\nmap_cnt: %d\n", *map_cnt);
//     print_mb(cur);
// }