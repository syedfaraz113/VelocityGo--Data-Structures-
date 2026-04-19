VelocityGo — Ride Hailing App (Data Structures Edition)

VelocityGo is a console-based **ride-hailing application** built entirely in **C++** as a Data Structures & Algorithms (DSA) course project. It simulates the core functionality of platforms like inDrive or Careem, applying real-world DSA concepts such as graphs, min-heaps, and hash tables to solve routing and matching problems efficiently.

---

Table of Contents

- [Overview](#overview)
- [Tech Stack](#tech-stack)
- [Data Structures Used](#data-structures-used)
- [File Structure](#file-structure)
- [Features](#features)
- [Getting Started](#getting-started)
- [Project Report & Presentation](#project-report--presentation)
- [Author](#author)

---

Overview

This project demonstrates how core data structures power real-world applications. VelocityGo applies DSA fundamentals to:
- Model a city road network as a **graph**
- Find the shortest route between rider and driver using **shortest path algorithms**
- Match drivers to riders efficiently using a **min-heap priority queue**
- Store and look up user/driver data using **hash tables**

The project was built with **Visual Studio** on Windows and includes a full report and presentation.

---

Tech Stack

| Technology | Purpose |
|------------|---------|
| C++ (100%) | Core application logic |
| Visual Studio | IDE and build system |
| `.sln` Solution File | Project configuration |

---

Data Structures Used

| Data Structure | Application in VelocityGo |
|----------------|--------------------------|
| **Graph** | City road network — nodes are locations, edges are roads |
| **Dijkstra's Algorithm / Shortest Path** | Finding the optimal route from driver to rider |
| **Min-Heap / Priority Queue** | Driver matching — assigning the nearest available driver |
| **Hash Table / Hashing** | Fast lookup of users, drivers, and ride records |
| **Linked Lists / Arrays** | Managing ride history and driver/rider queues |

---

File Structure

```
VelocityGo--Data-Structures-/
├── Data Structures/          # Main C++ source files
│   └── *.cpp / *.h           # Application logic, DSA implementations
├── x64/Debug/                # Compiled build output (Visual Studio)
├── Data Structures.sln       # Visual Studio solution file
├── dsa_projectreport.docx    # Full project report
├── dsaprojectpresentation.pptx # Project presentation slides
└── .gitattributes
```

---

Features

- 🗺️ **City Map Simulation** — Road network modeled as a weighted graph
- 📍 **Shortest Route Calculation** — Optimal path from driver to pickup point
- 🚘 **Driver-Rider Matching** — Nearest driver assigned using a priority queue
- 👤 **User & Driver Management** — Fast record lookup via hashing
- 📋 **Ride Booking Flow** — Full console-based ride request and assignment
- 📊 **Ride History Tracking** — Logged rides per user session

---

Getting Started

### Prerequisites

- **Visual Studio 2019/2022** (Windows) with C++ Desktop Development workload
- Or any C++17-compatible compiler (GCC, Clang)

### Run with Visual Studio

1. Clone the repository:
   ```bash
   git clone https://github.com/syedfaraz113/VelocityGo--Data-Structures-.git
   ```
2. Open `Data Structures.sln` in Visual Studio
3. Set build configuration to **x64 → Debug** or **Release**
4. Press **F5** or click **Run** to build and execute

### Run with GCC (Command Line)

```bash
cd "Data Structures"
g++ -std=c++17 *.cpp -o VelocityGo
./VelocityGo
```

---

Project Report & Presentation

The repo includes full academic documentation:

`dsa_projectreport.docx`** — Detailed report covering problem statement, data structure choices, algorithm analysis, and results
`dsaprojectpresentation.pptx`** — Presentation slides summarizing the project

---

Author

**Syed Faraz Ibne Saleem**
- GitHub: [@syedfaraz113](https://github.com/syedfaraz113)
- University: Air University, Islamabad
- Course: Data Structures & Algorithms (BS Computer Science)
