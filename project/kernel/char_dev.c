#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#define  DEVICE_NAME "cronk_scd"
#define  CLASS_NAME  "scd"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel space cron jobs");
MODULE_VERSION("0.1");

static int majorNumber;   
static char   message[256] = {0};        
static short  size_of_message;              
static int    numberOpens = 0;              
static struct class*  scdClass  = NULL; 
static struct device* scdDevice = NULL; 

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

static void init_character_device(void) {
	printk(KERN_INFO "CRONK: Initializing the CRONK LKM\n");

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0) {
		printk(KERN_ALERT "CRONK failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "CRONK: registered correctly with major number %d\n", majorNumber);
	
	scdClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(scdClass)) {               
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(scdClass);          
	}
	printk(KERN_INFO "CRONK: device class registered correctly\n");
	
	scdDevice = device_create(scdClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(scdDevice)) {              
		class_destroy(scdClass);           
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(scdDevice);
	}
	printk(KERN_INFO "CRONK: device class created correctly\n"); 
}

static int __init cronk_init(void) {
	init_character_device();
	return 0;
}

static void __exit cronk_exit(void) {
	device_destroy(scdClass, MKDEV(majorNumber, 0));     
	class_unregister(scdClass);                          
	class_destroy(scdClass);                             
	unregister_chrdev(majorNumber, DEVICE_NAME);             
	printk(KERN_INFO "Closing cronk\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
	numberOpens++;
	printk(KERN_INFO "CRONK: Device has been opened %d time(s)\n", numberOpens);
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
	int error_count = 0;
	
	error_count = copy_to_user(buffer, message, size_of_message);

	if (error_count==0) {
		printk(KERN_INFO "CRONK: Sent %d characters to the user\n", size_of_message);
		return (size_of_message=0);  
	}
	else {
		printk(KERN_INFO "CRONK: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
	sprintf(message, "%s", buffer);
	size_of_message = strlen(message);
	printk(KERN_INFO "CRONK: Received %s\n", message);
	return len;
}

static int dev_release(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "CRONK: Device successfully closed\n");
	return 0;
}

module_init(cronk_init);
module_exit(cronk_exit);
