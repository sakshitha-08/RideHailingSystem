/*
 * driver.h
 * --------
 * Public interface for the Driver database and all driver-related operations.
 * The database is an AVL tree (avl.h) keyed on d_ID.
 *
 * updateDriverLocation policy
 * ──────────────────────────────────────────────────────────
 * Location updates ARE permitted even while a driver is booked (status = 1).
 * Rationale: a real-world cab driver continuously moves while en-route to
 * a pickup or during a ride.  Blocking updates while booked would make
 * live-tracking impossible.  This policy is intentional and documented here.
 */

#ifndef DRIVER_H
#define DRIVER_H

#include "avl.h"
#include "models.h"

/* ── Global driver AVL tree ──────────────────────────────── */
extern AVLTree driverDB;

/* ── Initialise / tear-down ─────────────────────────────── */
void driver_db_init(void);
void driver_db_free(void);

/* ── Core CRUD ───────────────────────────────────────────── */

/*
 * addDriver – insert a new driver.
 * Initialises status = DRIVER_FREE, total_earnings = 0.
 * Returns 1 on success, 0 if d_ID already exists.
 */
int addDriver(int id, const char *name, int type, int x, int y);

/*
 * deleteDriver – remove a driver.
 * Only allowed when driver status == DRIVER_FREE.
 * Prints an error and returns 0 if the driver is booked or does not exist.
 * Returns 1 on success.
 */
int deleteDriver(int d_id);

/*
 * updateDriverLocation – change a driver's coordinates.
 * Allowed even while the driver is booked (see policy above).
 * Returns 1 on success, 0 if driver not found.
 */
int updateDriverLocation(int d_id, int new_x, int new_y);

/* ── Lookup / display ────────────────────────────────────── */

/*
 * findNearestVehicle – locate the closest FREE driver within 5 km.
 * prefType: VEHICLE_CAB, VEHICLE_BIKE, or VEHICLE_ANY.
 * Returns pointer to the Driver struct, or NULL if none found.
 */
Driver *findNearestVehicle(int p_x, int p_y, int prefType);

/* Print all drivers with status == DRIVER_FREE. */
void displayAvailableVehicles(void);

/*
 * displayTopDrivers – print the top 3 drivers by total_earnings.
 * NOTE: earnings are read from the stored total_earnings field which is
 * updated on every completeRide().  The ranking is recomputed fresh each call.
 */
void displayTopDrivers(void);

/* ── Helper used by booking.c ───────────────────────────── */
Driver *getDriverById(int d_id);

#endif /* DRIVER_H */
