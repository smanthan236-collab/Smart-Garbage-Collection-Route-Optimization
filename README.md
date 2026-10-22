# Smart Garbage Collection & Route Optimization

A console application that decides **which bins to empty first, which truck takes them, and which road route each truck follows**. The algorithms are written in plain C; the application around them (objects, database, menu) is written in C++17 with SQLite for storage.

> **Phase:** Phase 2 - core algorithms + C++ application layer (console menu, SQLite, CSV seed data).

---

## Problem

Fixed collection rounds visit half-empty bins while nearly full bins wait, and trucks of different capacities can be overloaded or underused. This project uses each bin's fill level to:

1. Rank bins by urgency.
2. Assign bins to trucks without exceeding capacity.
3. Build a short road route for every truck, starting and ending at the depot.

**Success measures (to be evaluated, not yet measured):** km per cycle versus a fixed-order baseline, share of bins >= 90% served first, truck utilisation, number of bins left unassigned.

---

## How it works

```
CSV seed data -> SQLite -> C++ objects (Bin, Truck, Route)
                                |
                         CollectionManager  <-- bridge -->  C algorithms (dsa/)
```

| Step | Algorithm | File |
|---|---|---|
| Score a bin by fill level (tiers: >=90, >=75, >=50, >=25, rest) | Tiered priority function | `dsa/priority_queue.c` |
| Order bins, fullest first | Max-heap priority queue, O(log n) push/pop | `dsa/priority_queue.c` |
| Give each bin to a truck | Greedy best-fit (truck with least space left after loading) | `dsa/assignment.c` |
| Model the road network | Undirected weighted graph (adjacency list) | `dsa/graph.c` |
| Shortest road distance between two nodes | Dijkstra, O(V^2) | `dsa/graph.c` |
| Visit order for one truck | Nearest-neighbor heuristic using Dijkstra for each leg | `dsa/nearest_neighbor.c` |
| Find a bin by ID | Binary search (bins loaded sorted by ID) | `dsa/searching.c` |

Merge sort (`dsa/sorting.c`), linear search and `prioritize_bins` are implemented but not currently called by the C++ application.

---

## Project structure

```
.
|-- dsa/                    Plain C algorithms (no C++ types)
|   |-- dsa_types.h         Shared structs (CBin, CTruck, CRouteResult ...)
|   |-- priority_queue.*    Priority scoring + max-heap
|   |-- graph.*             Graph + Dijkstra
|   |-- assignment.*        Best-fit truck assignment
|   |-- nearest_neighbor.c  Route builder (routing.h)
|   |-- searching.*         Linear / binary search
|   `-- sorting.*           Merge sort
`-- cpp/                    C++17 application
    |-- main.cpp            Console menu
    |-- CollectionManager.* Bridge between C++ objects and the C algorithms
    |-- DatabaseManager.*   SQLite persistence (prepared statements)
    |-- CSVManager.*        CSV import
    |-- PriorityManager.*   RAII wrapper around the C heap
    |-- Bin / Truck / Route Domain classes
    |-- SimulationManager.* One-call simulation cycle (not wired into the menu yet)
    |-- build.ps1           Windows build script
    `-- data/               bins.csv, trucks.csv, roads.csv (seed data)
```

A line-by-line explanation of every file is in [`CODE_WALKTHROUGH.md`](CODE_WALKTHROUGH.md).

---

## Requirements

- Windows with [MSYS2](https://www.msys2.org/) (UCRT64 toolchain): `g++`, `gcc`, and the SQLite library
- PowerShell

```
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sqlite3
```

`build.ps1` looks for the compiler in `C:\msys64\ucrt64\bin\ucrt64\bin` and then `C:\msys64\ucrt64\bin`. If your MSYS2 is installed elsewhere, edit the `$candidates` list at the top of the script.

---

## Build and run

Run everything from the `cpp` folder, because the program reads `data/` and `database/` relative to it.

```powershell
cd cpp
powershell -ExecutionPolicy Bypass -File .\build.ps1
.\build\garbage.exe
```

If the program exits immediately with no menu (exit code `-1073741515`), Windows cannot find the MSYS2 DLLs. Either add your MSYS2 `bin` folder to the Windows `PATH`, or run once per terminal:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin\ucrt64\bin;C:\msys64\ucrt64\bin;" + $env:PATH
```

In VS Code, open the **`cpp`** folder (not its parent) to use the included build task (`Ctrl+Shift+B`) and debug configuration (F5).

On first start the program creates `database/garbage.db` and imports the CSV files.

---

## Menu

| Option | Action |
|---|---|
| 1 | Display bins |
| 2 | Display trucks |
| 3 | Show bin priority order |
| 4 | Assign bins to trucks |
| 5 | Generate optimized routes |
| 6 | Update a bin's fill level |
| 7 | Collect currently assigned bins |
| 8 | Reload data from SQLite |
| 9 | Re-import CSV seed data |
| 10 | Randomize bin fill levels |
| 11 | Exit |

Typical order: `1` -> `4` -> `5` -> `7`. Options 6, 8 and 10 reload data and **clear assignments**, so run `4` again afterwards. Use `9` to restore the original seed data.

---

## Sample data and expected result

Seed data: 6 bins (nodes 1-6), 2 trucks (400 kg and 300 kg), 10 undirected roads, depot = node 0.

Calculated by hand from the seed data, so compare with your own run:

| Truck | Bins | Load | Route | Distance |
|---|---|---|---|---|
| 2 | 3, 1, 2 | 280 kg | 0 -> 1 -> 2 -> 4 -> 3 -> 1 -> 0 | 6.7 km |
| 1 | 4, 6, 5 | 202 kg | 0 -> 2 -> 4 -> 5 -> 6 -> 4 -> 2 -> 0 | 9.8 km |

Bin priority order: 3, 1, 4, 6, 2, 5.

---

## Database

SQLite file `database/garbage.db` with five tables: `bins`, `trucks`, `roads`, `routes` (generated route history) and `collections` (collection log).

---

## Known limitations

- Best-fit and nearest-neighbor are **heuristics**; results are not guaranteed optimal. They have not yet been compared against brute force or 2-opt.
- Dijkstra is O(V^2) and is recomputed for every candidate bin at every stop.
- The depot is hard-coded to node 0, and routes are limited to 1024 nodes.
- Each truck makes one trip; a bin that fits no truck is left unassigned without a detailed report.
- Truck assignments are not saved to the database, so reloading clears them.
- CSV parsing does not support quoted fields or commas inside values.
- No automated tests yet.

---

## Roadmap ideas

- Baselines: fixed-order routing, first-fit assignment, brute force for small inputs, 2-opt improvement.
- Min-heap Dijkstra and one shortest-path run per stop.
- Unit tests for the heap, Dijkstra and assignment edge cases.
- Wire `SimulationManager` into the menu and log metrics (distance, utilisation).
- Multiple trips per truck and a configurable depot.

---

## Author

Manthan Singh - Graphic Era

_Add team members, guide/faculty name and license here._
