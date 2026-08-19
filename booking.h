/*
 * booking.h
 * ---------
 * Public interface for the Booking database and ride-lifecycle functions.
 * The database is an AVL tree keyed on booking_id.
 *
 * booking_id is assigned by a module-level static counter that starts at 1
 * and increments on every successful requestRide().  It is persisted to the
 * bookings.dat file so IDs remain unique across runs.
 */

#ifndef BOOKING_H
#define BOOKING_H

#include "avl.h"
#include "models.h"

/* ── Global booking AVL tree ─────────────────────────────── */
extern AVLTree bookingDB;

/* The next booking_id to assign (persisted via file I/O) */
extern int nextBookingId;

/* ── Initialise / tear-down ─────────────────────────────── */
void booking_db_init(void);
void booking_db_free(void);

/*
 * requestRide – attempt to book the nearest available vehicle.
 * Returns the new booking_id on success, or -1 on failure.
 */
int requestRide(int p_id, int p_x, int p_y, int prefType);

/*
 * completeRide – finalise a booking with actual distance travelled.
 * Calculates and stores fare, updates driver earnings + passenger frequency.
 * Returns 1 on success, 0 if booking_id not found.
 */
int completeRide(int booking_id, float distance);

/*
 * calculateDriverEarnings – sum fares by traversing the Booking DB.
 * Returns the total (does NOT use the cached Driver.total_earnings field).
 */
float calculateDriverEarnings(int d_id);

/* Print all bookings in ascending booking_id order. */
void displayBookingHistory(void);

/* Find and print the (driver, passenger) pair with the most shared rides. */
void displayFrequentPairs(void);

#endif /* BOOKING_H */
