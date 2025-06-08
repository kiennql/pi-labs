/*
 * SPI Device Driver
 * Master
 * 
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/omap-dma.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gcd.h>
#include <linux/spi/spi.h>

struct foo_master {
	struct spi_master	*master;
	void __iomem		*base;      /* Virtual base address of the controller */
	unsigned long		phys;
	struct device		*dev;
};

static int foo_spi_setup(struct spi_device *spi)
{
	printk("Foo setup invoked\n");
	return 0;
}

static void foo_spi_cleanup(struct spi_device *spi)
{
	printk("Foo clean up invoked\n");
}

static int foo_transfer_one_message(struct spi_master *master,
				     struct spi_message *m)
{
	printk("Foo transfer_one_message invoked\n");
	m->status = 0;
	spi_finalize_current_message(master);
	return 0;
}

static const struct of_device_id foo_of_match[] = {
	{ .compatible = "foo-master", },
	{ }
};
MODULE_DEVICE_TABLE(of, foo_of_match);

static int foo_spi_probe(struct platform_device *pdev)
{
	struct spi_master	*master;
	struct foo_master	*foomaster;
	const struct of_device_id *match;
	struct device_node	*node = pdev->dev.of_node;
	u32			num_cs = 1; /* default number of chipselect */
	static int		bus_num = 1;
	int			status;

	printk("Foo SPI Probe invoked \n");

	master = spi_alloc_master(&pdev->dev, sizeof(*foomaster));
	if (!master) {
		dev_dbg(&pdev->dev, "Master allocation failed\n");
		return -ENOMEM;
	}

	/* Configure SPI master properties */
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;
	master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 32);
	master->setup = foo_spi_setup;
	master->transfer_one_message = foo_transfer_one_message;
	master->cleanup = foo_spi_cleanup;
	master->dev.of_node = node;

	platform_set_drvdata(pdev, master);

	foomaster = spi_master_get_devdata(master);
	foomaster->master = master;

	match = of_match_device(foo_of_match, &pdev->dev);
	if (match) {
		of_property_read_u32(node, "num-cs", &num_cs);
		master->num_chipselect = num_cs;
		master->bus_num = bus_num++;
	} else {
		dev_err(&pdev->dev, "No matching device found for SPI master\n");
		spi_master_put(master);
		return -ENODEV;
	}

	status = spi_register_master(master);
	if (status) {
		spi_master_put(master);
		return status;
	}

	return 0;
}

static int foo_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master = platform_get_drvdata(pdev);

	spi_unregister_master(master);
	return 0;
}

static struct platform_driver foo_spi_driver = {
	.driver = {
		.name = "foo_spi",
		.owner = THIS_MODULE,
		.of_match_table = foo_of_match,
	},
	.probe = foo_spi_probe,
	.remove = foo_spi_remove,
};

module_platform_driver(foo_spi_driver);

MODULE_DESCRIPTION("SPI Master Device Driver");
MODULE_AUTHOR("Kai Akihiko");
MODULE_LICENSE("GPL");


