#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_KEY_LEN 6
#define MAX_LINE_LEN 256

typedef enum {
    RED,
    BLACK
} Color;

typedef struct RBNode {
    char key[MAX_KEY_LEN + 1];
    double value;
    Color color;
    struct RBNode *left;
    struct RBNode *right;
    struct RBNode *parent;
} RBNode;

typedef struct {
    RBNode *root;
    RBNode *nil;
} RBTree;

void trim_newline(char *s) {
    if (s == NULL) return;

    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

int is_valid_key(const char *key) {
    size_t len, i;

    if (key == NULL || key[0] == '\0') {
        return 0;
    }

    len = strlen(key);
    if (len == 0 || len > MAX_KEY_LEN) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (!((key[i] >= 'a' && key[i] <= 'z') ||
              (key[i] >= 'A' && key[i] <= 'Z'))) {
            return 0;
        }
    }

    return 1;
}

const char *color_name(Color color) {
    return (color == RED) ? "R" : "B";
}

int init_tree(RBTree *tree) {
    tree->nil = (RBNode *)malloc(sizeof(RBNode));
    if (tree->nil == NULL) {
        return 0;
    }

    strcpy(tree->nil->key, "NIL");
    tree->nil->value = 0.0;
    tree->nil->color = BLACK;
    tree->nil->left = tree->nil;
    tree->nil->right = tree->nil;
    tree->nil->parent = tree->nil;

    tree->root = tree->nil;
    return 1;
}

RBNode *create_node(RBTree *tree, const char *key, double value) {
    RBNode *node = (RBNode *)malloc(sizeof(RBNode));
    if (node == NULL) {
        return NULL;
    }

    strcpy(node->key, key);
    node->value = value;
    node->color = RED;
    node->left = tree->nil;
    node->right = tree->nil;
    node->parent = tree->nil;

    return node;
}

void free_subtree(RBTree *tree, RBNode *node) {
    if (node == tree->nil) {
        return;
    }

    free_subtree(tree, node->left);
    free_subtree(tree, node->right);
    free(node);
}

void destroy_tree(RBTree *tree) {
    free_subtree(tree, tree->root);
    free(tree->nil);
}

void left_rotate(RBTree *tree, RBNode *x) {
    RBNode *y = x->right;

    x->right = y->left;
    if (y->left != tree->nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == tree->nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

void right_rotate(RBTree *tree, RBNode *y) {
    RBNode *x = y->left;

    y->left = x->right;
    if (x->right != tree->nil) {
        x->right->parent = y;
    }

    x->parent = y->parent;

    if (y->parent == tree->nil) {
        tree->root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }

    x->right = y;
    y->parent = x;
}

RBNode *search_node(RBTree *tree, const char *key) {
    RBNode *cur = tree->root;

    while (cur != tree->nil) {
        int cmp = strcmp(key, cur->key);

        if (cmp == 0) {
            return cur;
        } else if (cmp < 0) {
            cur = cur->left;
        } else {
            cur = cur->right;
        }
    }

    return tree->nil;
}

RBNode *tree_minimum(RBTree *tree, RBNode *node) {
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

void insert_fixup(RBTree *tree, RBNode *z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode *y = z->parent->parent->right;

            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(tree, z);
                }

                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                right_rotate(tree, z->parent->parent);
            }
        } else {
            RBNode *y = z->parent->parent->left;

            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(tree, z);
                }

                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                left_rotate(tree, z->parent->parent);
            }
        }
    }

    tree->root->color = BLACK;
}

int insert_value(RBTree *tree, const char *key, double value) {
    RBNode *parent = tree->nil;
    RBNode *cur = tree->root;
    RBNode *node;

    while (cur != tree->nil) {
        int cmp = strcmp(key, cur->key);
        parent = cur;

        if (cmp == 0) {
            return 0;
        } else if (cmp < 0) {
            cur = cur->left;
        } else {
            cur = cur->right;
        }
    }

    node = create_node(tree, key, value);
    if (node == NULL) {
        return -1;
    }

    node->parent = parent;

    if (parent == tree->nil) {
        tree->root = node;
    } else if (strcmp(node->key, parent->key) < 0) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    insert_fixup(tree, node);
    return 1;
}

