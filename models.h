/*
 * models.h
 * --------
 * Domain data structures for the Ride-Hailing System.
 * All three databases (Driver, Passenger, Booking) are stored in AVL trees
 * defined in avl.h / avl.c.  These structs are the *payloads* carried by
 * each AVL node.
 */

#ifndef MODELS_H
#define MODELS_H

/* ── Fare rates ─────────────────────────────────────────── */
#define FARE_CAB   10.0f   /* ₹ per km for cabs  */
#define FARE_BIKE   5.0f   /* ₹ per km for bikes */

/* ── Vehicle types ──────────────────────────────────────── */
#define VEHICLE_CAB   0
#define VEHICLE_BIKE  1
#define VEHICLE_ANY  -1

/* ── Driver status ──────────────────────────────────────── */
#define DRIVER_FREE   0
#define DRIVER_BOOKED 1

/* ── 2-D coordinate (assumed to be in km already) ────────── */
typedef struct {
    int x;
    int y;
} Location;

/* ── Driver record ──────────────────────────────────────── */
typedef struct {
    int      d_ID;           /* unique driver identifier                 */
    char     name[64];       /* driver's full name                       */
    int      vehicle_type;   /* VEHICLE_CAB or VEHICLE_BIKE              */
    Location loc;            /* current position in km                   */
    int      status;         /* DRIVER_FREE or DRIVER_BOOKED             */
    float    total_earnings; /* cumulative earnings in ₹                 */
} Driver;

/* ── Passenger record ───────────────────────────────────── */
typedef struct {
    int  p_ID;        /* unique passenger identifier  */
    char name[64];    /* passenger's full name        */
    char mobile[20];  /* mobile number (string)       */
    int  frequency;   /* total rides taken so far     */
} Passenger;

/* ── Booking record ─────────────────────────────────────── */
typedef struct {
    int   booking_id;        /* unique booking identifier (static counter) */
    int   d_ID;              /* driver involved                            */
    int   p_ID;              /* passenger involved                         */
    int   vehicle_type;      /* cab or bike                                */
    float distance_travelled;/* km, updated on completeRide()             */
    float fare;              /* ₹, calculated on completeRide()           */
    long  timestamp;         /* simulated Unix-style timestamp             */
} Booking;

#endif /* MODELS_H */
