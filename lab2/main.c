#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

typedef struct {
    int *array;
    int low;
    int cnt;
    int dir;
} SortTask;

int MAX_THREADS;

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void bitonicMerge(int arr[], int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = low; i < low + k; i++) {
            if (dir == (arr[i] > arr[i + k])) {
                swap(&arr[i], &arr[i + k]);
            }
        }
        bitonicMerge(arr, low, k, dir);
        bitonicMerge(arr, low + k, k, dir);
    }
}

void bitonicSortSeq(int arr[], int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonicSortSeq(arr, low, k, !dir);
        bitonicSortSeq(arr, low + k, k, dir);
        bitonicMerge(arr, low, cnt, dir);
    }
}

void* sortThread(void *arg) {
    SortTask *task = (SortTask *)arg;
    bitonicSortSeq(task->array, task->low, task->cnt, task->dir);
    free(task);
    return NULL;
}

void parallelBitonicSort(int *arr, int n, int dir) {
    int N = 1;
    while (N < n) N <<= 1;
    printf("Исходный размер: %d, Приведен к: %d\n", n, N);
    int *temp = malloc(N * sizeof(int));
    if (!temp) {
        printf("err\n");
        return;
    }
    memcpy(temp, arr, n * sizeof(int));
    for (int i = n; i < N; i++) {
        temp[i] = INT_MAX;
    }
    if (N <= 1024 || MAX_THREADS == 1) {
        bitonicSortSeq(temp, 0, N, dir);
    } else {
        int block_size = N / MAX_THREADS;
        while (block_size & (block_size - 1)) {
            block_size++;
        }
        if (block_size > N / 2) block_size = N / 2;
        pthread_t *threads = malloc(MAX_THREADS * sizeof(pthread_t));
        if (!threads) {
            printf("err\n");
            free(temp);
            return;
        }
        for (int i = 0; i < MAX_THREADS; i++) {
            SortTask *task = malloc(sizeof(SortTask));
            if (!task) {
                printf("err\n");
                continue;
            }
            task->array = temp;
            task->low = i * block_size;
            task->cnt = (i == MAX_THREADS - 1) ? (N - i * block_size) : block_size;
            task->dir = (i % 2 == 0) ? dir : !dir; 
            
            if (pthread_create(&threads[i], NULL, sortThread, task) != 0) {
                printf("err %d\n", i);
                free(task);
            }
        }
        
        for (int i = 0; i < MAX_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
        int merge_size = block_size;
        while (merge_size < N) {
            for (int start = 0; start < N; start += 2 * merge_size) {
                int end = start + 2 * merge_size;
                if (end > N) end = N;
                int merge_dir = ((start / (2 * merge_size)) % 2 == 0) ? dir : !dir;
                for (int i = start; i < start + merge_size && i + merge_size < end; i++) {
                    if (merge_dir == (temp[i] > temp[i + merge_size])) {
                        swap(&temp[i], &temp[i + merge_size]);
                    }
                }
                if (merge_size > 1) {
                    for (int i = start; i < end; i += merge_size) {
                        int block_end = i + merge_size;
                        if (block_end > end) block_end = end;
                        bitonicMerge(temp, i, block_end - i, merge_dir);
                    }
                }
            }
            merge_size *= 2;
        }
        free(threads);
    }
    memcpy(arr, temp, n * sizeof(int));
    free(temp);
}

int isSorted(int *arr, int n) {
    for (int i = 1; i < n; i++) {
        if (arr[i] < arr[i - 1]) {
            printf("err %d-%d: %d > %d\n", 
                   i-1, i, arr[i-1], arr[i]);
            return 0;
        }
    }
    return 1;
}

void testWithSmallArray() {
    printf("\n=== ТЕСТ НА МАЛЕНЬКОМ МАССИВЕ ===\n");
    int test_arr[] = {3, 7, 4, 8, 6, 2, 1, 5};
    int test_size = 8;
    printf("Исходный массив: ");
    for (int i = 0; i < test_size; i++) printf("%d ", test_arr[i]);
    printf("\n");
    parallelBitonicSort(test_arr, test_size, 1);
    printf("Отсортированный массив: ");
    for (int i = 0; i < test_size; i++) printf("%d ", test_arr[i]);
    printf("\n");
    if (isSorted(test_arr, test_size)) {
        printf("Тест пройден\n");
    } else {
        printf("Тест не пройден\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Использование: %s <количество_потоков> <размер_массива>\n", argv[0]);
        printf("Пример: %s 4 100000\n", argv[0]);
        testWithSmallArray();
        return 1;
    }
    MAX_THREADS = atoi(argv[1]);
    int n = atoi(argv[2]);
    if (MAX_THREADS < 1 || n < 1) {
        printf("err\n");
        return 1;
    }
    int *arr = malloc(n * sizeof(int));
    if (!arr) {
        printf("err\n");
        return 1;
    }
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000000;
    }
    printf("Сортировка %d, потоков: %d\n", n, MAX_THREADS);
    printf("PID: %d\n", getpid());
    printf("Первые 10: ");
    for (int i = 0; i < 10 && i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    clock_t start = clock();
    parallelBitonicSort(arr, n, 1);
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Время: %.4f \n", time_taken);
    printf("Первые 10: ");
    for (int i = 0; i < 10 && i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    if (isSorted(arr, n)) {
        printf("Массив отсортирован\n");
    } else {
        printf("Ошибка\n");
    }
    printf("\nИССЛЕДОВАНИЕ ПРОИЗВОДИТЕЛЬНОСТИ\n");
    
    int sizes[] = {1000, 10000, 100000, 1000000};
    int num_sizes = 4;
    
    for (int threads = 1; threads <= 8; threads *= 2) {
        printf("\n--- %d поток(ов) ---\n", threads);
        
        for (int s = 0; s < num_sizes; s++) {
            int n = sizes[s];
            int *arr = malloc(n * sizeof(int));
            srand(42); 
            for (int i = 0; i < n; i++) {
                arr[i] = rand() % 1000000;
            }
            
            MAX_THREADS = threads;
            clock_t start = clock();
            parallelBitonicSort(arr, n, 1);
            clock_t end = clock();
            
            double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
            
            int sorted = 1;
            for (int i = 1; i < n; i++) {
                if (arr[i] < arr[i-1]) {
                    sorted = 0;
                    break;
                }
            }
            
            printf("Размер: %d, Время: %.4f сек, Корректность: %s\n", 
                   n, time_taken, sorted ? "Да" : "Нет");
            
            free(arr);
        }
    }

    free(arr);
    return 0;

}
