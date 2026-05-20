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

static ssize_t vring_write(struct file *file, const char __user *user_buf,
                           size_t len, loff_t *offset) {
  if (count >= BUFFER_SIZE) {
    pr_warn("vringlog: Queue is full, dropping write request\n");
    return -ENOBUFS;
  }

  size_t bytes_to_copy = (len > SLOT_SIZE - 1) ? SLOT_SIZE - 1 : len;

  // Secure memory lane validation crossing from User Frame to Kernel Frame
  if (copy_from_user(ring_buffer[head], user_buf, bytes_to_copy)) {
    return -EFAULT;
  }

  ring_buffer[head][bytes_to_copy] = '\0';
  pr_info("vringlog: Stored log message slot [%d]: %s", head,
          ring_buffer[head]);

  head = (head + 1) % BUFFER_SIZE;
  count++;

  return len;
}

static ssize_t vring_read(struct file *file, char __user *user_buf, size_t len,
                          loff_t *offset) {
  if (*offset > 0) {
    return 0;
  }

  if (count == 0) {
    pr_info("vringlog: Queue is empty, nothing to extract\n");
    return 0;
  }

  size_t bytes_to_copy = strlen(ring_buffer[tail]);

  if (copy_to_user(user_buf, ring_buffer[tail], bytes_to_copy)) {
    return -EFAULT;
  }

  pr_info("vringlog: Extracted log message slot [%d]\n", tail);

  tail = (tail + 1) % BUFFER_SIZE;
  count--;

  *offset += bytes_to_copy;
  return bytes_to_copy;
}

static struct file_operations vring_fops = {
    .owner = THIS_MODULE,
    .open = vring_open,
    .release = vring_release,
    .read = vring_read,
    .write = vring_write,
};

static int __init vringlog_init(void) {
  pr_info("vringlog: Initializing character driver interface\n");

  if (alloc_chrdev_region(&dev_num, 0, 1, "vringlog_dev") < 0) {
    pr_err("vringlog: Major allocation failed\n");
    return -1;
  }
  pr_info("vringlog: Allocated Major %d, Minor %d\n", MAJOR(dev_num),
          MINOR(dev_num));

  cdev_init(&my_cdev, &vring_fops);
  if (cdev_add(&my_cdev, dev_num, 1) < 0) {
    pr_err("vringlog: Character device registration inside kernel database "
           "failed\n");
    unregister_chrdev_region(dev_num, 1);
    return -1;
  }

  my_class = class_create("vringlog_class");
  if (IS_ERR(my_class)) {
    pr_err("vringlog: Failed to create sysfs class pointer mapping\n");
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return PTR_ERR(my_class);
  }

  my_device = device_create(my_class, NULL, dev_num, NULL, "vringlog");
  if (IS_ERR(my_device)) {
    pr_err("vringlog: Failed to map node footprint at /dev/vringlog\n");
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return PTR_ERR(my_device);
  }

  pr_info("vringlog: Hardware footprint online at /dev/vringlog\n");
  return 0;
}

static void __exit vringlog_exit(void) {
  pr_info("vringlog: Teardown routine starting\n");

  device_destroy(my_class, dev_num);
  class_destroy(my_class);
  cdev_del(&my_cdev);
  unregister_chrdev_region(dev_num, 1);

  pr_info("vringlog: Module successfully purged from system core\n");
}

module_init(vringlog_init);
module_exit(vringlog_exit);
