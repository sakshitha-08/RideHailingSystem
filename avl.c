/*
 * avl.c
 * -----
 * Implementation of the Generic AVL tree for the Ride-Hailing System.
 *
 * BALANCING STRATEGY
 * ──────────────────
 * After every insertion or deletion we walk back up the path to the root
 * re-computing heights and checking the balance factor (BF) at each
 * ancestor node.
 *
 *   BF(node) = height(node->left) - height(node->right)
 *
 * If |BF| > 1 at any node we apply one of four rotations:
 *
 *   Case 1 – LL (right-rotate):  BF > 1 and BF(left child) >= 0
 *   Case 2 – LR (left-right):    BF > 1 and BF(left child) <  0
 *   Case 3 – RR (left-rotate):   BF < -1 and BF(right child) <= 0
 *   Case 4 – RL (right-left):    BF < -1 and BF(right child) >  0
 *
 * Each rotation is a simple pointer re-wiring that runs in O(1) time.
 * At most one rotation (or one double rotation) is needed per
 * insertion; deletions may require O(log n) rotations up the path.
 */

#include "avl.h"
#include <stdlib.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════
 * Internal helpers
 * ═══════════════════════════════════════════════════════════ */

/* NULL-safe height accessor */
int avl_height(AVLNode *n) {
    return n ? n->height : 0;
}

/* Recompute the stored height from children */
static void update_height(AVLNode *n) {
    int lh = avl_height(n->left);
    int rh = avl_height(n->right);
    n->height = 1 + (lh > rh ? lh : rh);
}

/* Balance factor: positive = left-heavy, negative = right-heavy */
static int balance_factor(AVLNode *n) {
    return n ? avl_height(n->left) - avl_height(n->right) : 0;
}

/* ── Single right rotation (LL case) ───────────────────────
 *
 *       z                y
 *      / \             /   \
 *     y   T4   →     x     z
 *    / \            / \   / \
 *   x   T3         T1 T2 T3 T4
 *  / \
 * T1  T2
 */
static AVLNode *rotate_right(AVLNode *z) {
    AVLNode *y  = z->left;
    AVLNode *T3 = y->right;

    y->right = z;
    z->left  = T3;

    update_height(z);
    update_height(y);
    return y;   /* new subtree root */
}

/* ── Single left rotation (RR case) ────────────────────────
 *
 *   z                  y
 *  / \               /   \
 * T1   y    →       z     x
 *     / \          / \   / \
 *    T2   x       T1 T2 T3 T4
 *        / \
 *       T3  T4
 */
static AVLNode *rotate_left(AVLNode *z) {
    AVLNode *y  = z->right;
    AVLNode *T2 = y->left;

    y->left  = z;
    z->right = T2;

    update_height(z);
    update_height(y);
    return y;
}

/* ── Rebalance after insert / delete ───────────────────────
 * Checks BF and applies the appropriate rotation(s). */
static AVLNode *rebalance(AVLNode *n) {
    update_height(n);
    int bf = balance_factor(n);

    /* LL */
    if (bf > 1 && balance_factor(n->left) >= 0)
        return rotate_right(n);

    /* LR */
    if (bf > 1 && balance_factor(n->left) < 0) {
        n->left = rotate_left(n->left);
        return rotate_right(n);
    }

    /* RR */
    if (bf < -1 && balance_factor(n->right) <= 0)
        return rotate_left(n);

    /* RL */
    if (bf < -1 && balance_factor(n->right) > 0) {
        n->right = rotate_right(n->right);
        return rotate_left(n);
    }

    return n;   /* already balanced */
}

/* ── Allocate a new leaf node ───────────────────────────── */
static AVLNode *new_node(int key, void *data) {
    AVLNode *n = (AVLNode *)malloc(sizeof(AVLNode));
    if (!n) { perror("avl: malloc"); exit(EXIT_FAILURE); }
    n->key    = key;
    n->data   = data;
    n->height = 1;
    n->left   = NULL;
    n->right  = NULL;
    return n;
}

