# PC Component Management System

A terminal-based C program for managing PC components in memory and synchronizing them with a CSV file.

The project is Linux-first, but the code is kept portable enough to build on the Windows NT family of operating systems with a C11 compiler such as MinGW-w64.

## Purpose

This program is designed to manage a small PC component inventory:

- load component records from a `.csv` file into RAM;
- show all loaded components;
- search components by name with linear search;
- add, update, and delete records;
- track low-stock components;
- sort records with merge sort;
- save the current in-memory records back to CSV.

The rewrite focuses on **logic correctness**, not only memory safety. Tools such as Valgrind are still useful, but they do not detect business-logic errors such as duplicate IDs, wrong sort direction, skipped CSV rows, or unsafe auto-save behavior.

## Current source layout

```text
.
├── data/
│   └── pc_component_csv.csv
├── docs/
│   ├── diagrams/
│   └── requirements/
├── include/
│   ├── pc_component_data.h
│   ├── pc_component_function.h
│   ├── pc_component_ui.h
│   ├── pc_file.h
│   ├── pc_platform.h
│   ├── pc_search.h
│   ├── pc_sort.h
│   └── pc_validation.h
├── src/
│   ├── component/
│   │   ├── pc_component.c
│   │   ├── pc_file.c
│   │   ├── pc_search.c
│   │   └── pc_sort.c
│   ├── invoice/
│   │   └── README.md
│   ├── ui/
│   │   ├── pc_component_ui.c
│   │   └── pc_platform.c
│   ├── validation/
│   │   └── pc_validation.c
│   └── main.c
├── Makefile
└── README.md
```

### Module responsibilities

| Module | Responsibility |
| --- | --- |
| `src/main.c` | Application entry point, menu dispatch, load/save lifecycle, dirty-state handling. |
| `src/component/pc_component.c` | Core component manager logic: add, update, delete, ID allocation, and sort wrappers. |
| `src/component/pc_file.c` | CSV load/save. Loading is transactional, and saving uses a temporary file before replacing the target. |
| `src/component/pc_search.c` | Portable case-insensitive linear search by component name. |
| `src/component/pc_sort.c` | Stable merge sort using a normal three-way comparator contract. |
| `src/ui/pc_component_ui.c` | All terminal input/output for component operations. Uses `fgets`, not mixed `scanf`/`fgets`. |
| `src/ui/pc_platform.c` | Linux/POSIX and Windows console helpers. Avoids `system("clear")` and `system("cls")`. |
| `src/validation/pc_validation.c` | Shared validation and parsing helpers for UI and CSV code. |
| `src/invoice/` | Reserved for a future invoice module. It is intentionally not linked yet. |

## Important logic changes in this version

1. **CSV parsing no longer uses `sscanf`**
   - Empty fields are handled.
   - Quoted CSV fields are handled, including commas inside a field.
   - Escaped double quotes are handled as `""`.
   - UTF-8 BOM and Windows CRLF endings are tolerated.
   - Very long lines are skipped instead of partially parsed.

2. **CSV loading is transactional**
   - A fatal load error does not destroy the current in-memory manager.
   - The manager is replaced only after usable data is parsed successfully.

3. **IDs are kept unique**
   - `next_id` is recalculated after file load.
   - Duplicate or zero IDs found during load are reassigned to the next free ID.
   - New records use a monotonic unique ID allocator.

4. **Sorting uses correct comparator semantics**
   - Merge sort now expects comparators to return negative / zero / positive, like standard C comparator style.
   - This fixes the previous problem where “stock descending” could sort in the wrong direction.

5. **No variable-length arrays in merge sort**
   - The sort module uses one heap-allocated temporary buffer.
   - This is more portable and avoids large stack usage.

6. **Input handling is consistent**
   - The UI uses `fgets` and explicit unsigned-integer parsing.
   - Negative values and trailing garbage are rejected.
   - This avoids the common bug where `scanf` leaves a newline that breaks the next `fgets` call.

7. **Safer save behavior**
   - Option `9` saves explicitly.
   - Option `0` exits and asks whether to save if there are unsaved changes.
   - This prevents a partially repaired load from silently overwriting the original CSV.

