/*
 *	This is an example character linux module
 * 	This module does data operations in streams
 * 	This module is also does data operations in sync
 */

//
// Required headers for module, data operations, data types, and user space communication
#include <linux/stat.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

// Defining some info used for init
#define __MODULE_NAME "CHARACTER_DEVICE_MODULE"
#define __MODULE_DESCRIPTION "Example character device module"
#define __MODULE_LISCENSE "GPL"
#define __MODULE_AUTHOR "Matthew Todd Geiger <matthewgeiger99@gmail.com>"
#define __MODULE_MAJOR_NUMBER 240

// Set mod info
MODULE_AUTHOR(__MODULE_AUTHOR);
MODULE_LICENSE(__MODULE_LISCENSE);
MODULE_DESCRIPTION(__MODULE_DESCRIPTION);

// Set some globals
// Remember that a kernel stack is small!
static int iMajorNumber = 0;

static uint8_t u8DeviceOpen = 0;

static char szBuffer[80];
static char *szPtr;

 /***************************************/
// CREATE YOUR FILE OPERATION FUNCTIONS //
/***************************************/

//
// FUNCTION RAN WHEN DEVICE FILE IS OPENED
int
__mod_open(struct inode *inode, struct file *file)
{
	// Check if device file is already open
	if(u8DeviceOpen) {
		printk(KERN_ALERT "%s: Device file already opened!\n", __MODULE_NAME);
		return -EBUSY;
	}

	// Label device open
	u8DeviceOpen++;

	// Create buffer for read operation
	sprintf(szBuffer, "%s: [!] DEVICE FILE RESPONSE [!]\n", __MODULE_NAME);
	szPtr = szBuffer;

	// Lock module
	try_module_get(THIS_MODULE);

	printk(KERN_INFO "%s: Device file opened\n", __MODULE_NAME);
	return 0;
}

//
// FUNCTION TO CLOSE DEVICE FILE
int
__mod_close(struct inode *inode, struct file *file)
{
	// Check if device file is already closed
	if(u8DeviceOpen <= 0) {
		printk(KERN_ALERT "%s: Device file already closed!\n", __MODULE_NAME);
		return -EBUSY;
	}

	// Label device closed
	u8DeviceOpen--;

	// Unlock module
	module_put(THIS_MODULE);

	printk(KERN_INFO "%s: Device closed successfully\n", __MODULE_NAME);
	return 0;
}

//
// FUNCTION TO READ DEVICE FILE
ssize_t
__mod_read(struct file* file, char *buffer, size_t length, loff_t *offset)
{
	int iBytesRead = 0;

	// Check for NULL string
	if(*szPtr == 0)
		return 0;

	// Send one byte at a time
	while(length && *szPtr) {
		// Send byte and increment
		put_user(*(szPtr++), buffer++);

		// Subtract length and increment bytes read
		length--;
		iBytesRead++;
	}

	printk(KERN_INFO "%s: Device read successfully\n", __MODULE_NAME);
	return iBytesRead;
}

ssize_t
__mod_write(struct file* file, const char *buffer, size_t length, loff_t *offset)
{
	printk(KERN_ALERT "%s: WRITE OPERATION IS NOT AVAILIABLE IN THIS MODULE\n", __MODULE_NAME);
	return EINVAL;
}

// Define functions used in file operations
const struct file_operations fopp = {
	.owner = THIS_MODULE,
	.open = __mod_open,
	.release = __mod_close,
	.read = __mod_read,
	.write = __mod_write,
};

//
// MODULE INIT FUNCTION
static int
__mod_init(void)
{
	// Register character device
	iMajorNumber = register_chrdev(	__MODULE_MAJOR_NUMBER,	/* Major number you want to request */
									__MODULE_NAME,			/* Set your module name */
									&fopp);					/* Define the functions you want to use for file operations */

	// Always error check
	if(iMajorNumber < 0) {
		printk(KERN_ALERT "%s: Failed to initialize module\n", __MODULE_NAME);
		return iMajorNumber;
	}

	printk(KERN_ALERT "%s: Module successfully initialized\n", __MODULE_NAME);
	return 0;
}

//
// MODULE EXIT FUNCTION
static void
__mod_exit(void)
{
	// Unregister character device
	unregister_chrdev(__MODULE_MAJOR_NUMBER, __MODULE_NAME);

	printk(KERN_ALERT "%s: Module successfully unloaded\n", __MODULE_NAME);
}

//
// REGISTER INIT AND EXIT FUNCTIONS
module_init(__mod_init);
module_exit(__mod_exit);