/* ── Find in-order successor (leftmost node in right subtree) */
static AVLNode *min_node(AVLNode *n) {
    while (n->left) n = n->left;
    return n;
}

/* ═══════════════════════════════════════════════════════════
 * Recursive helpers (return updated subtree root)
 * ═══════════════════════════════════════════════════════════ */

static AVLNode *insert_rec(AVLNode *node, int key, void *data, int *inserted) {
    if (!node) { *inserted = 1; return new_node(key, data); }

    if (key < node->key)
        node->left  = insert_rec(node->left,  key, data, inserted);
    else if (key > node->key)
        node->right = insert_rec(node->right, key, data, inserted);
    else {
        *inserted = 0;   /* duplicate key – rejected */
        return node;
    }

    return rebalance(node);
}

static AVLNode *delete_rec(AVLNode *node, int key, int free_data, int *deleted) {
    if (!node) { *deleted = 0; return NULL; }

    if (key < node->key) {
        node->left  = delete_rec(node->left,  key, free_data, deleted);
    } else if (key > node->key) {
        node->right = delete_rec(node->right, key, free_data, deleted);
    } else {
        /* Found the node to delete */
        *deleted = 1;

        if (!node->left || !node->right) {
            /* 0 or 1 child */
            AVLNode *child = node->left ? node->left : node->right;
            if (free_data && node->data) free(node->data);
            free(node);
            return child;   /* may be NULL */
        }

        /* 2 children: replace with in-order successor's data */
        AVLNode *succ = min_node(node->right);
        node->key  = succ->key;
        node->data = succ->data;
        /* Delete successor (it has at most a right child) */
        int dummy = 0;
        node->right = delete_rec(node->right, succ->key, 0, &dummy);
    }

    return rebalance(node);
}

static void inorder_rec(AVLNode *node, AVLVisitor visitor, void *ctx) {
    if (!node) return;
    inorder_rec(node->left, visitor, ctx);
    visitor(node, ctx);
    inorder_rec(node->right, visitor, ctx);
}

static void range_rec(AVLNode *node, int lo, int hi,
                      AVLVisitor visitor, void *ctx) {
    if (!node) return;

    /* Prune left subtree entirely if all keys there are < lo */
    if (node->key > lo)
        range_rec(node->left, lo, hi, visitor, ctx);

    if (node->key >= lo && node->key <= hi)
        visitor(node, ctx);

    /* Prune right subtree entirely if all keys there are > hi */
    if (node->key < hi)
        range_rec(node->right, lo, hi, visitor, ctx);
}

static void free_rec(AVLNode *node, int free_data) {
    if (!node) return;
    free_rec(node->left,  free_data);
    free_rec(node->right, free_data);
    if (free_data && node->data) free(node->data);
    free(node);
}

/* ═══════════════════════════════════════════════════════════
 * Public API
 * ═══════════════════════════════════════════════════════════ */

void avl_init(AVLTree *tree) {
    tree->root  = NULL;
    tree->count = 0;
}

int avl_insert(AVLTree *tree, int key, void *data) {
    int inserted = 0;
    tree->root = insert_rec(tree->root, key, data, &inserted);
    if (inserted) tree->count++;
    return inserted;
}

int avl_delete(AVLTree *tree, int key, int free_data) {
    int deleted = 0;
    tree->root = delete_rec(tree->root, key, free_data, &deleted);
    if (deleted) tree->count--;
    return deleted;
}

AVLNode *avl_search(AVLTree *tree, int key) {
    AVLNode *cur = tree->root;
    while (cur) {
        if      (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else                     return cur;
    }
    return NULL;
}

void avl_inorder(AVLTree *tree, AVLVisitor visitor, void *ctx) {
    inorder_rec(tree->root, visitor, ctx);
}

void avl_range(AVLTree *tree, int lo, int hi, AVLVisitor visitor, void *ctx) {
    range_rec(tree->root, lo, hi, visitor, ctx);
}

void avl_free_tree(AVLTree *tree, int free_data) {
    free_rec(tree->root, free_data);
    tree->root  = NULL;
    tree->count = 0;
}