## CSV format

Default file:

```text
data/pc_component_csv.csv
```

Schema:

```csv
sep=,
id,category,name,socket,ddr,warrantyPeriod,stock
1,CPU,intel_i7,l200,0,6,28
```

Fields:

| Field | Meaning |
| --- | --- |
| `id` | Unsigned 32-bit record ID. IDs must be unique after load. |
| `category` | One of `CPU`, `MAINBOARD`, `PSU`, `RAM`, `SSD`, `GPU`, `CASE`. |
| `name` | Component name, shorter than `COMPONENT_NAME_LEN`. |
| `socket` | Socket/interface text, shorter than `COMPONENT_SOCKET_LEN`. Empty CSV sockets become `N/A`. |
| `ddr` | DDR generation, `0` when not applicable. Current maximum: `7`. |
| `warrantyPeriod` | Warranty period in months. Current maximum: `1200`. |
| `stock` | Stock quantity. Current maximum: `1000000`. |

CSV fields containing commas or quotes should be quoted:

```csv
29,SSD,"Kingston NV2 1TB, M.2 NVMe",N/A,0,36,15
30,GPU,"ASUS ""TUF"" RTX 4090",N/A,0,36,3
```

Current limitation: quoted fields with embedded newlines are not supported because the loader reads one record per physical line.

## Build and run on Linux / POSIX

The preferred target is Linux.

### Requirements

- GCC or Clang
- GNU Make
- A POSIX-like shell

On Debian/Ubuntu-based systems:

```sh
sudo apt update
sudo apt install build-essential
```

### Build

```sh
cd pc-component-manager
make clean
make
```

The executable is created as:

```text
./pc_app
```

### Run with the default CSV file

```sh
./pc_app
```

This uses:

```text
data/pc_component_csv.csv
```

### Run with a custom CSV file

```sh
./pc_app path/to/your_file.csv
```

### Run the smoke test

```sh
make check
```

The smoke test copies the sample CSV into `/tmp`, runs a scripted menu flow, and writes output to:

```text
/tmp/pc_component_manager_smoke.log
```

It does not modify `data/pc_component_csv.csv`.

### Clean build files

```sh
make clean
```

## Build and run on Windows NT family

Recommended environment: **MSYS2 MinGW-w64**.

From an MSYS2 MinGW terminal:

```sh
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-gcc
cd pc-component-manager
mingw32-make TARGET=pc_app.exe
./pc_app.exe data/pc_component_csv.csv
```

If your shell uses Windows path syntax, run:

```bat
pc_app.exe data\pc_component_csv.csv
```

Notes for Windows:

- The project avoids POSIX-only terminal commands such as `clear`.
- The CSV loader accepts Windows CRLF line endings.
- The save path must point to an existing directory. For example, `data\pc_component_csv.csv` works if `data` already exists.

## Development notes

The code keeps a Linux-kernel-inspired style where it makes sense for a user-space C program:

- explicit `struct pc_component`, no struct typedef alias;
- tabs for indentation;
- small functions with clear ownership of responsibilities;
- no VLAs;
- no unchecked array indexing by enum values;
- no mixed `scanf`/`fgets` input flow;
- error codes returned from core/file logic instead of hard exits.

The program is still intentionally simple: it uses a fixed-size array in `struct component_manager` instead of dynamic containers or a database.

## Common commands

```sh
# Build
make

# Run with sample data
./pc_app

# Run with another CSV
./pc_app ~/inventory.csv

# Smoke test
make check

# Debug build
make debug

# Release build
make release

# Clean
make clean
```

## Recommended next tests before deployment

- Load a CSV with duplicate IDs and confirm the report shows reassigned IDs.
- Load a CSV with a quoted comma in the name field.
- Load a CSV with Windows CRLF and UTF-8 BOM.
- Add a component, save, reload, and verify `next_id` does not collide.
- Sort by stock ascending/descending and warranty descending.
- Try invalid menu input, negative numbers, huge numbers, and empty required fields.
- Run Valgrind on Linux for memory checks, but keep separate logic tests for CSV, IDs, and sorting.
