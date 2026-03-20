#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int id;
    int data;
    struct Node* firstChild;
    struct Node* nextSibling;   
} Node;

Node* createNode(int id, int data) {
    Node* node = (Node*)malloc(sizeof(Node));

    if (node == NULL) {
        printf("Ошибка выделения памяти\n");
        exit(1);
    }

    node->id = id;
    node->data = data;
    node->firstChild = NULL;
    node->nextSibling = NULL;
    return node;
}

Node* findNode(Node* root, int id) {
    if (root == NULL) {
        return NULL;
    }

    if (root->id == id) {
        return root;
    }

    Node* found = findNode(root->firstChild, id);

    if (found != NULL) {
        return found;
    }
    return findNode(root->nextSibling, id);
}

Node* addNode(Node* root, int parentId, int id, int data) {
    if (root == NULL) {
        return createNode(id, data);
    }

    if (findNode(root, id) != NULL) {
        printf("Узел с id %d уже существует.\n", id);
        return root;
    }

    Node* parent = findNode(root, parentId);

    if (parent == NULL) {
        printf("Родитель с id %d не найден.\n", parentId);
        return root;
    }

    Node* newNode = createNode(id, data);

    if (parent->firstChild == NULL) {
        parent->firstChild = newNode;
    } else {
        Node* sibling = parent->firstChild;
        while (sibling->nextSibling != NULL) {
            sibling = sibling->nextSibling;
        }
        sibling->nextSibling = newNode;
    }
    return root;
}

int getDegree(Node* node) {
    int count = 0;
    Node* child = node->firstChild;

    while (child != NULL) {
        count++;
        child = child->nextSibling;
    }
    return count;
}

void freeTree(Node* root) {
    if (root == NULL) {
        return;
    }

    freeTree(root->firstChild);
    freeTree(root->nextSibling);
    free(root);
}

Node* deleteNode(Node* root, int id) {
    if (root == NULL) {
        return NULL;
    }

    if (root->id == id) {
        Node* sibling = root->nextSibling;
        root->nextSibling = NULL;
        freeTree(root);
        return sibling;
    }

    root->firstChild = deleteNode(root->firstChild, id);
    root->nextSibling = deleteNode(root->nextSibling, id);
    return root;
}

int countMatchingNodes(Node* root) {
    if (root == NULL) {
        return 0;
    }

    int count = 0;

    if (getDegree(root) == root->data) {
        count++;
    }
    
    count += countMatchingNodes(root->firstChild);
    count += countMatchingNodes(root->nextSibling);
    return count;
}

void printTree(Node* root, int level) {
    if (root == NULL) {
        return;
    }
    for (int i = 0; i < level; i++) {
        printf("    ");
    }
    printf("[%d] %d\n", root->id, root->data);
    printTree(root->firstChild, level + 1);
    printTree(root->nextSibling, level);
}

int main() {
    Node* root = NULL;
    int choice;
    int id, data, parentId;

    do {
        printf("\nМеню:\n");
        printf("1. Добавить узел\n");
        printf("2. Вывести дерево\n");
        printf("3. Удалить узел\n");
        printf("4. Подсчитать число вершин, у которых степень равна значению\n");
        printf("0. Выход\n");
        printf("Ваш выбор: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                printf("Введите id нового узла: ");
                scanf("%d", &id);

                printf("Введите значение нового узла: ");
                scanf("%d", &data);

                if (root == NULL) {
                    root = addNode(root, 0, id, data);
                    printf("Корень создан.\n");
                } else {
                    printf("Введите id родителя: ");
                    scanf("%d", &parentId);
                    root = addNode(root, parentId, id, data);
                }
                break;

            case 2:
                if (root == NULL) {
                    printf("Дерево пусто.\n");
                } else {
                    printf("Дерево:\n");
                    printTree(root, 0);
                }
                break;

            case 3:
                if (root == NULL) {
                    printf("Дерево пусто.\n");
                } else {
                    printf("Введите id узла для удаления: ");
                    scanf("%d", &id);
                    root = deleteNode(root, id);
                }
                break;

            case 4:
                if (root == NULL) {
                    printf("Дерево пусто.\n");
                } else {
                    printf("Количество подходящих вершин: %d\n", countMatchingNodes(root));
                }
                break;

            case 0:
                break;

            default:
                printf("Неверный пункт меню.\n");
        }

    } while (choice != 0);

    freeTree(root);
    return 0;
}