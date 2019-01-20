#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#define __MOD_NAME "PROC_MODULE"
#define __MOD_DESC "Example read/write proc module"
#define __MOD_AUTH "Matthew Todd Geiger <matthewgeiger99@gmail.com>"

MODULE_AUTHOR(__MOD_AUTH);
MODULE_DESCRIPTION(__MOD_DESC);

MODULE_LICENSE("GPL");

static uint8_t u8DeviceOpen = 0;

static char szBuffer[80];
static char *szPtr = NULL;

//
// Create open function
int
__proc_open(struct inode *inode, struct file *file)
{
    // Check if PROC file is open
    if(u8DeviceOpen) {
        printk(KERN_ALERT "%s: PROC file is already open!\n", __MOD_NAME);
        return -EBUSY;
    }

    u8DeviceOpen++;

    // Lock module
    try_module_get(THIS_MODULE);

    printk(KERN_INFO "%s: PROC file opened\n", __MOD_NAME);
    return 0;
}

//
// Create close function
int
__proc_close(struct inode *inode, struct file *file)
{
    // Check if device is closed
    if(u8DeviceOpen <= 0) {
        printk(KERN_ALERT "%s: PROC file is already closed!\n", __MOD_NAME);
        return -EBUSY;
    }

    u8DeviceOpen--;

    // Unlock module
    module_put(THIS_MODULE);

    printk(KERN_INFO "%s: PROC file closed\n", __MOD_NAME);
    return 0;
}

//
// Create read function
ssize_t
__proc_read(struct file *file, char *buffer,
            size_t length, loff_t *offset)
{
    // Keep track of bytes read
    int iBytesRead = 0;

    // Check buffer
    if(szPtr == NULL || *szPtr == 0)
        return 0;

    length = strlen(szBuffer);

    // Loop through string
    while(length && *szPtr) {
        // Write byte to STDOUT of user
        put_user(*(szPtr++), buffer++);

        length--;
        iBytesRead++;
    }

    printk(KERN_INFO "%s: PROC file read\n", __MOD_NAME);
    return iBytesRead;
}

//
// Create write function
ssize_t
__proc_write(struct file *file, const char *buffer,
            size_t length, loff_t *offset)
{
    // Check if ptr is NULL and reset buffer
    memset(szBuffer, 0, sizeof(szBuffer));
	if(szPtr != NULL) {
        memset(szPtr, 0, sizeof(szPtr));
    	kfree(szPtr);
    	szPtr = NULL;
	}

    // Check length
    if(length > 80)
        length = 80;

    // Copy text string user
    if(copy_from_user(szBuffer, buffer, length)) {
        printk(KERN_ALERT "%s: Failed to get data from user!\n", __MOD_NAME);
        return -1;
    }

    if(*szBuffer == 0)
        return 0;

    // Allocate new string on the heap
    szPtr = kmalloc(strlen(szBuffer), GFP_KERNEL);
    strncpy(szPtr, szBuffer, strlen(szBuffer) + 1);
    szPtr[strlen(szBuffer)] = 0;

    printk(KERN_INFO "%s: PROC file write\n", __MOD_NAME);
    return length;
}

// Define PROC entry and file operations
static struct proc_dir_entry *proc;
static struct file_operations fopp = {
    .owner = THIS_MODULE,
    .open = __proc_open,
    .release = __proc_close,
    .read = __proc_read,
    .write = __proc_write,
};

//
// Module init function
static int
__mod_init(void)
{
    // Create PROC file
    proc = proc_create(__MOD_NAME, 0660, NULL, &fopp);
    if(proc == NULL) {
        printk(KERN_ALERT "%s: Failed to create PROC file!\n", __MOD_NAME);
        return -1;
    }

    printk(KERN_ALERT "%s: Created PROC file\n", __MOD_NAME);
    return 0;
}

//
// Module exit function
static void
__mod_exit(void)
{
    // Remove PROC file
    proc_remove(proc);
    printk(KERN_ALERT "%s: Module unloaded\n", __MOD_NAME);
}

// Establish init and exit functions
module_init(__mod_init);
module_exit(__mod_exit);