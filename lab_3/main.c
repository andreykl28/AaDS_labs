#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_NAME_LEN 64
#define MAX_INPUT_LEN 1024

typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_OPERATOR
} NodeType;

typedef struct Node {
    NodeType type;
    double value;
    char name[MAX_NAME_LEN];
    char op; /* + - * / ^ ~ , где ~ = унарный минус */
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    const char *text;
    int pos;
    int error;
    char message[256];
} Parser;

Node *create_number(double value) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        printf("Ошибка: не удалось выделить память\n");
        exit(1);
    }

    node->type = NODE_NUMBER;
    node->value = value;
    node->name[0] = '\0';
    node->op = '\0';
    node->left = NULL;
    node->right = NULL;

    return node;
}

Node *create_variable(const char *name) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        printf("Ошибка: не удалось выделить память\n");
        exit(1);
    }

    node->type = NODE_VARIABLE;
    node->value = 0.0;
    strncpy(node->name, name, MAX_NAME_LEN - 1);
    node->name[MAX_NAME_LEN - 1] = '\0';
    node->op = '\0';
    node->left = NULL;
    node->right = NULL;

    return node;
}

Node *create_operator(char op, Node *left, Node *right) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        printf("Ошибка: не удалось выделить память\n");
        exit(1);
    }

    node->type = NODE_OPERATOR;
    node->value = 0.0;
    node->name[0] = '\0';
    node->op = op;
    node->left = left;
    node->right = right;

    return node;
}

Node *clone_tree(const Node *root) {
    if (root == NULL) {
        return NULL;
    }

    Node *copy = (Node *)malloc(sizeof(Node));
    if (copy == NULL) {
        printf("Ошибка: не удалось выделить память\n");
        exit(1);
    }

    copy->type = root->type;
    copy->value = root->value;
    strcpy(copy->name, root->name);
    copy->op = root->op;
    copy->left = clone_tree(root->left);
    copy->right = clone_tree(root->right);

    return copy;
}

