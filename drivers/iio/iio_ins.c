#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/types.h>

/* Global pointer for the platform device */
static struct platform_device *pdev;

/* 
 * Initialize and add the platform device.
 * This function is called when the module is loaded.
 * It allocates and registers a platform device with the name "iio-dummy-random".
 *
 * Returns: 0 on success, negative error code on failure.
 */
static int __init fake_iio_add(void)
{
    int instance_id = 0;  /* Unique ID for the instance */
    
    /* Allocate the platform device */
    pdev = platform_device_alloc("iio-dummy-random", instance_id);
    if (!pdev) {
        pr_err("Failed to allocate platform device\n");
        return -ENOMEM;
    }

    /* Add the platform device to the system */
    int ret = platform_device_add(pdev);
    if (ret) {
        pr_err("Failed to add platform device: %d\n", ret);
        platform_device_put(pdev);
        return ret;
    }

    pr_info("iio-dummy-random added\n");
    return 0;
}

/* 
 * Clean up and remove the platform device.
 * This function is called when the module is unloaded.
 * It unregisters and releases the allocated platform device.
 */
static void __exit fake_iio_put(void)
{
    pr_info("iio-dummy-random removed\n");
    platform_device_unregister(pdev);
}

module_init(fake_iio_add);
module_exit(fake_iio_put);

MODULE_DESCRIPTION("Platform device");
MODULE_AUTHOR("Kai Akihiko");
MODULE_LICENSE("GPL");