# Benchmarks

---

## Test Environment

### Hardware Configuration

| Component                | Specification                                                 |
| ------------------------ | ------------------------------------------------------------- |
| **Computer Model**       | Lenovo ThinkPad P15v Gen 3                                    |
| **CPU**                  | 12th Gen Intel Core i7-12800H (20 logical, 14 physical cores) |
| **Base Clock**           | 2.80 GHz                                                      |
| **Turbo Clock**          | 4.80 GHz                                                      |
| **L1 Data Cache**        | 48 KiB (×6 P-cores) + 32 KiB (×8 E-cores)                     |
| **L1 Instruction Cache** | 32 KiB (×6 P-cores) + 64 KiB (×2 E-core clusters)             |
| **L2 Unified Cache**     | 1.25 MiB (×6 P-cores) + 2 MiB (×2 E-core clusters)            |
| **L3 Unified Cache**     | 24 MiB (×1 shared)                                            |
| **RAM**                  | DDR4-3200 (32GB)                                              |
| **GPU**                  | NVIDIA RTX A2000 4GB GDDR6                                    |

### Software Configuration

| Platform    | OS         | Benchmark Framework     | C++ Compiler         |
| ----------- | ---------- | ----------------------- | -------------------- |
| **Linux**   | LMDE 7     | Google Benchmark v1.9.5 | Clang 19.1.7-x64     |
| **Windows** | Windows 10 | Google Benchmark v1.9.5 | MinGW GCC 14.2.0-x64 |
| **Windows** | Windows 10 | Google Benchmark v1.9.5 | MSVC 19.50.35721-x64 |

---

# Performance Results

## JSON Access

### Value Retrieval

| Operation                   | Linux Clang | Windows MinGW GCC | Windows MSVC |
| --------------------------- | :---------: | :---------------: | :----------: |
| **Get by Key (Top Level)**  |   4.64 ns   |      2.49 ns      |   1.59 ns    |
| **Get by Key (Nested)**     |   17.7 ns   |      6.51 ns      |   16.3 ns    |
| **Get by Pointer (Top)**    |   21.9 ns   |      21.6 ns      |   28.0 ns    |
| **Get by Pointer (Nested)** |   44.2 ns   |      48.5 ns      |   61.6 ns    |
| **Get Deep Nested (3 Lvl)** |   26.1 ns   |      13.6 ns      |   24.8 ns    |
| **Get Deep Nested (4 Lvl)** |   35.0 ns   |      18.7 ns      |   36.6 ns    |
| **Has Field (Exists)**      |   24.2 ns   |      17.4 ns      |   31.8 ns    |
| **Has Field (Not Exists)**  |   30.3 ns   |      26.4 ns      |   40.7 ns    |
| **Has Field (Nested)**      |   93.8 ns   |      98.6 ns      |   118  ns    |

## JSON Iteration

### Container Traversal

| Operation                   | Linux Clang | Windows MinGW GCC | Windows MSVC |
| --------------------------- | :---------: | :---------------: | :----------: |
| **Object Field Iteration**  |   78.1 ns   |      86.7 ns      |   62.2 ns    |
| **Array Element Iteration** |   43.7 ns   |      36.3 ns      |   61.8 ns    |
| **Nested Object Iteration** |   2757 ns   |      5484 ns      |   6111 ns    |

## JSON Modification

### Document Mutation

| Operation                  | Linux Clang | Windows MinGW GCC | Windows MSVC |
| -------------------------- | :---------: | :---------------: | :----------: |
| **Set Primitive (String)** |   60.3 ns   |      92.8 ns      |   94.2 ns    |
| **Set Primitive (Int)**    |   51.2 ns   |      82.0 ns      |   87.4 ns    |
| **Set Primitive (Bool)**   |   51.4 ns   |      86.4 ns      |   88.5 ns    |
| **Set Primitive (Double)** |   50.8 ns   |      89.9 ns      |   88.6 ns    |
| **Set Nested (2 Levels)**  |   107  ns   |      179  ns      |   189  ns    |
| **Set Nested (3 Levels)**  |   151  ns   |      270  ns      |   274  ns    |
| **Set Nested (4 Levels)**  |   197  ns   |      373  ns      |   358  ns    |
| **Add Object Field (Seq)** |   456  ns   |      636  ns      |   761  ns    |
| **Append Array Element**   |   530  ns   |      1034 ns      |   1056 ns    |
| **Append to Large Array**  |   5302 ns   |      8838 ns      |   8722 ns    |

