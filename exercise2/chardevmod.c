#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

static int chardev_start(void);
static void chardev_end(void);
static void cleanup(void);

static int uevent(struct device *dev, struct kobj_uevent_env *env);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define CHRDEV_NAME "chrdev"    // Device name will appear in /proc/devices
#define DEVICE_NAME "chardev"   // Device will be located at /dev/chardev
#define CLASS_NAME "chardevice" // Device will be located at /sys/class/chardevice
#define SUCCESS 0
#define BUF_LEN 20              // Maximum buffer string length

static int Device_Open = 0;	// Check if device is open in case of concurrent access

static dev_t first;             // Global variable for the first device number
static struct cdev c_dev;       // Global variable for the character device structure
static struct class *cl;        // Global variable for the device class

static char msg[BUF_LEN];
static char *msg_Ptr;

static struct file_operations fops = {
	.owner   = THIS_MODULE,
	.read    = device_read,
	.write   = device_write,
	.open    = device_open,
	.release = device_release
};

static int counter = 5;

// Permission docs found at https://man7.org/linux/man-pages/man2/fchmod.2.html
#define READ_WRITE_USER S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP
#define READABLE_EVERYONE S_IRUSR|S_IRGRP|S_IROTH

module_param(counter, int, READ_WRITE_USER);
MODULE_PARM_DESC(counter, "Test param (default = 5)");

static int uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", READABLE_EVERYONE);
    return 0;
}


static void cleanup(void) {
        cdev_del(&c_dev);
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
}

static int __init chardev_start(void) 
{
	int ret;
	struct device *dev_ret;

       	printk(KERN_INFO "Initializing countdown module with starting value %d.\n", counter);
	if ((ret = alloc_chrdev_region(&first, 0, 1, CHRDEV_NAME)) < 0)
	{
		return ret;
	}
	if (IS_ERR(cl = class_create(THIS_MODULE, CLASS_NAME)))
	{
		unregister_chrdev_region(first, 1);
		return PTR_ERR(cl);
	}
	cl->dev_uevent = uevent;
	if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, DEVICE_NAME)))
	{
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return PTR_ERR(dev_ret);
	}
	cdev_init(&c_dev, &fops);
	if ((ret = cdev_add(&c_dev, first, 1)) < 0)
	{
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return ret;
	}
	return SUCCESS;
} 


static void __exit chardev_end(void)
{
	cleanup();
	printk(KERN_INFO "Done!");
}

static int device_open(struct inode *inode, struct file *file)
{
	if (Device_Open) {
		return -EBUSY;
        }
	Device_Open++;
	if (counter == 0) {
		sprintf(msg, "Blastoff!\n");
	} else {
		sprintf(msg, "%d\n", counter);
		counter--;
	}
	msg_Ptr = msg;

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;
	return SUCCESS;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
	int bytes_read = 0;

	if (*msg_Ptr == 0) {
		return 0;
	}

	while (length && *msg_Ptr) {
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}
	return bytes_read;
}

// Skip write support for now
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Sorry, device isn't writable.\n");
	return -EINVAL;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joshua Stone");
MODULE_DESCRIPTION("Countdown kernel module");
MODULE_VERSION("0.1");

module_init(chardev_start); 
module_exit(chardev_end); 

