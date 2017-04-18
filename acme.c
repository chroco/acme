/*
	Chad Coates
	ECE372
	Homework #2
	April 16, 2017
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>	
#include <linux/errno.h>	
#include <asm/uaccess.h>	

#define DEVCOUNT			1
#define DEVNAME				"acme"
#define SYSCALL_VAL		40

struct acme_dev {
	struct cdev cdev;				
	int syscall_val;
} *acme_devp;

ssize_t acme_read(struct file *,char __user *buff,size_t,loff_t *);
ssize_t acme_write(struct file *,const char __user *buff,size_t,loff_t *);

static struct file_operations acme_fops = {
	.owner = THIS_MODULE,
	.read = acme_read,		
	.write = acme_write,
};

static dev_t acme_dev_number;
static struct class *acme_class;
static int syscall_val=SYSCALL_VAL;

module_param(syscall_val,int,S_IRUGO);

int __init acme_init(void){
	acme_class = class_create(THIS_MODULE,DEVNAME);
	
	if(alloc_chrdev_region(&acme_dev_number,0,DEVCOUNT,DEVNAME) < 0) {
		printk(KERN_DEBUG "Can't register device\n"); 
		return -ENODEV;
	}
	
	acme_devp = kmalloc(sizeof(struct acme_dev), GFP_KERNEL);
	if (!acme_devp) {
		printk("Bad Kmalloc\n"); 
		return -ENOMEM;
	}
	
	cdev_init(&acme_devp->cdev,&acme_fops);
	acme_devp->syscall_val=syscall_val;
	acme_devp->cdev.owner=THIS_MODULE;
	
	if(cdev_add(&acme_devp->cdev,acme_dev_number,DEVCOUNT)){
		printk(KERN_NOTICE "cdev_add() failed");
		unregister_chrdev_region(acme_dev_number,DEVCOUNT);
		return -1;
	}

	device_create(acme_class,NULL,acme_dev_number,NULL,DEVNAME);
	
	printk("ACME Driver Initialized.\n");
	return 0;
}

void __exit acme_cleanup(void){
	cdev_del(&acme_devp->cdev);
	unregister_chrdev_region(acme_dev_number,DEVCOUNT);
	kfree(acme_devp);
	device_destroy(acme_class,acme_dev_number);
	class_destroy(acme_class);
	printk("ACME Driver Uninstalled.\n");
}

ssize_t acme_read(struct file *filp,char __user *buf,size_t len,loff_t *offset){
	int ret;
	if(*offset>=sizeof(int))return 0;
	if(!buf){
		ret = -EINVAL;
		goto out;
	}
	if(copy_to_user(buf,&acme_devp->syscall_val,sizeof(int))){
		ret = -EFAULT;
		goto out;
	}
	ret=sizeof(int);
	*offset += len;
	
out:
	return ret;
}

ssize_t acme_write(struct file *filp,const char __user *buf,size_t len,loff_t *offset){
	int ret;
	if(!buf){
		ret = -EINVAL;
		goto out;
	}

	if(copy_from_user(&acme_devp->syscall_val,buf,len)){
		ret = -EFAULT;
		goto out;
	}
	ret = len;
out:
	return ret;
}

MODULE_AUTHOR("Chad Coates");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(acme_init);
module_exit(acme_cleanup);

