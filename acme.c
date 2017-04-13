#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>	
#include <linux/errno.h>	
#include <asm/uaccess.h>	

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

struct acme_dev {
	struct cdev cdev;								/* The cdev structure */
	int syscall_val;
} *acme_devp;

int acme_open(struct inode *inode, struct file *filp){
	struct acme_dev *acme_devp;
	acme_devp=container_of(inode->i_cdev,struct acme_dev, cdev);
	filp->private_data=acme_devp;
	return 0;
}

ssize_t acme_read(struct file *filp,char __user *buff,size_t count,loff_t *offp){
//	char data[sizeof(int)];
//	memcpy(data,&acme_devp->syscall_val,sizeof(int));
	
//	struct acme_dev *acme_devp=filp->private_data;
//	printk("data: %s\n",data);
//	if(copy_to_user(buff,(void *)data,count))return -EFAULT;
	if(copy_to_user(buff,&acme_devp->syscall_val,count))return -EFAULT;
	return count;
}

ssize_t acme_write(struct file *filp,const char __user *buff,size_t count,loff_t *offp){
	if(copy_from_user(&acme_devp->syscall_val,buff,count))return -EFAULT;
	return count;
}

static struct file_operations acme_fops = {
	.owner = THIS_MODULE,
//	.open = acme_open,
	.read = acme_read,						/* Read method */
	.write = acme_write,					/* Write method */
};

static dev_t acme_dev_number;		/* Allotted device number */
struct class *acme_class;

#define DEVCOUNT	1
#define DEVNAME		"acme"

int __init acme_init(void){
	int err;
	
	acme_class = class_create(THIS_MODULE,DEVNAME);
	
	if(alloc_chrdev_region(&acme_dev_number,0,DEVCOUNT,DEVNAME) < 0) {
		printk(KERN_DEBUG "Can't register device\n"); 
		return -1;
	}

	acme_devp = kmalloc(sizeof(struct acme_dev), GFP_KERNEL);
	if (!acme_devp) {
		printk("Bad Kmalloc\n"); 
		return 1;
	}

	cdev_init(&acme_devp->cdev,&acme_fops);
//	devno=MKDEV(acme_dev_number,0);
	acme_devp->syscall_val=40;
	acme_devp->cdev.owner = THIS_MODULE;
	acme_devp->cdev.ops = &acme_fops; //???????
	err=cdev_add(&acme_devp->cdev,acme_dev_number,1);
	if(err){
		printk(KERN_NOTICE "error %d adding acme device",err);
		return 1;
	}

	device_create(acme_class,NULL,acme_dev_number,NULL,DEVNAME);
	
	printk("ACME Driver Initialized.\n");
	return 0;
}

void __exit acme_cleanup(void){
	cdev_del(&acme_devp->cdev);
	unregister_chrdev_region(MAJOR(acme_dev_number),DEVCOUNT);
	kfree(acme_devp);
	device_destroy(acme_class,acme_dev_number);
	class_destroy(acme_class);
}

module_init(acme_init);
module_exit(acme_cleanup);

