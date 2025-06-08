/*
 * SPI Device Driver
 * Client
 * 
 */

#include <linux/err.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>

struct foo_data {
	struct spi_device *spi;
	struct spi_message msg;
	struct spi_transfer transfer[2];

	u8 tx_buf;
	u8 rx_buf[2];
	/* Character Driver Files */
	dev_t devt;
	struct cdev cdev;
	struct class *class;
};

static ssize_t foo_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
	struct foo_data *dev = file->private_data;
	int ret;

	if (*f_pos == 0) {
		dev->tx_buf = 3;
		ret = spi_sync(dev->spi, &dev->msg);
		if (ret < 0)
			return ret;

		if (copy_to_user(buf, dev->rx_buf, sizeof(dev->rx_buf))) {
			pr_err("Failed to send to user space\n");
			return -EFAULT;
		}
		*f_pos = 1;
		return sizeof(dev->rx_buf);
	} else {
		*f_pos = 0;
		return 0;
	}
}

static int foo_open(struct inode *inode, struct file *file)
{
	struct foo_data *dev = container_of(inode->i_cdev, struct foo_data, cdev);
	file->private_data = dev;
	return 0;
}

static int foo_close(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations fops = {
	.open = foo_open,
	.release = foo_close,
	.read = foo_read,
};

static int foo_probe(struct spi_device *spi)
{
	struct foo_data *data;
	int ret;

	data = devm_kzalloc(&spi->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->spi = spi;
	data->transfer[0].tx_buf = &data->tx_buf;
	data->transfer[0].len = sizeof(data->tx_buf);
	data->transfer[1].rx_buf = data->rx_buf;
	data->transfer[1].len = sizeof(data->rx_buf);

	spi_message_init_with_transfers(&data->msg, data->transfer, ARRAY_SIZE(data->transfer));
	spi_set_drvdata(spi, data);

	ret = alloc_chrdev_region(&data->devt, 0, 1, "spi_foo");
	if (ret < 0) {
		pr_err("Device registration failed\n");
		return ret;
	}
	pr_info("Major Nr: %d\n", MAJOR(data->devt));

	data->class = class_create(THIS_MODULE, "spifoo");
	if (IS_ERR(data->class)) {
		pr_err("Class creation failed\n");
		unregister_chrdev_region(data->devt, 1);
		return PTR_ERR(data->class);
	}

	if (IS_ERR(device_create(data->class, NULL, data->devt, NULL, "spi_foo%d", 0))) {
		pr_err("Device creation failed\n");
		class_destroy(data->class);
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}

	cdev_init(&data->cdev, &fops);
	ret = cdev_add(&data->cdev, data->devt, 1);
	if (ret < 0) {
		pr_err("Device addition failed\n");
		device_destroy(data->class, data->devt);
		class_destroy(data->class);
		unregister_chrdev_region(data->devt, 1);
		return ret;
	}

	return 0;
}

static int foo_remove(struct spi_device *spi)
{
	struct foo_data *data = spi_get_drvdata(spi);

	cdev_del(&data->cdev);
	device_destroy(data->class, data->devt);
	class_destroy(data->class);
	unregister_chrdev_region(data->devt, 1);

	return 0;
}

static const struct spi_device_id foo_id[] = {
	{ "foo-spi", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, foo_id);

static struct spi_driver foo_driver = {
	.driver = {
		.name = "foo_client",
		.owner = THIS_MODULE,
	},
	.probe = foo_probe,
	.remove = foo_remove,
	.id_table = foo_id,
};
module_spi_driver(foo_driver);

MODULE_DESCRIPTION("SPI Client Device Driver");
MODULE_AUTHOR("Kai Akihiko");
MODULE_LICENSE("GPL");
