/*
 * main.c
 * ------
 * Menu-driven CLI entry point for the Ride-Hailing System.
 *
 * Startup sequence:
 *   1. Initialise all three AVL trees (driver, passenger, booking).
 *   2. Load persisted data from disk (if any).
 *   3. Present the interactive menu.
 *   4. On exit: save all data to disk, free all memory.
 *
 * Input handling notes:
 *   - scanf() is used for numeric fields; fgets() for strings.
 *   - After every scanf() call the input buffer is flushed so that
 *     subsequent fgets() calls work correctly.
 *   - All inputs are validated before being passed to domain functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "models.h"
#include "avl.h"
#include "driver.h"
#include "passenger.h"
#include "booking.h"
#include "persistence.h"

/* ── Utility: flush stdin after scanf ───────────────────── */
static void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ── Utility: read a trimmed string from stdin ──────────── */
static void read_string(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';   /* strip trailing newline */
    }
}

/* ── Utility: read a validated integer ─────────────────── */
static int read_int(const char *prompt) {
    int val;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &val) == 1) { flush_stdin(); return val; }
        printf("  [!] Invalid input – please enter an integer.\n");
        flush_stdin();
    }
}

/* ── Utility: read a validated float ───────────────────── */
static float read_float(const char *prompt) {
    float val;
    while (1) {
        printf("%s", prompt);
        if (scanf("%f", &val) == 1) { flush_stdin(); return val; }
        printf("  [!] Invalid input – please enter a number.\n");
        flush_stdin();
    }
}

/* ════════════════════════════════════════════════════════════
 * Menu handler functions
 * ════════════════════════════════════════════════════════════ */

/* ── 1. Add a driver ──────────────────────────────────────── */
static void menu_add_driver(void) {
    puts("\n─── Add Driver ───────────────────────────────────────");
    int  id   = read_int("  Driver ID  : ");
    char name[64];
    /* read_int() already flushed stdin; go straight to fgets */
    read_string("  Name       : ", name, sizeof(name));

    printf("  Vehicle type (0=Cab, 1=Bike): ");
    int type;
    if (scanf("%d", &type) != 1 || (type != 0 && type != 1)) {
        printf("  [!] Invalid vehicle type.\n");
        flush_stdin();
        return;
    }
    flush_stdin();

    int x = read_int("  Location X : ");
    int y = read_int("  Location Y : ");

    addDriver(id, name, type, x, y);
}

/* ── 2. Add a passenger ───────────────────────────────────── */
static void menu_add_passenger(void) {
    puts("\n─── Add Passenger ────────────────────────────────────");
    int  id = read_int("  Passenger ID : ");
    char name[64], mobile[20];
    /* read_int() already flushed stdin; go straight to fgets */
    read_string("  Name         : ", name,   sizeof(name));
    read_string("  Mobile no.   : ", mobile, sizeof(mobile));

    if (strlen(mobile) == 0) {
        printf("  [!] Mobile number cannot be empty.\n");
        return;
    }
    addPassenger(id, name, mobile);
}

/* ── 3. Request a ride ───────────────────────────────────── */
static void menu_request_ride(void) {
    puts("\n─── Request Ride ─────────────────────────────────────");
    int p_id = read_int("  Passenger ID       : ");
    int p_x  = read_int("  Your location X   : ");
    int p_y  = read_int("  Your location Y   : ");

    printf("  Vehicle preference (0=Cab, 1=Bike, -1=Any): ");
    int pref;
    if (scanf("%d", &pref) != 1) {
        printf("  [!] Invalid preference.\n");
        flush_stdin();
        return;
    }
    flush_stdin();

    if (pref != 0 && pref != 1 && pref != -1) {
        printf("  [!] Preference must be 0, 1, or -1.\n");
        return;
    }

    requestRide(p_id, p_x, p_y, pref);
}

/* ── 4. Complete a ride ──────────────────────────────────── */
static void menu_complete_ride(void) {
    puts("\n─── Complete Ride ────────────────────────────────────");
    int   bid  = read_int("  Booking ID        : ");
    float dist = read_float("  Distance (km)    : ");
    if (dist < 0) {
        printf("  [!] Distance must be non-negative.\n");
        return;
    }
    completeRide(bid, dist);
}