void free_tree(Node *root) {
    if (root == NULL) {
        return;
    }

    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

void set_error(Parser *p, const char *msg) {
    if (!p->error) {
        p->error = 1;
        strncpy(p->message, msg, sizeof(p->message) - 1);
        p->message[sizeof(p->message) - 1] = '\0';
    }
}

void skip_spaces(Parser *p) {
    while (isspace((unsigned char)p->text[p->pos])) {
        p->pos++;
    }
}

char current_char(Parser *p) {
    skip_spaces(p);
    return p->text[p->pos];
}

int match(Parser *p, char c) {
    skip_spaces(p);
    if (p->text[p->pos] == c) {
        p->pos++;
        return 1;
    }
    return 0;
}

Node *parse_expression(Parser *p);
Node *parse_term(Parser *p);
Node *parse_factor(Parser *p);
Node *parse_power(Parser *p);
Node *parse_primary(Parser *p);

Node *parse_number(Parser *p) {
    skip_spaces(p);

    const char *start = p->text + p->pos;
    char *endptr;
    double value = strtod(start, &endptr);

    if (endptr == start) {
        set_error(p, "Ожидалось число");
        return NULL;
    }

    p->pos += (int)(endptr - start);
    return create_number(value);
}

Node *parse_variable(Parser *p) {
    skip_spaces(p);

    if (!(isalpha((unsigned char)p->text[p->pos]) || p->text[p->pos] == '_')) {
        set_error(p, "Ожидалась переменная");
        return NULL;
    }

    char buffer[MAX_NAME_LEN];
    int i = 0;

    while (isalnum((unsigned char)p->text[p->pos]) || p->text[p->pos] == '_') {
        if (i < MAX_NAME_LEN - 1) {
            buffer[i++] = p->text[p->pos];
        }
        p->pos++;
    }

    buffer[i] = '\0';
    return create_variable(buffer);
}

Node *parse_primary(Parser *p) {
    skip_spaces(p);

    char c = current_char(p);

    if (c == '(') {
        p->pos++;
        Node *inside = parse_expression(p);

        if (p->error) {
            return NULL;
        }

        if (!match(p, ')')) {
            free_tree(inside);
            set_error(p, "Ожидалась закрывающая скобка ')'");
            return NULL;
        }

        return inside;
    }

    if (isdigit((unsigned char)c) || c == '.') {
        return parse_number(p);
    }

    if (isalpha((unsigned char)c) || c == '_') {
        return parse_variable(p);
    }

    set_error(p, "Неожиданный символ в выражении");
    return NULL;
}

Node *parse_power(Parser *p) {
    Node *left = parse_primary(p);
    if (p->error) {
        return NULL;
    }

    if (match(p, '^')) {
        Node *right = parse_factor(p);
        if (p->error) {
            free_tree(left);
            return NULL;
        }
        return create_operator('^', left, right);
    }

    return left;
}

Node *parse_factor(Parser *p) {
    skip_spaces(p);

    if (match(p, '+')) {
        return parse_factor(p);
    }

    if (match(p, '-')) {
        Node *right = parse_factor(p);
        if (p->error) {
            return NULL;
        }
        return create_operator('~', NULL, right);
    }

    return parse_power(p);
}

Node *parse_term(Parser *p) {
    Node *left = parse_factor(p);
    if (p->error) {
        return NULL;
    }

    while (1) {
        if (match(p, '*')) {
            Node *right = parse_factor(p);
            if (p->error) {
                free_tree(left);
                return NULL;
            }
            left = create_operator('*', left, right);
        } else if (match(p, '/')) {
            Node *right = parse_factor(p);
            if (p->error) {
                free_tree(left);
                return NULL;
            }
            left = create_operator('/', left, right);
        } else {
            break;
        }
    }

    return left;
}

Node *parse_expression(Parser *p) {
    Node *left = parse_term(p);
    if (p->error) {
        return NULL;
    }

    while (1) {
        if (match(p, '+')) {
            Node *right = parse_term(p);
            if (p->error) {
                free_tree(left);
                return NULL;
            }
            left = create_operator('+', left, right);
        } else if (match(p, '-')) {
            Node *right = parse_term(p);
            if (p->error) {
                free_tree(left);
                return NULL;
            }
            left = create_operator('-', left, right);
        } else {
            break;
        }
    }

    return left;
}

void print_indent(int n) {
    for (int i = 0; i < n; i++) {
        printf("    ");
    }
}

void print_number(double value) {
    double r = round(value);
    if (fabs(value - r) < 1e-9) {
        printf("%.0f", value);
    } else {
        printf("%g", value);
    }
}

void print_node_label(const Node *node) {
    if (node == NULL) {
        return;
    }

    if (node->type == NODE_NUMBER) {
        print_number(node->value);
    } else if (node->type == NODE_VARIABLE) {
        printf("%s", node->name);
    } else {
        if (node->op == '~') {
            printf("u-");
        } else {
            printf("%c", node->op);
        }
    }
}

void print_tree(const Node *root, int level) {
    if (root == NULL) {
        return;
    }

    print_tree(root->right, level + 1);
    print_indent(level);
    print_node_label(root);
    printf("\n");
    print_tree(root->left, level + 1);
}

int precedence_of_operator(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '~') return 3;
    if (op == '^') return 4;
    return 100;
}

int precedence_of_node(const Node *node) {
    if (node == NULL) return 0;
    if (node->type != NODE_OPERATOR) return 100;
    return precedence_of_operator(node->op);
}

int need_parentheses(const Node *child, char parent_op, int is_right_child) {
    if (child == NULL || child->type != NODE_OPERATOR) {
        return 0;
    }

    int child_prec = precedence_of_node(child);
    int parent_prec = precedence_of_operator(parent_op);

    if (child_prec < parent_prec) {
        return 1;
    }

    if (child_prec > parent_prec) {
        return 0;
    }

    if (parent_op == '+') {
        if (!is_right_child) return 0;
        return child->op == '-';
    }

    if (parent_op == '-') {
        if (!is_right_child) return 0;
        return 1;
    }

    if (parent_op == '*') {
        if (!is_right_child) return 0;
        return child->op == '/';
    }

    if (parent_op == '/') {
        if (!is_right_child) return 0;
        return 1;
    }

    if (parent_op == '^') {
        if (!is_right_child) return child->op == '^';
        return 0;
    }

    if (parent_op == '~') {
        return 1;
    }

    return 0;
}

