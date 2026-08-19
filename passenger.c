/*
 * passenger.c
 * -----------
 * Implementation of the Passenger database.
 * AVL tree keyed on p_ID.
 */

#include "passenger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Module-level AVL tree ───────────────────────────────── */
AVLTree passengerDB;

/* ═══════════════════════════════════════════════════════════
 * Init / Teardown
 * ═══════════════════════════════════════════════════════════ */

void passenger_db_init(void) {
    avl_init(&passengerDB);
}

void passenger_db_free(void) {
    avl_free_tree(&passengerDB, 1);
}

/* ═══════════════════════════════════════════════════════════
 * Mobile-duplicate check via inorder traversal
 * ═══════════════════════════════════════════════════════════ */

typedef struct { const char *mobile; int found; } MobileCtx;

static void check_mobile(AVLNode *node, void *ctx) {
    MobileCtx  *mc = (MobileCtx *)ctx;
    Passenger  *p  = (Passenger *)node->data;
    if (!mc->found && strcmp(p->mobile, mc->mobile) == 0)
        mc->found = 1;
}

/* ═══════════════════════════════════════════════════════════
 * addPassenger
 * ═══════════════════════════════════════════════════════════ */

int addPassenger(int id, const char *name, const char *mobile) {
    /* Check duplicate p_ID */
    if (avl_search(&passengerDB, id)) {
        printf("[Passenger] ERROR: Passenger with ID %d already exists.\n", id);
        return 0;
    }

    /* Check duplicate mobile */
    MobileCtx mc = { mobile, 0 };
    avl_inorder(&passengerDB, check_mobile, &mc);
    if (mc.found) {
        printf("[Passenger] ERROR: Mobile number %s already registered.\n", mobile);
        return 0;
    }

    Passenger *p = (Passenger *)malloc(sizeof(Passenger));
    if (!p) { perror("addPassenger: malloc"); return 0; }

    p->p_ID     = id;
    strncpy(p->name,   name,   sizeof(p->name)   - 1);
    strncpy(p->mobile, mobile, sizeof(p->mobile) - 1);
    p->name[sizeof(p->name) - 1]     = '\0';
    p->mobile[sizeof(p->mobile) - 1] = '\0';
    p->frequency = 0;

    avl_insert(&passengerDB, id, p);
    printf("[Passenger] Added: ID=%d  Name=%s  Mobile=%s\n", id, name, mobile);
    return 1;
}

/* ═══════════════════════════════════════════════════════════
 * rangeSearchPassengers
 * ═══════════════════════════════════════════════════════════ */

static void print_passenger(AVLNode *node, void *ctx) {
    (void)ctx;
    Passenger *p = (Passenger *)node->data;
    printf("  ID=%-4d  %-20s  Mobile=%-15s  Rides=%d\n",
           p->p_ID, p->name, p->mobile, p->frequency);
}

void rangeSearchPassengers(int lo, int hi) {
    if (lo > hi) {
        printf("[Passenger] ERROR: Invalid range [%d, %d].\n", lo, hi);
        return;
    }

    printf("\n── Passengers with ID in [%d, %d] ─────────────────\n", lo, hi);
    printf("  %-6s %-20s %-15s Rides\n", "ID", "Name", "Mobile");
    printf("  %-6s %-20s %-15s ─────\n", "──", "────", "──────");

    /* AVL range traversal uses in-order, so results are in ascending p_ID order */
    avl_range(&passengerDB, lo, hi, print_passenger, NULL);
    printf("───────────────────────────────────────────────────\n");
}

/* ═══════════════════════════════════════════════════════════
 * getPassengerById
 * ═══════════════════════════════════════════════════════════ */

Passenger *getPassengerById(int p_id) {
    AVLNode *node = avl_search(&passengerDB, p_id);
    return node ? (Passenger *)node->data : NULL;
}