/* ── 5. Display top drivers ──────────────────────────────── */
static void menu_top_drivers(void) {
    displayTopDrivers();
}

/* ── 6. Display frequent pairs ───────────────────────────── */
static void menu_frequent_pairs(void) {
    displayFrequentPairs();
}

/* ── 7. Display available vehicles ───────────────────────── */
static void menu_available_vehicles(void) {
    displayAvailableVehicles();
}

/* ── 8. Update driver location ───────────────────────────── */
static void menu_update_location(void) {
    puts("\n─── Update Driver Location ───────────────────────────");
    int d_id  = read_int("  Driver ID  : ");
    int new_x = read_int("  New X      : ");
    int new_y = read_int("  New Y      : ");
    updateDriverLocation(d_id, new_x, new_y);
}

/* ── 9. Delete a driver ──────────────────────────────────── */
static void menu_delete_driver(void) {
    puts("\n─── Delete Driver ────────────────────────────────────");
    int d_id = read_int("  Driver ID : ");
    deleteDriver(d_id);
}

/* ── 10. Display booking history ─────────────────────────── */
static void menu_booking_history(void) {
    displayBookingHistory();
}

/* ── 11. Range-search passengers ─────────────────────────── */
static void menu_range_search(void) {
    puts("\n─── Range Search Passengers ──────────────────────────");
    int lo = read_int("  From Passenger ID : ");
    int hi = read_int("  To   Passenger ID : ");
    rangeSearchPassengers(lo, hi);
}

/* ── 12. Calculate driver earnings (bonus) ───────────────── */
static void menu_driver_earnings(void) {
    puts("\n─── Calculate Driver Earnings (from Booking DB) ──────");
    int d_id = read_int("  Driver ID : ");
    calculateDriverEarnings(d_id);
}

/* ════════════════════════════════════════════════════════════
 * Main menu loop
 * ════════════════════════════════════════════════════════════ */

static void print_menu(void) {
    puts("\n╔══════════════════════════════════════════════╗");
    puts("║       🚖  Ride-Hailing System  🚲            ║");
    puts("╠══════════════════════════════════════════════╣");
    puts("║  1.  Add a driver                            ║");
    puts("║  2.  Add a passenger                         ║");
    puts("║  3.  Request a ride                          ║");
    puts("║  4.  Complete a ride                         ║");
    puts("║  5.  Display top drivers                     ║");
    puts("║  6.  Display frequent pairs                  ║");
    puts("║  7.  Display available vehicles              ║");
    puts("║  8.  Update driver location                  ║");
    puts("║  9.  Delete a driver                         ║");
    puts("║ 10.  Display booking history                 ║");
    puts("║ 11.  Range-search passengers by ID           ║");
    puts("║ 12.  Calculate driver earnings (traversal)   ║");
    puts("║  0.  Exit                                    ║");
    puts("╚══════════════════════════════════════════════╝");
}

int main(void) {
    /* ── Initialise databases ── */
    driver_db_init();
    passenger_db_init();
    booking_db_init();

    /* ── Load persisted data ── */
    puts("Ride-Hailing System  –  Loading saved data...");
    loadAll();

    /* ── Menu loop ── */
    int choice;
    do {
        print_menu();
        choice = read_int("  Enter choice: ");

        switch (choice) {
            case  1: menu_add_driver();        break;
            case  2: menu_add_passenger();     break;
            case  3: menu_request_ride();      break;
            case  4: menu_complete_ride();     break;
            case  5: menu_top_drivers();       break;
            case  6: menu_frequent_pairs();    break;
            case  7: menu_available_vehicles();break;
            case  8: menu_update_location();   break;
            case  9: menu_delete_driver();     break;
            case 10: menu_booking_history();   break;
            case 11: menu_range_search();      break;
            case 12: menu_driver_earnings();   break;
            case  0:
                puts("\nSaving data...");
                saveAll();
                puts("Goodbye!");
                break;
            default:
                printf("  [!] Invalid choice. Please enter 0–12.\n");
        }
    } while (choice != 0);

    /* ── Free all memory ── */
    driver_db_free();
    passenger_db_free();
    booking_db_free();

    return 0;
}
