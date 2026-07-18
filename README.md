# PC Component Sales & Compatibility Validation System (PRF192 - Topic 05)

## 📌 Project Overview

This repository contains a structured, modular C application designed to manage PC component inventories and automate the compatibility validation process for custom PC assembly configurations. 

This project is built using pure standard ISO C, ensuring strict cross-platform portability. While the development environment heavily leverages Windows-based platforms (Win32 / Dev-C++), the codebase is explicitly written to target, run, and compile natively on **Linux systems**. Deep diving into standard POSIX-compliant behaviors and running execution cycles within Linux kernels allows the team to achieve deeper insight into memory boundaries, clean file handling, and terminal-independent behaviors.

---

## 🛠️ Core Technical Specifications

- **Language:** Standard ISO C (C99/C11 compliant) — ensuring absolute zero dependence on platform-specific libraries (e.g., `<windows.h>` or `<conio.h>`).
- **Memory Paradigm:** Structured records utilizing localized manager arrays with safe boundary controls to guarantee memory isolation.
- **Data Persistence:** Comma-Separated Values (CSV) database schema with automated parsing mechanisms (`fgets`, `sscanf`) for data persistence.
- **Documentation Strategy:** Shifted entirely from heavy binary formats (`.drawio`) to **Text-Based Visual Planning (ASCII Art & Markdown Diagrams)**, allowing native rendering directly on GitHub and seamless structural versioning via `git diff`.

---

## 📁 Repository Structure

Below is the production-ready directory layout of this repository:

```text
.
├── README.md                           # Main documentation & project blueprint
├── require_before_coding_(must_read).md # Onboarding engineering rules for team members
├── function_diagrams.md                # System architectural workflows (Text Graph format)
├── functionsAndVariables.md            # Mapping specification of variables and logic
├── graph.txt                           # Master ASCII scratchpad for logic flowcharts
└── pc_management_and_compatibility/    # Main Core Application Directory
    ├── pc_main.c                       # System Entrypoint & execution loop
    ├── pc_menu.c / pc_menu.h           # UI Layer & Terminal Router Interface
    ├── pc_component.c                  # Implementation of CRUD & Sorting algorithms
    ├── pc_component_data.h             # Core component types, Enums, and Structs
    ├── pc_component_function.h         # Function declarations for inventories
    ├── pc_file.c / pc_file.h           # File I/O stream handler (CSV Load/Save)
    ├── pc_invoice.c                    # Invoice calculation & order processing logic
    ├── pc_invoice_data.h               # Invoice struct blueprints
    ├── pc_invoice_function.h           # Invoice module function prototypes
    ├── components_data.csv             # Persistent component database table
    └── pc_invoice_csv.csv              # Persistent transaction log history
 ```
    
### 👥 Team Members (Group 08 - SU26)

1. Tien Khai - [https://github.com/nozomi-kobayashi]

2. Phu Hung - [GitHub Link]

3. Tuan Kiet - [GitHub Link]

4. Dinh Duc - [GitHub Link]
