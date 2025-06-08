#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>

static struct platform_device *pdev;

static int __init platform_dummy_char_add(void)
{
    int inst_id = 0;
    pdev = platform_device_alloc("platform-dummy-char", inst_id);
    platform_device_add(pdev);
    pr_info("platform-dummy-char device added\n");
    return 0;
}

static void __exit fplatform_dummy_char_put(void)
{
    pr_info("platform-dummy-char device removed\n");
	platform_device_put(pdev);
}

module_init(platform_dummy_char_add);
module_exit(fplatform_dummy_char_put);

MODULE_DESCRIPTION("Platform Device Instance");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kai Akihiko");