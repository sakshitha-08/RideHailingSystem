/*
 * passenger.h
 * -----------
 * Public interface for the Passenger database.
 * The database is an AVL tree keyed on p_ID.
 *
 * Duplicate detection:
 *   addPassenger() rejects both duplicate p_ID (O(log n) via tree search)
 *   and duplicate mobile_no (O(n) linear scan via inorder traversal).
 */

#ifndef PASSENGER_H
#define PASSENGER_H

#include "avl.h"
#include "models.h"

/* ── Global passenger AVL tree ───────────────────────────── */
extern AVLTree passengerDB;

/* ── Initialise / tear-down ─────────────────────────────── */
void passenger_db_init(void);
void passenger_db_free(void);

/*
 * addPassenger – insert a new passenger.
 * Rejects duplicate p_ID or duplicate mobile_no.
 * Initialises frequency = 0.
 * Returns 1 on success, 0 on duplicate / error.
 */
int addPassenger(int id, const char *name, const char *mobile);

/*
 * rangeSearchPassengers – print passengers with p_ID in [lo, hi].
 * Uses the AVL range traversal (in-order, ascending).
 */
void rangeSearchPassengers(int lo, int hi);

/* ── Helper used by booking.c ───────────────────────────── */
Passenger *getPassengerById(int p_id);

#endif /* PASSENGER_H */
