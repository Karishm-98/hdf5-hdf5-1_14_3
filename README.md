# HDF5 Project (v1.14.3)

This project demonstrates how to work with HDF5 datasets and capture low-level file I/O operations using `strace`.

---

## Steps to Run

### 1. Compile the C program
This program (`chunk_read.c`) creates an HDF5 dataset and reads chunks.

```bash
gcc chunk_read.c -o chunk_read -I/usr/include/hdf5/serial -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5

Ensure your system can find HDF5 shared libraries at runtime:

export LD_LIBRARY_PATH=$HOME/.local/lib:$LD_LIBRARY_PATH

Capture all system calls (read, write, lseek, etc.) during execution:

strace -o trace_output.txt ./chunk_read

Use the Python script to extract HDF5 chunk offsets and read operations:

python3 parse_strace_reads.py


