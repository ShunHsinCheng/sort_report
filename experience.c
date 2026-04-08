#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 100000
#define TRIALS_N2 10
#define TRIALS_NLOGN 10


typedef void (*SortFn)(int *arr, int n);

static int is_sorted(const int *arr, int n)
{
    for (int i = 1; i < n; i++) {
        if (arr[i - 1] > arr[i]) {
            return 0;
        }
    }
    return 1;
}

static void fill_worst_case_array(int *arr, int n)
{
    for (int i = 0; i < n; i++) {
        arr[i] = n - i;
    }
}

static double benchmark_sort(SortFn sort_fn, const int *source, int n, int trials)
{
    double total_seconds = 0.0;

    for (int trial = 0; trial < trials; trial++) {
        int *buffer = (int *)malloc((size_t)n * sizeof(int));
        if (buffer == NULL) {
            fprintf(stderr, "記憶體配置失敗\n");
            exit(EXIT_FAILURE);
        }

        memcpy(buffer, source, (size_t)n * sizeof(int));

        clock_t start = clock();
        sort_fn(buffer, n);
        clock_t end = clock();

        total_seconds += (double)(end - start) / CLOCKS_PER_SEC;

        if (!is_sorted(buffer, n)) {
            fprintf(stderr, "排序驗證失敗，n = %d\n", n);
            free(buffer);
            exit(EXIT_FAILURE);
        }

        free(buffer);
    }

    return total_seconds / trials;
}

static void run_experiment(const char *title, SortFn sort_fn, const int *sizes, int size_count, int trials)
{
    printf("\n==================== %s ====================\n", title);
    printf("%-10s %-20s %-10s\n", "n", "平均時間(秒)", "驗證");

    for (int i = 0; i < size_count; i++) {
        int n = sizes[i];
        int *source = (int *)malloc((size_t)n * sizeof(int));
        if (source == NULL) {
            fprintf(stderr, "記憶體配置失敗\n");
            exit(EXIT_FAILURE);
        }

        fill_worst_case_array(source, n);

        double average_seconds = benchmark_sort(sort_fn, source, n, trials);
        printf("%-10d %-20.9f %-10s\n", n, average_seconds, "通過");

        free(source);
    }
}


/* ============================================================= */
/* 氣泡排序 Bubble Sort */
/* ============================================================= */


static void bubble_sort(int *arr, int n)
{
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

static void run_bubble_experiment(void)
{
    const int sizes[] = {N};
    run_experiment("氣泡排序 Bubble Sort", bubble_sort, sizes, (int)(sizeof(sizes) / sizeof(sizes[0])), TRIALS_N2);
}

/* ============================================================= */
/* 選擇排序 Selection Sort */
/* ============================================================= */

static void selection_sort(int *arr, int n)
{
    for (int i = 0; i < n - 1; i++) {
        int min_index = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_index]) {
                min_index = j;
            }
        }

        if (min_index != i) {
            int temp = arr[i];
            arr[i] = arr[min_index];
            arr[min_index] = temp;
        }
    }
}

static void run_selection_experiment(void)
{
    const int sizes[] = {N};
    run_experiment("選擇排序 Selection Sort", selection_sort, sizes, (int)(sizeof(sizes) / sizeof(sizes[0])), TRIALS_N2);
}

/* ============================================================= */
/* 插入排序 Insertion Sort */
/* ============================================================= */


static void insertion_sort(int *arr, int n)
{
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;

        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }

        arr[j + 1] = key;
    }
}

static void run_insertion_experiment(void)
{
    const int sizes[] = {N};
    run_experiment("插入排序 Insertion Sort", insertion_sort, sizes, (int)(sizeof(sizes) / sizeof(sizes[0])), TRIALS_N2);
}

/* ============================================================= */
/* 合併排序 Merge Sort */
/* ============================================================= */

static void merge(int *arr, int *temp, int left, int mid, int right)
{
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    while (j <= right) {
        temp[k++] = arr[j++];
    }

    for (int index = left; index <= right; index++) {
        arr[index] = temp[index];
    }
}

static void merge_sort_recursive(int *arr, int *temp, int left, int right)
{
    if (left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;
    merge_sort_recursive(arr, temp, left, mid);
    merge_sort_recursive(arr, temp, mid + 1, right);
    merge(arr, temp, left, mid, right);
}

static void merge_sort(int *arr, int n)
{
    int *temp = (int *)malloc((size_t)n * sizeof(int));
    if (temp == NULL) {
        fprintf(stderr, "記憶體配置失敗\n");
        exit(EXIT_FAILURE);
    }

    merge_sort_recursive(arr, temp, 0, n - 1);
    free(temp);
}

static void run_merge_experiment(void)
{
    const int sizes[] = {N};
    run_experiment("合併排序 Merge Sort", merge_sort, sizes, (int)(sizeof(sizes) / sizeof(sizes[0])), TRIALS_NLOGN);
}

/* ============================================================= */
/* 快速排序 Quick Sort */
/* ============================================================= */

static void swap_int(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

static int partition(int *arr, int low, int high)
{
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap_int(&arr[i], &arr[j]);
        }
    }

    swap_int(&arr[i + 1], &arr[high]);
    return i + 1;
}

static void quick_sort_recursive(int *arr, int low, int high)
{
    while (low < high) {
        int pivot_index = partition(arr, low, high);

        if (pivot_index - low < high - pivot_index) {
            quick_sort_recursive(arr, low, pivot_index - 1);
            low = pivot_index + 1;
        } else {
            quick_sort_recursive(arr, pivot_index + 1, high);
            high = pivot_index - 1;
        }
    }
}

static void quick_sort(int *arr, int n)
{
    quick_sort_recursive(arr, 0, n - 1);
}

static void run_quick_experiment(void)
{
    const int sizes[] = {N};
    run_experiment("快速排序 Quick Sort", quick_sort, sizes, (int)(sizeof(sizes) / sizeof(sizes[0])), TRIALS_NLOGN);
}

/* ============================================================= */
/* 堆積排序 Heap Sort */
/* ============================================================= */

static void heapify(int *arr, int n, int root)
{
    int largest = root;
    int left = 2 * root + 1;
    int right = 2 * root + 2;

    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }

    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }

    if (largest != root) {
        swap_int(&arr[root], &arr[largest]);
        heapify(arr, n, largest);
    }
}

static void heap_sort(int *arr, int n)
{
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    for (int i = n - 1; i > 0; i--) {
        swap_int(&arr[0], &arr[i]);
        heapify(arr, i, 0);
    }
}

static void run_heap_experiment(void)
{
    const int sizes[] = {N};
    run_experiment("堆積排序 Heap Sort", heap_sort, sizes, (int)(sizeof(sizes) / sizeof(sizes[0])), TRIALS_NLOGN);
}

int main(void)
{
    srand(20260408);

    puts("排序法模擬實驗開始");

    run_bubble_experiment();
    run_selection_experiment();
    run_insertion_experiment();
    run_merge_experiment();
    run_quick_experiment();
    run_heap_experiment();

    puts("\n排序法模擬實驗完成");
    return 0;
}