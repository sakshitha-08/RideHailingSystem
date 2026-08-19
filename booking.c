/*
 * booking.c
 * ---------
 * Implementation of the Booking database and all ride-lifecycle functions.
 * AVL tree keyed on booking_id.
 */

#include "booking.h"
#include "driver.h"
#include "passenger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── Module-level AVL tree & counter ─────────────────────── */
AVLTree bookingDB;
int     nextBookingId = 1;   /* starts at 1; persisted between runs */

/* ═══════════════════════════════════════════════════════════
 * Init / Teardown
 * ═══════════════════════════════════════════════════════════ */

void booking_db_init(void) {
    avl_init(&bookingDB);
}

void booking_db_free(void) {
    avl_free_tree(&bookingDB, 1);
}

/* ═══════════════════════════════════════════════════════════
 * requestRide
 * ═══════════════════════════════════════════════════════════ */

int requestRide(int p_id, int p_x, int p_y, int prefType) {
    /* Verify passenger exists */
    Passenger *p = getPassengerById(p_id);
    if (!p) {
        printf("[Booking] ERROR: Passenger ID %d not found.\n", p_id);
        return -1;
    }

    /* Find nearest free vehicle */
    Driver *d = findNearestVehicle(p_x, p_y, prefType);
    if (!d) {
        const char *typeStr =
            (prefType == VEHICLE_CAB)  ? "cab"  :
            (prefType == VEHICLE_BIKE) ? "bike" : "vehicle";
        printf("[Booking] No free %s available within 5 km of (%d,%d).\n",
               typeStr, p_x, p_y);
        return -1;
    }

    /* Create booking */
    Booking *b = (Booking *)malloc(sizeof(Booking));
    if (!b) { perror("requestRide: malloc"); return -1; }

    b->booking_id        = nextBookingId++;
    b->d_ID              = d->d_ID;
    b->p_ID              = p_id;
    b->vehicle_type      = d->vehicle_type;
    b->distance_travelled = 0.0f;
    b->fare              = 0.0f;
    b->timestamp         = (long)time(NULL);

    /* Mark driver as booked */
    d->status = DRIVER_BOOKED;

    avl_insert(&bookingDB, b->booking_id, b);

    printf("[Booking] Ride booked!\n");
    printf("          Booking ID : %d\n",  b->booking_id);
    printf("          Passenger  : %s (ID=%d)\n", p->name, p_id);
    printf("          Driver     : %s (ID=%d)\n", d->name, d->d_ID);
    printf("          Vehicle    : %s\n",
           (d->vehicle_type == VEHICLE_CAB) ? "Cab" : "Bike");

    return b->booking_id;
}

/* ═══════════════════════════════════════════════════════════
 * completeRide
 * ═══════════════════════════════════════════════════════════ */

int completeRide(int booking_id, float distance) {
    AVLNode *node = avl_search(&bookingDB, booking_id);
    if (!node) {
        printf("[Booking] ERROR: Booking ID %d not found.\n", booking_id);
        return 0;
    }

    Booking *b = (Booking *)node->data;

    if (distance < 0.0f) {
        printf("[Booking] ERROR: Distance cannot be negative.\n");
        return 0;
    }

    /* Calculate fare */
    float rate = (b->vehicle_type == VEHICLE_CAB) ? FARE_CAB : FARE_BIKE;
    b->distance_travelled = distance;
    b->fare               = distance * rate;

    /* Update driver */
    Driver *d = getDriverById(b->d_ID);
    if (d) {
        d->total_earnings += b->fare;
        d->status          = DRIVER_FREE;
    }

    /* Update passenger frequency */
    Passenger *p = getPassengerById(b->p_ID);
    if (p) p->frequency++;

    printf("[Booking] Ride completed!\n");
    printf("          Booking ID : %d\n",   booking_id);
    printf("          Distance   : %.2f km\n", distance);
    printf("          Fare       : ₹%.2f\n",   b->fare);
    if (d) printf("          Driver     : %s  (Total earnings ₹%.2f)\n",
                  d->name, d->total_earnings);
    if (p) printf("          Passenger  : %s  (Total rides %d)\n",
                  p->name, p->frequency);

    return 1;
}

/* ═══════════════════════════════════════════════════════════
 * calculateDriverEarnings
 * Traverses the Booking DB and sums fares for the given driver.
 * ═══════════════════════════════════════════════════════════ */

