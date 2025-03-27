#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>


#undef pr_fmt
#define pr_fmt(fmt) "%s: " fmt, __func__

#define NO_OF_DEVICES       4

/* Device private data structure */
struct gpiodev_private_data {
    char label[20];
    struct gpio_desc *desc;
};

/* Driver private data stucture */
struct gpiodrv_private_data {
    int total_devices;
    struct class *class_gpio;
};
struct gpiodrv_private_data gpiodrv_data = {
    .total_devices = NO_OF_DEVICES,
};

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
    return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    return count;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    return count;
}

int pcd_open(struct inode *inode, struct file *filp)
{
    int ret;

    return ret;
}

int pcd_release(struct inode *inode, struct file *filp)
{
    pr_info("Release was successful\n");
    return 0;
}

/* file operations of driver */
struct file_operations pcd_fops = {
    .open = pcd_open,
    .release = pcd_release,
    .read = pcd_read,
    .write = pcd_write,
    .owner = THIS_MODULE,
};

int gpio_sysfs_probe(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;
    struct gpiodev_private_data *dev_data;
    struct device_node *parent = dev->of_node;
    struct device_node *child = NULL;
    const char* labelname;

    for_each_available_child_of_node(parent, child) {
        int i = 0;
        pr_info("Child name %s\n", child->name);

        dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
        if (!dev_data) {
            pr_info("Cannot allocate memory\n");
            return -ENOMEM;
        }

        // // Store child reference
        // dev_data->node = child;

        if (of_property_read_string(child, "label", &labelname)) {
            pr_info("Missing label\n");
            return -EINVAL;
        } else {
            strcpy(dev_data->label, labelname);
            pr_info("Label %s\n", dev_data->label);
        }

        dev_data->desc = devm_fwnode_gpiod_get_index(dev, of_fwnode_handle(child), NULL, i,\ 
                                    GPIOD_OUT_LOW, NULL);
        if (IS_ERR(dev_data->desc)) {
            pr_info("GPIOD get failed\n");
            return PTR_ERR(dev_data->desc);
        }

        /* set the gpio direction to output */
        ret = gpiod_direction_output(dev_data->desc, 0);
        if (ret) {
            pr_info("GPIOD direction set failed\n");
            return ret;
        }

        for (uint32_t i = 0; i < 10; i++) {
            gpiod_set_value(dev_data->desc, 1);
            msleep(500);
            gpiod_set_value(dev_data->desc, 0);
            msleep(500);
        }

        i++;
    }

    return 0;
}

int gpio_sysfs_remove(struct platform_device *pdev)
{
    // struct pcdev_private_data *pcdev_data = dev_get_drvdata(&pdev->dev);

    // /* 1. Remove a device that was created with device create() */
    // device_destroy(pcdrv_data.class_pcd, pcdev_data->dev_num);

    // /* 2. Remove a cdev entry from the system */
    // cdev_del(&pcdev_data->cdev);

    // pcdrv_data.total_devices--;

    // dev_info(&pdev->dev, "A device is removed\n");
    return 0;
}

struct of_device_id gpio_device_match[] = {
    { .compatible = "org,bone-gpio-sysfs" },
    { },
};

struct platform_driver gpiosysfs_platform_driver = {
    .probe = gpio_sysfs_probe,
    .remove = gpio_sysfs_remove,
    // .id_table = pcdev_ids,
    .driver = {
        .name = "bone-gpio-sysfs",
        .of_match_table = of_match_ptr(gpio_device_match)
    }
};

static int __init pcd_driver_init(void)
{
    int ret;
    /* 1. Dynamically allocate a device number for MAX_DEVICES */
    ret = alloc_chrdev_region(&gpiodrv_data.total_devices, 0, NO_OF_DEVICES, "bone_gpio-dev");
    if (ret < 0) {
        pr_err("Alloc chrdev failed\n");
        return ret;
    }

    /* 2. Create a device class under /sys/class */
    gpiodrv_data.class_gpio = class_create("bone_gpio");
    if (IS_ERR(gpiodrv_data.class_gpio)) {
        pr_err("Class creation failed\n");
        ret = PTR_ERR(gpiodrv_data.class_gpio);
        unregister_chrdev_region(gpiodrv_data.total_devices, NO_OF_DEVICES);
        return ret;
    }
    
    /* 3. Register a platform driver */
    platform_driver_register(&gpiosysfs_platform_driver);
    pr_info("Pcd Platform Driver Loaded\n");

    return 0;
}

static void __exit pcd_driver_cleanup(void)
{
    /* 1. Unregister platform driver */
    platform_driver_unregister(&gpiosysfs_platform_driver);

    /* 2. Class destroy */
    class_destroy(gpiodrv_data.class_gpio);

    /* 3. Unregister device number */
    unregister_chrdev_region(gpiodrv_data.total_devices, NO_OF_DEVICES);

    pr_info("Pcd Platform Driver Unoaded\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huy Nguyen");
MODULE_DESCRIPTION("A pseudo character driver which handles multiple devices");