void transplant(RBTree *tree, RBNode *u, RBNode *v) {
    if (u->parent == tree->nil) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }

    v->parent = u->parent;
}

void delete_fixup(RBTree *tree, RBNode *x) {
    while (x != tree->root && x->color == BLACK) {
        if (x == x->parent->left) {
            RBNode *w = x->parent->right;

            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                left_rotate(tree, x->parent);
                w = x->parent->right;
            }

            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    right_rotate(tree, w);
                    w = x->parent->right;
                }

                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                left_rotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            RBNode *w = x->parent->left;

            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                right_rotate(tree, x->parent);
                w = x->parent->left;
            }

            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    left_rotate(tree, w);
                    w = x->parent->left;
                }

                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }

    x->color = BLACK;
}

int delete_value(RBTree *tree, const char *key) {
    RBNode *z = search_node(tree, key);
    RBNode *y;
    RBNode *x;
    Color y_original_color;

    if (z == tree->nil) {
        return 0;
    }

    y = z;
    y_original_color = y->color;

    if (z->left == tree->nil) {
        x = z->right;
        transplant(tree, z, z->right);
    } else if (z->right == tree->nil) {
        x = z->left;
        transplant(tree, z, z->left);
    } else {
        y = tree_minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    free(z);

    if (y_original_color == BLACK) {
        delete_fixup(tree, x);
    }

    return 1;
}

void print_tree_recursive(RBTree *tree, RBNode *node, int level, FILE *out, const char *label) {
    int i;

    for (i = 0; i < level; i++) {
        fprintf(out, "    ");
    }

    if (node == tree->nil) {
        fprintf(out, "%s: NIL(B)\n", label);
        return;
    }

    fprintf(out, "%s: %s(%.10g,%s)\n", label, node->key, node->value, color_name(node->color));
    print_tree_recursive(tree, node->left, level + 1, out, "L");
    print_tree_recursive(tree, node->right, level + 1, out, "R");
}

void print_tree(RBTree *tree, FILE *out) {
    fprintf(out, "Дерево:\n");
    print_tree_recursive(tree, tree->root, 0, out, "ROOT");
}

int parse_insert_command(const char *line, char *key, double *value) {
    char extra[32];
    int count = sscanf(line, "1 %63s %lf %31s", key, value, extra);
    return count == 2;
}

int parse_delete_command(const char *line, char *key) {
    char extra[32];
    int count = sscanf(line, "2 %63s %31s", key, extra);
    return count == 1;
}

int parse_search_command(const char *line, char *key) {
    char extra[32];
    int count = sscanf(line, "4 %63s %31s", key, extra);
    return count == 1;
}

int parse_print_command(const char *line) {
    int op;
    char extra[32];
    int count = sscanf(line, "%d %31s", &op, extra);
    return count == 1 && op == 3;
}

int parse_exit_command(const char *line) {
    int op;
    char extra[32];
    int count = sscanf(line, "%d %31s", &op, extra);
    return count == 1 && op == 0;
}

void print_invalid_command(FILE *out) {
    fprintf(out, "Ошибка: некорректная команда.\n");
}

int process_command(RBTree *tree, const char *line, FILE *out) {
    int op;
    char key[64];
    double value;

    if (sscanf(line, "%d", &op) != 1) {
        print_invalid_command(out);
        fprintf(out, "\n");
        return 1;
    }

    fprintf(out, "Команда: %s\n", line);

    switch (op) {
        case 0:
            if (!parse_exit_command(line)) {
                print_invalid_command(out);
                fprintf(out, "\n");
                return 1;
            }
            fprintf(out, "Результат: завершение программы.\n\n");
            return 0;

        case 1:
            if (!parse_insert_command(line, key, &value)) {
                print_invalid_command(out);
            } else if (!is_valid_key(key)) {
                fprintf(out, "Ошибка: ключ должен содержать только латинские буквы, длина 1..6.\n");
            } else {
                int res = insert_value(tree, key, value);
                if (res == 1) {
                    fprintf(out, "Результат: узел добавлен.\n");
                } else if (res == 0) {
                    fprintf(out, "Результат: ключ уже существует.\n");
                } else {
                    fprintf(out, "Результат: ошибка выделения памяти.\n");
                }
            }
            break;

        case 2:
            if (!parse_delete_command(line, key)) {
                print_invalid_command(out);
            } else if (!is_valid_key(key)) {
                fprintf(out, "Ошибка: ключ должен содержать только латинские буквы, длина 1..6.\n");
            } else {
                if (delete_value(tree, key)) {
                    fprintf(out, "Результат: узел удален.\n");
                } else {
                    fprintf(out, "Результат: ключ не найден.\n");
                }
            }
            break;

        case 3:
            if (!parse_print_command(line)) {
                print_invalid_command(out);
            } else {
                print_tree(tree, out);
            }
            break;

        case 4:
            if (!parse_search_command(line, key)) {
                print_invalid_command(out);
            } else if (!is_valid_key(key)) {
                fprintf(out, "Ошибка: ключ должен содержать только латинские буквы, длина 1..6.\n");
            } else {
                RBNode *node = search_node(tree, key);
                if (node == tree->nil) {
                    fprintf(out, "Результат: ключ не найден.\n");
                } else {
                    fprintf(out, "Результат: %.10g\n", node->value);
                }
            }
            break;

        default:
            print_invalid_command(out);
            break;
    }

    fprintf(out, "\n");
    return 1;
}

void print_menu(void) {
    printf("Выберите режим работы:\n");
    printf("1 - ввод с клавиатуры\n");
    printf("2 - чтение из input.txt, вывод в output.txt\n");
    printf("Ваш выбор: ");
}

int main(void) {
    RBTree tree;
    char line[MAX_LINE_LEN];
    FILE *in = NULL;
    FILE *out = NULL;
    int mode;

    if (!init_tree(&tree)) {
        fprintf(stderr, "Ошибка: не удалось создать дерево.\n");
        return 1;
    }

    print_menu();

    if (scanf("%d", &mode) != 1) {
        fprintf(stderr, "Ошибка: нужно ввести 1 или 2.\n");
        destroy_tree(&tree);
        return 1;
    }
    getchar();

    if (mode == 1) {
        in = stdin;
        out = stdout;

        printf("\nКоманды:\n");
        printf("1 <key> <value>  - добавить\n");
        printf("2 <key>          - удалить\n");
        printf("3                - печать дерева\n");
        printf("4 <key>          - поиск\n");
        printf("0                - выход\n");
        printf("Ключ: только латинские буквы, длина от 1 до 6.\n\n");
    } else if (mode == 2) {
        in = fopen("input.txt", "r");
        if (in == NULL) {
            fprintf(stderr, "Ошибка: не удалось открыть input.txt\n");
            destroy_tree(&tree);
            return 1;
        }

        out = fopen("output.txt", "w");
        if (out == NULL) {
            fprintf(stderr, "Ошибка: не удалось открыть output.txt\n");
            fclose(in);
            destroy_tree(&tree);
            return 1;
        }
    } else {
        fprintf(stderr, "Ошибка: допустимы только режимы 1 и 2.\n");
        destroy_tree(&tree);
        return 1;
    }

    while (1) {
        if (in == stdin) {
            printf("Введите команду: ");
            fflush(stdout);
        }

        if (fgets(line, sizeof(line), in) == NULL) {
            break;
        }

        trim_newline(line);

        if (line[0] == '\0') {
            continue;
        }

        if (!process_command(&tree, line, out)) {
            break;
        }
    }

    if (in != stdin) {
        fclose(in);
    }
    if (out != stdout) {
        fclose(out);
        printf("Результат записан в output.txt\n");
    }

    destroy_tree(&tree);
    return 0;
}