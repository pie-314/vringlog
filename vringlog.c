#include <linux/cdev.h>    // Character device structure support
#include <linux/device.h>  // Automates udev node mapping inside /dev
#include <linux/fs.h>      // Character device registration region
#include <linux/init.h>    // Macros used for __init and __exit
#include <linux/kernel.h>  // Contains pr_info, pr_err alerts
#include <linux/module.h>  // Core header for loading LKMs
#include <linux/uaccess.h> // copy_to_user and copy_from_user safe links

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Systems Peer");
MODULE_DESCRIPTION("A Bounded FIFO Ring Buffer Character Device Driver");
MODULE_VERSION("1.0");

#define BUFFER_SIZE 4 // Bounded slots in our ring buffer
#define SLOT_SIZE 256 // Max characters per log message

// Global states tracking our ring buffer memory boundary
static char ring_buffer[BUFFER_SIZE][SLOT_SIZE];
static int head = 0;  // Where new logs are written to
static int tail = 0;  // Where old logs are read from
static int count = 0; // Number of active logs current in queue
                      //
static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static struct device
    *my_device; // Pointer for automated node population in /dev

static int vring_open(struct inode *inode, struct file *file) {
  pr_info("vringlog: Device node opened successfully\n");
  return 0;
}

static int vring_release(struct inode *inode, struct file *file) {
  pr_info("vringlog: Device node closed safely\n");
  return 0;
}