typedef struct { int d_id; float total; } EarningsCtx;

static void sum_fare(AVLNode *node, void *ctx) {
    EarningsCtx *ec = (EarningsCtx *)ctx;
    Booking     *b  = (Booking *)node->data;
    if (b->d_ID == ec->d_id)
        ec->total += b->fare;
}

float calculateDriverEarnings(int d_id) {
    Driver *d = getDriverById(d_id);
    if (!d) {
        printf("[Booking] ERROR: Driver ID %d not found.\n", d_id);
        return -1.0f;
    }

    EarningsCtx ec = { d_id, 0.0f };
    avl_inorder(&bookingDB, sum_fare, &ec);

    printf("[Booking] Driver %s (ID=%d) – Earnings from booking DB: ₹%.2f\n",
           d->name, d_id, ec.total);
    return ec.total;
}

/* ═══════════════════════════════════════════════════════════
 * displayBookingHistory
 * ═══════════════════════════════════════════════════════════ */

static void print_booking(AVLNode *node, void *ctx) {
    (void)ctx;
    Booking *b = (Booking *)node->data;
    printf("  BID=%-4d  DID=%-4d  PID=%-4d  %-4s  %6.2f km  ₹%8.2f\n",
           b->booking_id, b->d_ID, b->p_ID,
           (b->vehicle_type == VEHICLE_CAB) ? "Cab" : "Bike",
           b->distance_travelled, b->fare);
}

void displayBookingHistory(void) {
    if (bookingDB.count == 0) {
        printf("[Booking] No bookings yet.\n");
        return;
    }

    printf("\n── Booking History ────────────────────────────────\n");
    printf("  %-6s %-6s %-6s %-4s %-10s %s\n",
           "B-ID", "D-ID", "P-ID", "Type", "Distance", "Fare");
    printf("  %-6s %-6s %-6s %-4s %-10s %s\n",
           "────", "────", "────", "────", "────────", "────");
    avl_inorder(&bookingDB, print_booking, NULL);
    printf("───────────────────────────────────────────────────\n");
}

/* ═══════════════════════════════════════════════════════════
 * displayFrequentPairs
 * Find the (d_ID, p_ID) pair with the most rides together.
 * Uses a simple O(n^2) accumulation over the booking list;
 * adequate for the expected dataset sizes in a college project.
 * ═══════════════════════════════════════════════════════════ */

/* Step 1: collect all bookings into an array */
typedef struct { Booking **arr; int n; int cap; } BookingArr;

static void collect_booking(AVLNode *node, void *ctx) {
    BookingArr *ba = (BookingArr *)ctx;
    if (ba->n < ba->cap)
        ba->arr[ba->n++] = (Booking *)node->data;
}

void displayFrequentPairs(void) {
    if (bookingDB.count == 0) {
        printf("[Booking] No bookings to analyse.\n");
        return;
    }

    /* Collect all bookings */
    int cap = bookingDB.count;
    Booking **arr = (Booking **)malloc(cap * sizeof(Booking *));
    if (!arr) { perror("displayFrequentPairs: malloc"); return; }

    BookingArr ba = { arr, 0, cap };
    avl_inorder(&bookingDB, collect_booking, &ba);
    int n = ba.n;

    /* Count pair occurrences: O(n²) */
    int best_d = -1, best_p = -1, best_count = 0;

    for (int i = 0; i < n; i++) {
        int di = arr[i]->d_ID;
        int pi = arr[i]->p_ID;
        int cnt = 0;
        for (int j = 0; j < n; j++) {
            if (arr[j]->d_ID == di && arr[j]->p_ID == pi) cnt++;
        }
        if (cnt > best_count) {
            best_count = cnt;
            best_d     = di;
            best_p     = pi;
        }
    }

    free(arr);

    if (best_d == -1) {
        printf("[Booking] No frequent pair found.\n");
        return;
    }

    Driver    *d = getDriverById(best_d);
    Passenger *p = getPassengerById(best_p);

    printf("\n── Most Frequent Pair ─────────────────────────────\n");
    printf("  Driver   : %s (ID=%d)\n",
           d ? d->name : "Unknown", best_d);
    printf("  Passenger: %s (ID=%d)\n",
           p ? p->name : "Unknown", best_p);
    printf("  Shared rides: %d\n", best_count);
    printf("───────────────────────────────────────────────────\n");
}
