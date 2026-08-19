/*
 * persistence.h
 * -------------
 * File I/O for persisting the three AVL databases between runs.
 *
 * Each tree is serialised as a flat array of its payload structs written
 * in in-order (ascending key) sequence.  On load, the structs are read back
 * and re-inserted into a fresh AVL tree, restoring O(log n) structure.
 *
 * Files
 * ─────
 *   drivers.dat    – array of Driver structs
 *   passengers.dat – array of Passenger structs
 *   bookings.dat   – array of Booking structs followed by nextBookingId (int)
 *
 * All files are written in the same directory as the executable (CWD).
 * File format is binary (fwrite/fread of the raw C structs), so data files
 * are NOT portable across architectures with different struct layouts.
 */

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

/* Save all three databases to disk.  Call before exit. */
void saveAll(void);

/* Load all three databases from disk.  Call at startup. */
void loadAll(void);

#endif /* PERSISTENCE_H */