## JSON Parsing

### Parse from String

| Operation                | Linux Clang | Windows MinGW GCC | Windows MSVC |
| ------------------------ | :---------: | :---------------: | :----------: |
| **Parse Small Object**   |   225  ns   |      373 ns       |   420  ns    |
| **Parse Medium Object**  |   1031 ns   |      1640 ns      |   2316 ns    |
| **Parse Large Object**   |   3570 ns   |      5179 ns      |   7966 ns    |
| **Parse Nested Objects** |   475  ns   |      862 ns       |   906  ns    |
| **Parse Small Array**    |   165  ns   |      211 ns       |   334  ns    |
| **Parse Large Array**    |   2681 ns   |      4201 ns      |   5036 ns    |
| **Parse Mixed Types**    |   801  ns   |      1179 ns      |   1747 ns    |

## Schema Generation

### SchemaGenerator Performance

| Operation                   | Linux Clang | Windows MinGW GCC | Windows MSVC |
| --------------------------- | :---------: | :---------------: | :----------: |
| **Generate from Small Doc** |   2.09 us   |      3.25 us      |   3.34 us    |
| **Generate from Large Doc** |   9.29 us   |      15.3 us      |   15.2 us    |
| **Generate from 10 Docs**   |   22.3 us   |      35.5 us      |   38.2 us    |
| **Generate from 100 Docs**  |   209  us   |      327 us       |   361  us    |
| **Format Inference**        |   4.63 us   |      6.85 us      |   7.24 us    |
| **Constraint Inference**    |   107  us   |      163 us       |   222  us    |

## Schema Validation

### SchemaValidator Performance

| Operation                   | Linux Clang | Windows MinGW GCC | Windows MSVC |
| --------------------------- | :---------: | :---------------: | :----------: |
| **Validate Simple Schema**  |   2.61 us   |      3.93 us      |   3.91 us    |
| **Validate Complex Schema** |   5.64 us   |      8.65 us      |   8.87 us    |
| **Validate with $ref**      |   3.14 us   |      4.71 us      |   4.85 us    |
| **Validate String Formats** |   3.79 us   |      5.85 us      |   6.08 us    |
| **Validate with Errors**    |   2.66 us   |      4.28 us      |   4.20 us    |
| **Validate Deeply Nested**  |  0.868 us   |      2.19 us      |   1.40 us    |


## Unique Items

### Array Uniqueness Check

| Operation                          | Linux Clang | Windows MinGW GCC | Windows MSVC |
| ---------------------------------- | :---------: | :---------------: | :----------: |
| **Unique Integers (10)**           |  0.848 us   |      1.23 us      |   1.27 us    |
| **Unique Integers (100)**          |   4.23 us   |      5.09 us      |   5.86 us    |
| **Unique Integers (500)**          |   25.9 us   |      22.3 us      |   25.2 us    |
| **Unique Integers (1000)**         |   65.2 us   |      45.8 us      |   51.2 us    |
| **Unique Integers (5000)**         |   121  us   |      113 us       |   122  us    |
| **Unique Strings (100)**           |   7.08 us   |      10.2 us      |   8.39 us    |
| **Unique Strings (1000)**          |   93.9 us   |      102 us       |   92.9 us    |
| **Unique Objects (100)**           |   44.8 us   |      71.7 us      |   75.0 us    |
| **Unique Objects (1000)**          |   396  us   |      596 us       |   629  us    |
| **Unique Mixed (100)**             |   2.92 us   |      5.33 us      |   3.75 us    |
| **Unique Mixed (1000)**            |   16.9 us   |      23.2 us      |   25.7 us    |
| **Unique Duplicate at End (100)**  |   4.43 us   |      5.66 us      |   6.17 us    |
| **Unique Duplicate at End (1000)** |   42.7 us   |      46.5 us      |   53.6 us    |
| **Unique Duplicate at End (5000)** |   121  us   |      117 us       |   122  us    |

---

_Updated on January 23, 2026_
