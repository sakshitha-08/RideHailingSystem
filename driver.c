/*
 * driver.c
 * --------
 * Implementation of the Driver database and all driver operations.
 *
 * The Driver AVL tree is keyed on d_ID (unique positive integer).
 * All tree operations run in O(log n) time.
 *
 * findNearestVehicle performs a full in-order traversal (O(n)) because
 * location-based nearest-neighbour search cannot be accelerated by a
 * 1-D key-based tree; a k-d tree would be needed for O(log n) spatial
 * queries, but that is outside the scope of this assignment.
 */

#include "driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SEARCH_RADIUS 5.0   /* km */

/* ── Module-level AVL tree ───────────────────────────────── */
AVLTree driverDB;

/* ═══════════════════════════════════════════════════════════
 * Init / Teardown
 * ═══════════════════════════════════════════════════════════ */

void driver_db_init(void) {
    avl_init(&driverDB);
}

void driver_db_free(void) {
    avl_free_tree(&driverDB, 1);   /* 1 = also free the Driver payload */
}

/* ═══════════════════════════════════════════════════════════
 * addDriver
 * ═══════════════════════════════════════════════════════════ */

int addDriver(int id, const char *name, int type, int x, int y) {
    if (avl_search(&driverDB, id)) {
        printf("[Driver] ERROR: Driver with ID %d already exists.\n", id);
        return 0;
    }

    Driver *d = (Driver *)malloc(sizeof(Driver));
    if (!d) { perror("addDriver: malloc"); return 0; }

    d->d_ID          = id;
    strncpy(d->name, name, sizeof(d->name) - 1);
    d->name[sizeof(d->name) - 1] = '\0';
    d->vehicle_type  = type;
    d->loc.x         = x;
    d->loc.y         = y;
    d->status        = DRIVER_FREE;
    d->total_earnings = 0.0f;

    avl_insert(&driverDB, id, d);
    printf("[Driver] Added: ID=%d  Name=%s  Type=%s  Location=(%d,%d)\n",
           id, name,
           (type == VEHICLE_CAB) ? "Cab" : "Bike",
           x, y);
    return 1;
}

/* ═══════════════════════════════════════════════════════════
 * deleteDriver
 * ═══════════════════════════════════════════════════════════ */

int deleteDriver(int d_id) {
    AVLNode *node = avl_search(&driverDB, d_id);
    if (!node) {
        printf("[Driver] ERROR: Driver ID %d not found.\n", d_id);
        return 0;
    }

    Driver *d = (Driver *)node->data;
    if (d->status == DRIVER_BOOKED) {
        printf("[Driver] ERROR: Cannot delete Driver %d — currently booked.\n", d_id);
        return 0;
    }

    printf("[Driver] Deleted: ID=%d  Name=%s\n", d->d_ID, d->name);
    avl_delete(&driverDB, d_id, 1);   /* free payload */
    return 1;
}

/* ═══════════════════════════════════════════════════════════
 * updateDriverLocation
 * ═══════════════════════════════════════════════════════════ */

int updateDriverLocation(int d_id, int new_x, int new_y) {
    AVLNode *node = avl_search(&driverDB, d_id);
    if (!node) {
        printf("[Driver] ERROR: Driver ID %d not found.\n", d_id);
        return 0;
    }

    Driver *d = (Driver *)node->data;
    printf("[Driver] Location updated: ID=%d  (%d,%d) → (%d,%d)%s\n",
           d_id, d->loc.x, d->loc.y, new_x, new_y,
           (d->status == DRIVER_BOOKED) ? "  [driver is currently booked]" : "");
    d->loc.x = new_x;
    d->loc.y = new_y;
    return 1;
}

/* ═══════════════════════════════════════════════════════════
 * findNearestVehicle – context struct for the traversal callback
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    int     p_x, p_y;     /* passenger position    */
    int     prefType;      /* requested vehicle type */
    Driver *best;          /* nearest driver so far  */
    double  bestDist;      /* corresponding distance  */
} NearestCtx;

