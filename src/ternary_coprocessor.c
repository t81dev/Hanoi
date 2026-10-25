// ternary_coprocessor.c - Linux Kernel Module for HanoiVM Virtual Ternary Co-Processor
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "ternary_coprocessor"
#define CLASS_NAME  "ternary"
#define MMIO_BASE_ADDR 0xD0000000  // Match VirtualBox DevTernary.c BAR
#define MMIO_SIZE 0x100

#define TERNARY_REG_INPUT   0x00
#define TERNARY_REG_OUTPUT  0x04
#define TERNARY_REG_COMMAND 0x08
#define TERNARY_REG_STATUS  0x0C

// IOCTL commands
#define TERNARY_IOC_MAGIC 't'
#define TERNARY_IOC_SET_CMD _IOW(TERNARY_IOC_MAGIC, 1, uint32_t)
#define TERNARY_IOC_GET_STATUS _IOR(TERNARY_IOC_MAGIC, 2, uint32_t)

// Ternary commands (match DevTernary.c)
#define CMD_TERNARY_ADD 0x1
#define CMD_TERNARY_NOT 0x2
#define CMD_TERNARY_AND 0x3
#define CMD_TERNARY_PUSH 0x4
#define CMD_TERNARY_POP 0x5

#define TRIT_COUNT 16 // 16 trits per operation
typedef int8_t TRIT;  // -1, 0, +1 for balanced ternary

static void __iomem *mmio_base;
static int major_number;
static struct class *ternary_class = NULL;
static struct device *ternary_device = NULL;
static struct dentry *debugfs_dir;
static uint32_t entropy_cache;

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    if (len < sizeof(uint32_t)) return -EINVAL;
    uint32_t result = ioread32(mmio_base + TERNARY_REG_OUTPUT);
    uint32_t status = ioread32(mmio_base + TERNARY_REG_STATUS);
    if (status != 0) {
        pr_err("TernaryCoproc: Error status 0x%x\n", status);
        return -EIO;
    }
    if (copy_to_user(buffer, &result, sizeof(result))) return -EFAULT;
    pr_info("TernaryCoproc: Read output 0x%x\n", result);
    return sizeof(result);
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    if (len < sizeof(uint32_t)) return -EINVAL;
    uint32_t input;
    if (copy_from_user(&input, buffer, sizeof(input))) return -EFAULT;
    iowrite32(input, mmio_base + TERNARY_REG_INPUT);
    pr_info("TernaryCoproc: Wrote input 0x%x\n", input);
    return sizeof(input);
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    uint32_t value;
    switch (cmd) {
        case TERNARY_IOC_SET_CMD:
            if (copy_from_user(&value, (void __user *)arg, sizeof(value))) return -EFAULT;
            if (value > CMD_TERNARY_POP) return -EINVAL; // Validate command
            iowrite32(value, mmio_base + TERNARY_REG_COMMAND);
            pr_info("TernaryCoproc: Executed command 0x%x\n", value);
            break;
        case TERNARY_IOC_GET_STATUS:
            value = ioread32(mmio_base + TERNARY_REG_STATUS);
            if (copy_to_user((void __user *)arg, &value, sizeof(value))) return -EFAULT;
            pr_info("TernaryCoproc: Status 0x%x\n", value);
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
};

static int __init ternary_init(void) {
    mmio_base = ioremap(MMIO_BASE_ADDR, MMIO_SIZE);
    if (!mmio_base) {
        pr_err("TernaryCoproc: Failed to map MMIO at 0x%lx\n", MMIO_BASE_ADDR);
        return -ENOMEM;
    }

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        pr_err("TernaryCoproc: Failed to register chrdev: %d\n", major_number);
        iounmap(mmio_base);
        return major_number;
    }

    ternary_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ternary_class)) {
        int err = PTR_ERR(ternary_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        iounmap(mmio_base);
        return err;
    }

    ternary_device = device_create(ternary_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ternary_device)) {
        int err = PTR_ERR(ternary_device);
        class_destroy(ternary_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        iounmap(mmio_base);
        return err;
    }

    debugfs_dir = debugfs_create_dir("hanoivm", NULL);
    debugfs_create_u32("entropy_cache", 0444, debugfs_dir, &entropy_cache);

    pr_info("Ternary Coprocessor module loaded. MMIO mapped at 0x%p\n", mmio_base);
    return 0;
}

static void __exit ternary_exit(void) {
    debugfs_remove_recursive(debugfs_dir);
    device_destroy(ternary_class, MKDEV(major_number, 0));
    class_destroy(ternary_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    iounmap(mmio_base);
    pr_info("Ternary Coprocessor module unloaded.\n");
}

module_init(ternary_init);
module_exit(ternary_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HanoiVM Team");
MODULE_DESCRIPTION("Linux kernel module for VirtualBox ternary coprocessor");
MODULE_VERSION("0.2");
