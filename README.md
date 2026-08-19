# Ride-Hailing System for Cabs and Bikes

A command-line ride-hailing application written in C, submitted as a data-structures
college assignment.  All three databases (Driver, Passenger, Booking) are implemented
as **AVL trees** for O(log n) search, insertion, and deletion.  Data persists between
runs via binary file I/O.

---

## Project Structure

```
RideHailingSystem/
├── avl.h / avl.c          Generic AVL-tree implementation
├── models.h               Domain structs (Driver, Passenger, Booking, Location)
├── driver.h / driver.c    Driver database + operations
├── passenger.h / passenger.c  Passenger database + operations
├── booking.h / booking.c  Booking database + ride lifecycle
├── persistence.h / persistence.c  Binary file save/load
├── main.c                 Menu-driven CLI entry point
├── Makefile               Build rules
└── README.md              This file
```

---

## How to Compile

### Linux / macOS / WSL / MSYS2 (MinGW)

```bash
gcc -Wall -Wextra -std=c11 -g \
    main.c avl.c driver.c passenger.c booking.c persistence.c \
    -o ride_system -lm
```

Or simply:

```bash
make
```

### Windows (Command Prompt with MinGW in PATH)

```cmd
gcc -Wall -Wextra -std=c11 -g main.c avl.c driver.c passenger.c booking.c persistence.c -o ride_system.exe -lm
```

---

## How to Run

```bash
./ride_system          # Linux/macOS/WSL
ride_system.exe        # Windows CMD
```

The program loads any previously saved `.dat` files from the current directory,
presents the interactive menu, and saves data back to disk on exit.

---

## Design Decisions

### 1. Tree Choice – AVL Tree

An **AVL tree** was chosen over a B-tree or B+ tree for the following reasons:

| Criterion | AVL Tree | B-tree / B+ tree |
|-----------|----------|-----------------|
| Implementation complexity | Moderate (4 rotation cases) | High (variable-degree splits/merges) |
| Best use case | In-memory data | Disk-paged data |
| Search / Insert / Delete | O(log n) worst-case | O(log n) worst-case |
| Range query | In-order traversal ✔ | Leaf-level scan ✔ |
| Pedagogical clarity | High ✔ | Medium |

For an in-memory, RAM-resident dataset (typical for a college project), the AVL
tree gives optimal worst-case guarantees with a simpler, more auditable
implementation than a B/B+ tree.

The **balance invariant** is: for every node N,
```
|height(N.left) − height(N.right)| ≤ 1
```
Violations are fixed by one of four local rotations after each insert or delete:

- **LL** – single right rotation
- **RR** – single left rotation
- **LR** – left rotation on left child, then right rotation on parent
- **RL** – right rotation on right child, then left rotation on parent

### 2. File Persistence

On startup `loadAll()` reads three binary files:

| File | Contents |
|------|----------|
| `drivers.dat` | `int` count + array of `Driver` structs |
| `passengers.dat` | `int` count + array of `Passenger` structs |
| `bookings.dat` | `int` count + array of `Booking` structs + `int nextBookingId` |

Records are written in **in-order (ascending key) sequence** using `fwrite()`.
On load, `fread()` restores each struct and re-inserts it into a fresh AVL tree,
which rebalances automatically.

> **Warning:** The binary format is not portable across architectures with different
> struct padding or endianness.  Files are meant to be read by the same binary on
> the same machine.

### 3. `updateDriverLocation` while Booked Policy

**Location updates ARE permitted even while a driver is booked (`status = 1`).**

*Rationale:* A real-world cab driver continuously moves while en-route to a pickup
or during an active ride.  Blocking location updates while booked would make
live-tracking impossible, breaking the core use-case of showing a driver's current
position on a map.  A warning note is printed in the output if the driver is
currently booked so operators remain aware.

### 4. `calculateDriverEarnings` – Traversal vs. Cached Field

The function traverses the **entire Booking AVL tree** (in-order), summing fares
for the requested driver ID.  This demonstrates the traversal skill required by
the assignment; the cached `Driver.total_earnings` field is updated separately on
`completeRide()` for fast O(1) lookups (used by `displayTopDrivers`).

### 5. `findNearestVehicle` – Search Strategy

A full in-order AVL traversal (O(n)) is used because nearest-neighbour search on a
1-D key-based tree cannot be accelerated by the key ordering.  A 2-D spatial index
(k-d tree) would reduce this to O(√n + k) but is outside the scope of this
assignment.  The 5 km Euclidean threshold is applied as an early filter inside the
visitor callback.

---

## Menu Reference

| Option | Function |
|--------|----------|
| 1 | Add a driver |
| 2 | Add a passenger |
| 3 | Request a ride |
| 4 | Complete a ride |
| 5 | Display top 3 drivers by earnings |
| 6 | Display most frequent (driver, passenger) pair |
| 7 | Display all available (free) vehicles |
| 8 | Update a driver's location |
| 9 | Delete a driver (only if free) |
| 10 | Display full booking history |
| 11 | Range-search passengers by ID |
| 12 | Calculate driver earnings via Booking DB traversal |
| 0 | Save data and exit |

---

## Sample Session

```
Ride-Hailing System  –  Loading saved data...
[Persist] drivers.dat not found – starting with empty driver DB.
...

╔══════════════════════════════════════════════╗
║       🚖  Ride-Hailing System  🚲            ║
╠══════════════════════════════════════════════╣
║  1.  Add a driver                            ║
...

Enter choice: 1
─── Add Driver ───────────────────────────────────────
  Driver ID  : 101
  Name       : Ravi Kumar
  Vehicle type (0=Cab, 1=Bike): 0
  Location X : 3
  Location Y : 4
[Driver] Added: ID=101  Name=Ravi Kumar  Type=Cab  Location=(3,4)
```

---

## Fare Rates

| Vehicle | Rate |
|---------|------|
| Cab  | ₹10 / km |
| Bike | ₹5 / km |

---

*Assignment submitted for Data Structures course.*
