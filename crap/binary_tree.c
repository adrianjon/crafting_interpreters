#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// Node structure for the binary tree
typedef struct Node {
    char op; // Operator: '+', '-', '*', '/' or '\0' for numbers
    double value; // Used if op == '\0'
    struct Node *left, *right;
} Node;

typedef struct {
    void* value;
    enum { NUMBER, OPERATOR } type;
    Node* parent;
} Leaf;

// Create a new number node
Node* new_number(double value) {
    Node* node = malloc(sizeof(Node));
    node->op = '\0';
    node->value = value;
    node->left = node->right = NULL;
    return node;
}

// Create a new operator node
Node* new_operator(char op, Node* left, Node* right) {
    Node* node = malloc(sizeof(Node));
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

// Evaluate the expression tree
double eval(Node* root) {
    if (!root) return 0;
    if (root->op == '\0') return root->value;
    double l = eval(root->left);
    double r = eval(root->right);
    switch (root->op) {
        case '+': return l + r;
        case '-': return l - r;
        case '*': return l * r;
        case '/': return l / r;
        default: return 0;
    }
}

// Free the tree
void free_tree(Node* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// Example usage: ((3 + 4) * (5 - 2))
int main() {
    Node* n1 = new_number(3);
    Node* n2 = new_number(4);
    Node* n3 = new_operator('+', n1, n2);

    Node* n4 = new_number(5);
    Node* n5 = new_number(2);
    Node* n6 = new_operator('-', n4, n5);

    Node* root = new_operator('*', n3, n6);

    printf("Result: %f\n", eval(root)); // Output: 21.000000

    free_tree(root);
    return 0;
}