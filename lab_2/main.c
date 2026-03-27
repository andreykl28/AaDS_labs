#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MIN_COUNT 16
#define DATA_LEN 128
#define LINE_LEN 512
#define EPS 1e-9

void trim_newline(char *s) {
    size_t len;

    if (s == NULL) {
        return;
    }

    len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

void clear_input_line(FILE *stream) {
    int ch;

    do {
        ch = fgetc(stream);
    } while (ch != '\n' && ch != EOF);
}

int allocate_table(double **keys, char (**data)[DATA_LEN], int n) {
    *keys = malloc(n * sizeof(double));
    if (*keys == NULL) {
        return 0;
    }

    *data = malloc(n * sizeof(**data));
    if (*data == NULL) {
        free(*keys);
        *keys = NULL;
        return 0;
    }

    return 1;
}

void free_table(double *keys, char (*data)[DATA_LEN]) {
    free(keys);
    free(data);
}

int compare_keys(double a, double b) {
    if (fabs(a - b) < EPS) {
        return 0;
    }

    if (a < b) {
        return -1;
    }

    return 1;
}

void swap_records(double keys[], char data[][DATA_LEN], int i, int j) {
    double temp_key;
    char temp_data[DATA_LEN];

    temp_key = keys[i];
    keys[i] = keys[j];
    keys[j] = temp_key;

    strcpy(temp_data, data[i]);
    strcpy(data[i], data[j]);
    strcpy(data[j], temp_data);
}

void copy_table(const double src_keys[], char src_data[][DATA_LEN],
                double dst_keys[], char dst_data[][DATA_LEN], int n) {
    int i;

    for (i = 0; i < n; i++) {
        dst_keys[i] = src_keys[i];
        strcpy(dst_data[i], src_data[i]);
    }
}

void reverse_table(double keys[], char data[][DATA_LEN], int n) {
    int left = 0;
    int right = n - 1;

    while (left < right) {
        swap_records(keys, data, left, right);
        left++;
        right--;
    }
}

void print_table(const double keys[], char data[][DATA_LEN], int n, const char *title) {
    int i;

    printf("\n%s\n", title);
    printf("------------------------------------------------------------\n");
    printf("%-5s %-15s %s\n", "No", "Key", "Data");
    printf("------------------------------------------------------------\n");

    for (i = 0; i < n; i++) {
        printf("%-5d %-15.6f %s\n", i + 1, keys[i], data[i]);
    }

    printf("------------------------------------------------------------\n");
}

void sift_down(double keys[], char data[][DATA_LEN], int start, int end) {
    int root = start;

    while (2 * root + 1 <= end) {
        int child = 2 * root + 1;
        int swap_index = root;

        if (compare_keys(keys[swap_index], keys[child]) < 0) {
            swap_index = child;
        }

        if (child + 1 <= end &&
            compare_keys(keys[swap_index], keys[child + 1]) < 0) {
            swap_index = child + 1;
        }

        if (swap_index == root) {
            return;
        }

        swap_records(keys, data, root, swap_index);
        root = swap_index;
    }
}

void heap_sort(double keys[], char data[][DATA_LEN], int n) {
    int start;
    int end;

    for (start = n / 2 - 1; start >= 0; start--) {
        sift_down(keys, data, start, n - 1);
    }

    for (end = n - 1; end > 0; end--) {
        swap_records(keys, data, 0, end);
        sift_down(keys, data, 0, end - 1);
    }
}

int binary_search(const double keys[], int n, double target) {
    int left = 0;
    int right = n - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = compare_keys(keys[mid], target);

        if (cmp == 0) {
            return mid;
        }

        if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1;
}

int read_table(FILE *f, double keys[], char data[][DATA_LEN], int n) {
    int i;
    char line[LINE_LEN];

    for (i = 0; i < n; i++) {
        if (fgets(line, sizeof(line), f) == NULL) {
            return 0;
        }

        trim_newline(line);

        if (sscanf(line, " %lf %[^\n]", &keys[i], data[i]) < 2) {
            return 0;
        }
    }

    return 1;
}

void demonstrate_case(double keys[], char data[][DATA_LEN], int n, const char *case_name) {
    print_table(keys, data, n, case_name);
    heap_sort(keys, data, n);
    print_table(keys, data, n, "После сортировки");
}

int main(void) {
    FILE *f;
    int n;
    int m;
    int i;
    int found_index;
    double target;

    double *base_keys = NULL;
    double *sorted_keys = NULL;
    double *reverse_keys = NULL;
    double *unordered_keys = NULL;
    double *search_keys = NULL;

    char (*base_data)[DATA_LEN] = NULL;
    char (*sorted_data)[DATA_LEN] = NULL;
    char (*reverse_data)[DATA_LEN] = NULL;
    char (*unordered_data)[DATA_LEN] = NULL;
    char (*search_data)[DATA_LEN] = NULL;

    f = fopen("input.txt", "r");
    if (f == NULL) {
        printf("Ошибка: не удалось открыть файл input.txt.\n");
        return 1;
    }

    if (fscanf(f, "%d", &n) != 1) {
        printf("Ошибка: не удалось прочитать количество элементов.\n");
        fclose(f);
        return 1;
    }

    if (n < MIN_COUNT) {
        printf("Ошибка: количество элементов должно быть не меньше %d.\n", MIN_COUNT);
        fclose(f);
        return 1;
    }

    clear_input_line(f);

    if (!allocate_table(&base_keys, &base_data, n) ||
        !allocate_table(&sorted_keys, &sorted_data, n) ||
        !allocate_table(&reverse_keys, &reverse_data, n) ||
        !allocate_table(&unordered_keys, &unordered_data, n) ||
        !allocate_table(&search_keys, &search_data, n)) {
        printf("Ошибка: не удалось выделить память.\n");
        fclose(f);
        free_table(base_keys, base_data);
        free_table(sorted_keys, sorted_data);
        free_table(reverse_keys, reverse_data);
        free_table(unordered_keys, unordered_data);
        free_table(search_keys, search_data);
        return 1;
    }

    if (!read_table(f, base_keys, base_data, n)) {
        printf("Ошибка: неверный формат входных данных таблицы.\n");
        fclose(f);
        free_table(base_keys, base_data);
        free_table(sorted_keys, sorted_data);
        free_table(reverse_keys, reverse_data);
        free_table(unordered_keys, unordered_data);
        free_table(search_keys, search_data);
        return 1;
    }

    copy_table(base_keys, base_data, unordered_keys, unordered_data, n);

    copy_table(base_keys, base_data, sorted_keys, sorted_data, n);
    heap_sort(sorted_keys, sorted_data, n);

    copy_table(sorted_keys, sorted_data, reverse_keys, reverse_data, n);
    reverse_table(reverse_keys, reverse_data, n);

    demonstrate_case(sorted_keys, sorted_data, n,
                     "Случай 1. Элементы таблицы с самого начала упорядочены");

    demonstrate_case(reverse_keys, reverse_data, n,
                     "Случай 2. Элементы таблицы расставлены в обратном порядке");

    demonstrate_case(unordered_keys, unordered_data, n,
                     "Случай 3. Элементы таблицы не упорядочены");

    copy_table(base_keys, base_data, search_keys, search_data, n);
    heap_sort(search_keys, search_data, n);

    print_table(search_keys, search_data, n,
                "Отсортированная таблица для двоичного поиска");

    if (fscanf(f, "%d", &m) != 1 || m <= 0) {
        printf("Ошибка: не удалось прочитать количество ключей для поиска.\n");
        fclose(f);
        free_table(base_keys, base_data);
        free_table(sorted_keys, sorted_data);
        free_table(reverse_keys, reverse_data);
        free_table(unordered_keys, unordered_data);
        free_table(search_keys, search_data);
        return 1;
    }

    for (i = 0; i < m; i++) {
        if (fscanf(f, "%lf", &target) != 1) {
            printf("Ошибка: не удалось прочитать ключ для поиска.\n");
            fclose(f);
            free_table(base_keys, base_data);
            free_table(sorted_keys, sorted_data);
            free_table(reverse_keys, reverse_data);
            free_table(unordered_keys, unordered_data);
            free_table(search_keys, search_data);
            return 1;
        }

        found_index = binary_search(search_keys, n, target);

        if (found_index == -1) {
            printf("Ключ %.6f: элемент не найден.\n", target);
        } else {
            printf("Ключ %.6f: найден элемент -> %.6f %s\n",
                   target,
                   search_keys[found_index],
                   search_data[found_index]);
        }
    }

    fclose(f);

    free_table(base_keys, base_data);
    free_table(sorted_keys, sorted_data);
    free_table(reverse_keys, reverse_data);
    free_table(unordered_keys, unordered_data);
    free_table(search_keys, search_data);

    return 0;
}