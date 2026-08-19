/*
 * avl.h
 * -----
 * Generic AVL-tree interface for the Ride-Hailing System.
 *
 * DESIGN CHOICE – Why AVL trees?
 * ───────────────────────────────
 * An AVL tree guarantees O(log n) worst-case time for search, insert, and
 * delete by maintaining the AVL balance invariant: for every node, the
 * heights of its left and right subtrees differ by at most 1.  When an
 * insertion or deletion causes a violation the tree is restored with one of
 * four local rotations (LL, RR, LR, RL).  This makes AVL trees ideal for
 * an in-memory, single-process application like this one.
 *
 * Each AVL node carries a generic (void *) payload plus an integer key.
 * The three databases use different key spaces:
 *   • Driver DB      → keyed on d_ID
 *   • Passenger DB   → keyed on p_ID
 *   • Booking DB     → keyed on booking_id
 *
 * Traversal functions accept a callback so callers can apply any operation
 * (print, accumulate, search) without knowing internal tree details.
 */

#ifndef AVL_H
#define AVL_H

#include <stddef.h>   /* size_t */

/* ── Node ────────────────────────────────────────────────── */
typedef struct AVLNode {
    int            key;     /* search key (unique per tree)   */
    void          *data;    /* heap-allocated payload         */
    int            height;  /* height of this subtree         */
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

/* ── Tree handle ─────────────────────────────────────────── */
typedef struct {
    AVLNode *root;
    int      count;         /* number of nodes currently in tree */
} AVLTree;

/* ── Callback types ─────────────────────────────────────── */
/* Called during traversal; ctx is any caller-supplied context pointer */
typedef void (*AVLVisitor)(AVLNode *node, void *ctx);

/* ── Public API ──────────────────────────────────────────── */

/* Initialise an empty tree (call before first use). */
void avl_init(AVLTree *tree);

/*
 * Insert a new key/data pair.
 * Returns 1 on success, 0 if key already exists (no duplicate keys).
 * The caller must heap-allocate *data; the tree does NOT copy it.
 */
int avl_insert(AVLTree *tree, int key, void *data);

/*
 * Delete the node with the given key.
 * The payload pointed to by data is freed with free() if free_data != 0.
 * Returns 1 if found and deleted, 0 if key not present.
 */
int avl_delete(AVLTree *tree, int key, int free_data);

/*
 * Search for a key.
 * Returns a pointer to the AVLNode, or NULL if not found.
 */
AVLNode *avl_search(AVLTree *tree, int key);

/*
 * In-order traversal (ascending key order).
 * Calls visitor(node, ctx) for every node.
 */
void avl_inorder(AVLTree *tree, AVLVisitor visitor, void *ctx);

/*
 * In-order traversal restricted to keys in [lo, hi] inclusive.
 * Useful for range-search queries.
 */
void avl_range(AVLTree *tree, int lo, int hi, AVLVisitor visitor, void *ctx);

/*
 * Free every node (and optionally its payload) in the tree.
 * After this call tree->root == NULL and tree->count == 0.
 */
void avl_free_tree(AVLTree *tree, int free_data);

/* ── Utility ─────────────────────────────────────────────── */
int avl_height(AVLNode *n);   /* safe NULL-aware height getter */

#endif /* AVL_H */
