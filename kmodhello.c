#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/init.h>

static int __init hello_start(void) 
{ 
	printk(KERN_INFO "Hello world!\n"); 
	return 0; 
} 

static void __exit hello_end(void) 
{ 
	printk(KERN_INFO "Goodbye World!\n"); 
} 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joshua Stone");
MODULE_DESCRIPTION("Hello World kernel module");
MODULE_VERSION("0.1");

module_init(hello_start); 
module_exit(hello_end); 

