#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h> /* For platform devices */
#include <linux/cdev.h>
#include <linux/fs.h>

#define DEVICE_NAME "foo_char"
#define CLASS_NAME "foo_char_class"

static unsigned int major;
static struct class *foo_class;
static struct cdev foo_cdev;

/* Function Prototypes */
static int foo_open(struct inode *inode, struct file *filp);
static int foo_release(struct inode *inode, struct file *filp);
static ssize_t foo_read(struct file *filp, char __user *buf, size_t count, loff_t *offset);
static ssize_t foo_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset);
static int my_pdrv_probe(struct platform_device *pdev);
static int my_pdrv_remove(struct platform_device *pdev);

/* File operations structure */
static struct file_operations foo_fops = {
    .owner = THIS_MODULE,
    .open = foo_open,
    .release = foo_release,
    .read = foo_read,
    .write = foo_write,
};

static int foo_open(struct inode *inode, struct file *filp)
{
    pr_info("Foo device opened\n");
    return 0;
}

static int foo_release(struct inode *inode, struct file *filp)
{
    pr_info("Foo device closed\n");
    return 0;
}

static ssize_t foo_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    pr_info("Foo device read\n");
    return 0;
}

static ssize_t foo_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    pr_info("Foo device write\n");
    return count;
}

static int my_pdrv_probe(struct platform_device *pdev)
{
    int error;
    dev_t devt;
    struct device *foo_device;

    /* Allocate major number */
    error = alloc_chrdev_region(&devt, 0, 1, DEVICE_NAME);
    if (error) {
        pr_err("Failed to allocate major number\n");
        return error;
    }

    major = MAJOR(devt);
    pr_info("Allocated major number: %d\n", major);

    /* Create device class */
    foo_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(foo_class)) {
        pr_err("Failed to create class\n");
        unregister_chrdev_region(devt, 1);
        return PTR_ERR(foo_class);
    }

    /* Initialize the cdev structure and add it to kernel space */
    cdev_init(&foo_cdev, &foo_fops);
    foo_cdev.owner = THIS_MODULE;

    error = cdev_add(&foo_cdev, devt, 1);
    if (error) {
        pr_err("Failed to add cdev\n");
        class_destroy(foo_class);
        unregister_chrdev_region(devt, 1);
        return error;
    }

    /* Create device node in /dev */
    foo_device = device_create(foo_class, &pdev->dev, devt, NULL, DEVICE_NAME);
    if (IS_ERR(foo_device)) {
        pr_err("Failed to create device\n");
        cdev_del(&foo_cdev);
        class_destroy(foo_class);
        unregister_chrdev_region(devt, 1);
        return PTR_ERR(foo_device);
    }

    pr_info("Foo platform character driver loaded\n");
    return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    dev_t devt = MKDEV(major, 0);

    /* Clean up resources */
    device_destroy(foo_class, devt);
    cdev_del(&foo_cdev);
    class_destroy(foo_class);
    unregister_chrdev_region(devt, 1);

    pr_info("Foo platform character driver unloaded\n");
    return 0;
}

/* Platform driver structure */
static struct platform_driver mypdrv = {
    .probe = my_pdrv_probe,
    .remove = my_pdrv_remove,
    .driver = {
        .name = "platform-foo-char",
        .owner = THIS_MODULE,
    },
};

module_platform_driver(mypdrv);

MODULE_DESCRIPTION("Platform Device Driver");
MODULE_AUTHOR("Kai Akihiko");
MODULE_LICENSE("GPL");
