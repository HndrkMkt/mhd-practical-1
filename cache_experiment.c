#include <stdio.h>
#include <time.h>
#include <stdlib.h>

const int KB = 1024;
const int MB = 1024 * 1024;
const int ITERATIONS = 1000000000; // one billion iterations
const int INT_SIZE = sizeof(int);

int *init_looping_indices(int array_size, int stride);

int main() {
    int array_sizes[] = {4 * KB, 8 * KB, 16 * KB,
                         24 * KB, 32 * KB, 48 * KB, // narrow band around 32 KB
                         64 * KB, 128 * KB,
                         192 * KB, 256 * KB, 384 * KB, // narrow band around 256 KB
                         512 * KB, 1 * MB, 2 * MB,
                         3 * MB, 4 * MB, 6 * MB, // narrow band around 4 MB
                         8 * MB, 16 * MB, 32 * MB, 64 * MB, 128 * MB, 256 * MB, 512 * MB};
    int strides[] = {8, 16, 32,
                     48, 64, 96, // narrow band around 64 B
                     128, 256, 512, 1 * KB, 2 * KB, 4 * KB, 8 * KB, 16 * KB};
    int array_size, stride;
    double average_access_time;

    FILE *fp;
    fp = fopen("access_times.tsv", "w");

    int array_sizes_length = sizeof(array_sizes) / INT_SIZE;
    fprintf(fp, "Stride\tArray Size\tAccess Time\n");
    for (int stride_index = 0; stride_index < sizeof(strides) / INT_SIZE; stride_index++) {
        stride = strides[stride_index];
        for (int array_size_index = 0; array_size_index < array_sizes_length; array_size_index++) {
            array_size = array_sizes[array_size_index];
            /* init array and fill with offsets */
            struct timespec start, end;
            int *looping_indices = init_looping_indices(array_size, stride);
            int looping_index = 0;
            clock_gettime(CLOCK_REALTIME, &start);
#pragma unroll(100)
            for (int _ = 0; _ < ITERATIONS; _++) {
                //looping_index = _;
                looping_index = looping_indices[looping_index];
            }
            clock_gettime(CLOCK_REALTIME, &end);
            double access_time = (end.tv_sec - start.tv_sec) * 1000000000L; // convert seconds to nanoseconds
            access_time += (end.tv_nsec - start.tv_nsec);
            average_access_time = access_time / ITERATIONS;
            printf("Stride: %8d\tArray Size: %12d\tAverage Access Time: %8.5f ns\n", stride, array_size,
                   average_access_time);
            fprintf(fp, "%d\t%d\t%f\n", stride, array_size, average_access_time);
            free(looping_indices);
        }
    }
    fclose(fp);
}

int *init_looping_indices(int array_size, int stride) {
    int *looping_indices = malloc((size_t) array_size);
    int num_of_indices = array_size / INT_SIZE;
    for (int i = 0; i < num_of_indices; i++) {
        looping_indices[i] = 0;
    }
    int step_size = stride / INT_SIZE;
    int next_step = 0;
    int step;
    for (int i = 0; i < num_of_indices; i++) {
        step = next_step;
        next_step = (step + step_size) % num_of_indices;
        looping_indices[step] = next_step;
    }
    return looping_indices;
}