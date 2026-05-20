# vringlog

vringlog is a Linux kernel module that implements a bounded FIFO (First-In-First-Out) ring buffer as a character device. It provides a simple, memory-safe interface for logging and retrieving messages between user space and kernel space.

## Overview

In systems programming, a ring buffer is a classic data structure used for efficient data streaming. vringlog brings this concept into the Linux kernel, allowing multiple processes to write logs to a fixed-size memory region and read them back in the order they arrived.

This project is designed as both a functional utility and an educational resource for those exploring Linux device drivers, kernel memory management, and character device interfaces.

## Key Features

- Bounded Memory: The buffer is limited to a fixed number of slots to prevent uncontrolled memory consumption in the kernel.
- Automatic Node Management: Upon loading, the module automatically creates /dev/vringlog with the correct major and minor numbers.
- Safe User-Kernel Transfer: Implements rigorous memory boundary checks using copy_from_user and copy_to_user.
- Overflow Protection: Gracefully handles write requests when the buffer is full by returning standard error codes.

## Project Structure

- vringlog.c: The core C source code for the kernel module.
- Makefile: Standard build instructions for the Linux kernel build system.
- test_vring.py: A Python-based producer-consumer test script to verify behavior.

## Getting Started

### Prerequisites

To build this module, you need the Linux kernel headers and standard build tools (gcc, make) installed on your system.

On Ubuntu/Debian:
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)

### Building and Loading

1. Compile the module:
   make

2. Load the module into the kernel:
   sudo insmod vringlog.ko

3. Verify the device was created:
   ls -l /dev/vringlog

4. (Optional) Adjust permissions to allow non-root access:
   sudo chmod 666 /dev/vringlog

### Testing

You can interact with the device using standard Linux commands:

- Write a log:
  echo "Hello Kernel" > /dev/vringlog

- Read a log:
  cat /dev/vringlog

Alternatively, run the provided test script to see the producer-consumer logic and overflow handling in action:
python3 test_vring.py

### Unloading

To remove the module and clean up the system:
sudo rmmod vringlog
make clean

## Architecture and Design

### The Ring Buffer

The module maintains a static two-dimensional array in kernel memory. Two pointers, 'head' and 'tail', track the next write and read positions, respectively.

- Head: Moves forward with every successful write.
- Tail: Moves forward with every successful read.
- Count: Tracks the total number of active logs to distinguish between an empty and a full buffer.

### Buffer Constraints

By default, the module is configured with:
- 4 log slots.
- 256 characters maximum per slot.

These constraints ensure that the driver remains lightweight and does not impact system stability under heavy load.

### Kernel Integration

vringlog uses the modern 'cdev' and 'class' APIs to register itself. This approach ensures compatibility with 'udev', allowing the kernel to manage the device node lifecycle automatically without requiring manual 'mknod' commands.

## Troubleshooting

- Module not loading: Check 'dmesg | tail' for error logs. Common issues include kernel version mismatches or missing headers.
- Write fails: If the buffer is full, writes will return an 'Out of buffers' error. You must read some logs to free up space.
- Permission denied: Ensure you have read/write permissions for /dev/vringlog or run your commands with 'sudo'.

## License

GPL v2
