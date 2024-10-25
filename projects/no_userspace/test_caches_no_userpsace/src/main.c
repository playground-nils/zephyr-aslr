#include <stdio.h>
#include <zephyr/cache.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/random/random.h>

#define MAX_RUNS 50
#define RUNS 10
#define OFFSET 10
#define PRIORITY 5
#define USER_STACKSIZE 4096
#define HEAP_SIZE CONFIG_MMU_PAGE_SIZE * 200

#ifndef CONFIG_TEST_RANDOM_GENERATOR
#error This samples requires CONFIG_TEST_RANDOM_GENERATOR.
#endif

int main(void)
{
    printf("===START===\n");
#ifdef CONFIG_BENCHMARKING
    printf("y\n");
#else
    printf("n\n");
#endif
#ifdef CONFIG_SCTLR_BENCHMARKING
    printf("y\n");
#else
    printf("n\n");
#endif
    uint64_t start_time, end_time;
    uint64_t total_cycles = 0;
    uint64_t total_ns = 0;
    uint64_t size;
    uint64_t* array;

    uint64_t res[RUNS * MAX_RUNS];
    size_t index = 0;

    for (uint64_t i = OFFSET; i < MAX_RUNS; i++) {
        size = UINT64_C(1) << i;

        array = (uint64_t*)k_malloc(sizeof(uint64_t) * size);
        if (array == NULL) {
            uint64_t temp = OFFSET;
            for (int j = 0; j < (i - OFFSET) * RUNS; j++) {
                if (j > OFFSET && j % RUNS == 0) {
                    temp++;
                }
                printf("%" PRIu64 ",%" PRIu64 "\n", 
                        (UINT64_C(1) << temp) * sizeof(uint64_t),
                        res[j]);
            }
            printf("===END===\n");
            return;
        }

        for (unsigned int j = 0; j < RUNS; j++) {
            // https://www.cs.cornell.edu/gries/TechReports/86-786.pdf
            {
                double r;
                uint64_t theta;
                uint64_t temp;
                uint64_t i;

                for (i = 0 ; i < size; i++) {
                    array[i] = i;
                }

                i = size - 1;

                while (i != 0) {
                    // the line below makes the program crash
                    /*r = (double)sys_rand64_get() */
                    /*    / (double)(RAND_MAX / 1);*/
                    theta = (uint64_t)floor(i * r);
                    temp = array[i];
                    array[i] = array[theta];
                    array[theta] = temp;
                    i--;
                }
            }
            start_time = k_cycle_get_64();
            {
                uint64_t idx = 0;
                for (uint64_t i = 0; i < size; i++) {
                    idx = array[idx];
                }
            }
            end_time = k_cycle_get_64();
            total_cycles = end_time - start_time;

            res[index] = total_cycles;
            index++;
        }
        k_free(array);
    }

    return 0;
}
