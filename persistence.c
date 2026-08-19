/*
 * persistence.c
 * -------------
 * Binary file I/O to persist all three AVL databases between program runs.
 *
 * SAVE strategy
 * ─────────────
 * Each tree is traversed in-order.  For every visited node its payload
 * struct is written with fwrite().  A leading count (int) tells the reader
 * how many records to expect.
 *
 * LOAD strategy
 * ─────────────
 * Read the count, then read that many structs.  Each struct is heap-allocated
 * and inserted into the appropriate AVL tree.  The tree automatically
 * rebalances on insertion, so we recover full O(log n) performance.
 *
 * nextBookingId is written/read as a trailing int in bookings.dat.
 */

#include "persistence.h"
#include "driver.h"
#include "passenger.h"
#include "booking.h"
#include "models.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DRIVER_FILE    "drivers.dat"
#define PASSENGER_FILE "passengers.dat"
#define BOOKING_FILE   "bookings.dat"

/* ── Collector callbacks ──────────────────────────────────── */

typedef struct { FILE *fp; int written; } WriteCtx;

static void write_driver(AVLNode *node, void *ctx) {
    WriteCtx *wc = (WriteCtx *)ctx;
    fwrite(node->data, sizeof(Driver), 1, wc->fp);
    wc->written++;
}

static void write_passenger(AVLNode *node, void *ctx) {
    WriteCtx *wc = (WriteCtx *)ctx;
    fwrite(node->data, sizeof(Passenger), 1, wc->fp);
    wc->written++;
}

static void write_booking(AVLNode *node, void *ctx) {
    WriteCtx *wc = (WriteCtx *)ctx;
    fwrite(node->data, sizeof(Booking), 1, wc->fp);
    wc->written++;
}

/* ═══════════════════════════════════════════════════════════
 * saveAll
 * ═══════════════════════════════════════════════════════════ */

void saveAll(void) {
    FILE *fp;
    int  count;

    /* ── Drivers ── */
    fp = fopen(DRIVER_FILE, "wb");
    if (!fp) { perror("saveAll: " DRIVER_FILE); }
    else {
        count = driverDB.count;
        fwrite(&count, sizeof(int), 1, fp);
        WriteCtx wc = { fp, 0 };
        avl_inorder(&driverDB, write_driver, &wc);
        fclose(fp);
        printf("[Persist] Saved %d driver(s) to %s\n", wc.written, DRIVER_FILE);
    }

    /* ── Passengers ── */
    fp = fopen(PASSENGER_FILE, "wb");
    if (!fp) { perror("saveAll: " PASSENGER_FILE); }
    else {
        count = passengerDB.count;
        fwrite(&count, sizeof(int), 1, fp);
        WriteCtx wc = { fp, 0 };
        avl_inorder(&passengerDB, write_passenger, &wc);
        fclose(fp);
        printf("[Persist] Saved %d passenger(s) to %s\n", wc.written, PASSENGER_FILE);
    }

    /* ── Bookings + nextBookingId ── */
    fp = fopen(BOOKING_FILE, "wb");
    if (!fp) { perror("saveAll: " BOOKING_FILE); }
    else {
        count = bookingDB.count;
        fwrite(&count, sizeof(int), 1, fp);
        WriteCtx wc = { fp, 0 };
        avl_inorder(&bookingDB, write_booking, &wc);
        /* persist the counter so IDs stay unique across runs */
        fwrite(&nextBookingId, sizeof(int), 1, fp);
        fclose(fp);
        printf("[Persist] Saved %d booking(s) to %s  (nextID=%d)\n",
               wc.written, BOOKING_FILE, nextBookingId);
    }
}

/* ═══════════════════════════════════════════════════════════
 * loadAll
 * ═══════════════════════════════════════════════════════════ */

void loadAll(void) {
    FILE *fp;
    int   count, i;

    /* ── Drivers ── */
    fp = fopen(DRIVER_FILE, "rb");
    if (!fp) {
        printf("[Persist] %s not found – starting with empty driver DB.\n",
               DRIVER_FILE);
    } else {
        fread(&count, sizeof(int), 1, fp);
        for (i = 0; i < count; i++) {
            Driver *d = (Driver *)malloc(sizeof(Driver));
            if (!d) { perror("loadAll: driver malloc"); break; }
            if (fread(d, sizeof(Driver), 1, fp) != 1) { free(d); break; }
            avl_insert(&driverDB, d->d_ID, d);
        }
        fclose(fp);
        printf("[Persist] Loaded %d driver(s) from %s\n", driverDB.count, DRIVER_FILE);
    }

    /* ── Passengers ── */
    fp = fopen(PASSENGER_FILE, "rb");
    if (!fp) {
        printf("[Persist] %s not found – starting with empty passenger DB.\n",
               PASSENGER_FILE);
    } else {
        fread(&count, sizeof(int), 1, fp);
        for (i = 0; i < count; i++) {
            Passenger *p = (Passenger *)malloc(sizeof(Passenger));
            if (!p) { perror("loadAll: passenger malloc"); break; }
            if (fread(p, sizeof(Passenger), 1, fp) != 1) { free(p); break; }
            avl_insert(&passengerDB, p->p_ID, p);
        }
        fclose(fp);
        printf("[Persist] Loaded %d passenger(s) from %s\n",
               passengerDB.count, PASSENGER_FILE);
    }

    /* ── Bookings ── */
    fp = fopen(BOOKING_FILE, "rb");
    if (!fp) {
        printf("[Persist] %s not found – starting with empty booking DB.\n",
               BOOKING_FILE);
    } else {
        fread(&count, sizeof(int), 1, fp);
        for (i = 0; i < count; i++) {
            Booking *b = (Booking *)malloc(sizeof(Booking));
            if (!b) { perror("loadAll: booking malloc"); break; }
            if (fread(b, sizeof(Booking), 1, fp) != 1) { free(b); break; }
            avl_insert(&bookingDB, b->booking_id, b);
        }
        /* Restore counter */
        if (fread(&nextBookingId, sizeof(int), 1, fp) != 1) {
            /* Counter missing: derive from max booking_id + 1 */
            nextBookingId = bookingDB.count + 1;
        }
        fclose(fp);
        printf("[Persist] Loaded %d booking(s) from %s  (nextID=%d)\n",
               bookingDB.count, BOOKING_FILE, nextBookingId);
    }
}