static void nearest_visitor(AVLNode *node, void *ctx) {
    NearestCtx *nc = (NearestCtx *)ctx;
    Driver     *d  = (Driver *)node->data;

    /* Skip booked drivers */
    if (d->status != DRIVER_FREE) return;

    /* Skip wrong vehicle type (unless caller accepts any) */
    if (nc->prefType != VEHICLE_ANY && d->vehicle_type != nc->prefType) return;

    /* Euclidean distance in km */
    double dx = d->loc.x - nc->p_x;
    double dy = d->loc.y - nc->p_y;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist <= SEARCH_RADIUS && dist < nc->bestDist) {
        nc->bestDist = dist;
        nc->best     = d;
    }
}

Driver *findNearestVehicle(int p_x, int p_y, int prefType) {
    NearestCtx ctx = { p_x, p_y, prefType, NULL, SEARCH_RADIUS + 1.0 };
    avl_inorder(&driverDB, nearest_visitor, &ctx);
    return ctx.best;
}

/* ═══════════════════════════════════════════════════════════
 * displayAvailableVehicles
 * ═══════════════════════════════════════════════════════════ */

static void print_free_driver(AVLNode *node, void *ctx) {
    (void)ctx;
    Driver *d = (Driver *)node->data;
    if (d->status == DRIVER_FREE) {
        printf("  ID=%-4d  %-20s  %-4s  Location=(%d,%d)\n",
               d->d_ID, d->name,
               (d->vehicle_type == VEHICLE_CAB) ? "Cab" : "Bike",
               d->loc.x, d->loc.y);
    }
}

void displayAvailableVehicles(void) {
    if (driverDB.count == 0) {
        printf("[Driver] No drivers registered.\n");
        return;
    }
    printf("\n── Available Vehicles ─────────────────────────────\n");
    printf("  %-6s %-20s %-4s Location\n", "ID", "Name", "Type");
    printf("  %-6s %-20s %-4s --------\n", "──", "────", "────");
    avl_inorder(&driverDB, print_free_driver, NULL);
    printf("───────────────────────────────────────────────────\n");
}

/* ═══════════════════════════════════════════════════════════
 * displayTopDrivers – sort by total_earnings, print top 3
 * ═══════════════════════════════════════════════════════════ */

/* Collect all drivers into an array, then sort */
typedef struct { Driver *arr[1024]; int n; } DriverArr;

static void collect_driver(AVLNode *node, void *ctx) {
    DriverArr *da = (DriverArr *)ctx;
    if (da->n < 1024)
        da->arr[da->n++] = (Driver *)node->data;
}

static int cmp_earnings_desc(const void *a, const void *b) {
    float ea = (*(Driver **)a)->total_earnings;
    float eb = (*(Driver **)b)->total_earnings;
    return (ea < eb) - (ea > eb);   /* descending */
}

void displayTopDrivers(void) {
    DriverArr da = { {0}, 0 };
    avl_inorder(&driverDB, collect_driver, &da);

    if (da.n == 0) {
        printf("[Driver] No drivers to rank.\n");
        return;
    }

    qsort(da.arr, da.n, sizeof(Driver *), cmp_earnings_desc);

    printf("\n── Top Drivers by Earnings ────────────────────────\n");
    int top = (da.n < 3) ? da.n : 3;
    for (int i = 0; i < top; i++) {
        printf("  %d. %-20s  ₹%.2f\n", i + 1,
               da.arr[i]->name, da.arr[i]->total_earnings);
    }
    printf("───────────────────────────────────────────────────\n");
}

/* ═══════════════════════════════════════════════════════════
 * getDriverById – thin wrapper used by booking.c
 * ═══════════════════════════════════════════════════════════ */

Driver *getDriverById(int d_id) {
    AVLNode *node = avl_search(&driverDB, d_id);
    return node ? (Driver *)node->data : NULL;
}