void print_expression(const Node *root);

void print_unary_expression(const Node *root) {
    printf("-");

    if (root->right != NULL && root->right->type == NODE_OPERATOR &&
        precedence_of_node(root->right) <= precedence_of_operator('~')) {
        printf("(");
        print_expression(root->right);
        printf(")");
    } else {
        print_expression(root->right);
    }
}

void print_binary_expression(const Node *root) {
    int left_par = need_parentheses(root->left, root->op, 0);
    int right_par = need_parentheses(root->right, root->op, 1);

    if (left_par) printf("(");
    print_expression(root->left);
    if (left_par) printf(")");

    printf(" %c ", root->op);

    if (right_par) printf("(");
    print_expression(root->right);
    if (right_par) printf(")");
}

void print_expression(const Node *root) {
    if (root == NULL) {
        return;
    }

    if (root->type == NODE_NUMBER) {
        print_number(root->value);
        return;
    }

    if (root->type == NODE_VARIABLE) {
        printf("%s", root->name);
        return;
    }

    if (root->op == '~') {
        print_unary_expression(root);
        return;
    }

    print_binary_expression(root);
}

int get_integer_degree(const Node *node, long long *degree) {
    if (node == NULL || node->type != NODE_NUMBER) {
        return 0;
    }

    double r = round(node->value);
    if (fabs(node->value - r) > 1e-9) {
        return 0;
    }

    *degree = (long long)r;
    return 1;
}

Node *build_power_as_product(const Node *base, long long degree) {
    if (degree == 0) {
        return create_number(1);
    }

    Node *result = clone_tree(base);

    for (long long i = 2; i <= degree; i++) {
        result = create_operator('*', result, clone_tree(base));
    }

    return result;
}

Node *transform_powers(Node *root, int *count) {
    if (root == NULL) {
        return NULL;
    }

    if (root->type == NODE_OPERATOR) {
        root->left = transform_powers(root->left, count);
        root->right = transform_powers(root->right, count);

        if (root->op == '^' && root->left != NULL && root->right != NULL) {
            long long degree;

            if (get_integer_degree(root->right, &degree) && degree >= 0) {
                Node *replacement = build_power_as_product(root->left, degree);

                free_tree(root->left);
                free_tree(root->right);
                free(root);

                if (count != NULL) {
                    (*count)++;
                }

                return replacement;
            }
        }
    }

    return root;
}

int main(void) {
    char input[MAX_INPUT_LEN];

    printf("Введите арифметическое выражение:\n");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Ошибка чтения ввода\n");
        return 1;
    }

    input[strcspn(input, "\n")] = '\0';

    Parser parser;
    parser.text = input;
    parser.pos = 0;
    parser.error = 0;
    parser.message[0] = '\0';

    Node *root = parse_expression(&parser);

    skip_spaces(&parser);
    if (!parser.error && parser.text[parser.pos] != '\0') {
        parser.error = 1;
        strcpy(parser.message, "Лишние символы после конца выражения");
    }

    if (parser.error || root == NULL) {
        printf("Ошибка разбора: %s\n", parser.message);
        free_tree(root);
        return 1;
    }

    printf("\nИсходное выражение:\n");
    print_expression(root);
    printf("\n");

    printf("\nДерево исходного выражения:\n");
    print_tree(root, 0);

    int transform_count = 0;
    root = transform_powers(root, &transform_count);

    printf("\nКоличество выполненных преобразований: %d\n", transform_count);

    printf("\nПреобразованное дерево:\n");
    print_tree(root, 0);

    printf("\nПреобразованное выражение:\n");
    print_expression(root);
    printf("\n");

    free_tree(root);
    return 0;
}