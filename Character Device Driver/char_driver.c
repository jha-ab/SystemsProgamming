#include <linux/module.h>	/* for modules */
#include <linux/fs.h>		/* file_operations */
#include <linux/uaccess.h>	/* copy_(to,from)_user */
#include <linux/init.h>		/* module_init, module_exit */
#include <linux/slab.h>		/* kmalloc */
#include <linux/cdev.h>		/* cdev utilities */

#define MYDEV_NAME "mycdrv"

#define ramdisk_size (size_t) (16*PAGE_SIZE)

static unsigned int NUM_DEVICES = 3;

typedef struct
{
	struct cdev dev;
    char *ramdisk;
    struct semaphore sem;
    int devNo;
	int size;
	int disk_size;
} asp_mycdev;

static const struct file_operations fops;

static struct class *MydevClass;
asp_mycdev *dev_pointer;
int major;




/* Support N devices */
module_param(NUM_DEVICES, int, S_IRUGO);

#define CDEV_IOC_MAGIC 'Z'
#define ASP_CLEAR_BUFF _IOW(CDEV_IOC_MAGIC, 1,int)


static int mycdrv_open(struct inode *inode, struct file *file_ptr)
{
	asp_mycdev *mydev; /* device information */
	pr_info("Opening device %s \n",MYDEV_NAME); 
	mydev = container_of(inode->i_cdev, asp_mycdev, dev);
	file_ptr->private_data = mydev;
	return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file_ptr)
{
	pr_info("CLOSING device: %s\n\n", MYDEV_NAME);
	return 0;
}

static ssize_t
mycdrv_read(struct file *file_ptr, char __user * buf, size_t lbuf, loff_t * ppos)
{
	asp_mycdev *dev_ptr;
	ssize_t retval;

	dev_ptr = (asp_mycdev *)file_ptr->private_data;
	pr_info("File offset in read: %lld",*ppos );
	if(down_interruptible(&(dev_ptr->sem))!=0)
	{
		pr_err("%s unable to access semaphore\n", MYDEV_NAME);
		retval=  -ERESTARTSYS;
	}
	/* Enter criticle region */
	if(lbuf < 0 || *ppos<0 || *ppos > dev_ptr->disk_size || *ppos>=dev_ptr->size)
	{
		pr_info("Invalid offset");
		retval = -EINVAL;
		goto out;
	}
	if(lbuf + *ppos > dev_ptr->size)
	{
		lbuf = dev_ptr->size - *ppos;
	}
	pr_info("Buffer 1 :%s, %ld \n",buf,lbuf );
	copy_to_user(buf, (dev_ptr->ramdisk) + (*ppos), lbuf);
	(*ppos) += lbuf;	
	retval= lbuf;
	pr_info("Buffer :%s",buf );
	/* End criticle region */
out:up(&(dev_ptr->sem));
	return retval;
}

static ssize_t
mycdrv_write(struct file *file_ptr, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	asp_mycdev *dev_ptr;
	ssize_t retval;

	dev_ptr = (asp_mycdev *)file_ptr->private_data;
	
	if(down_interruptible(&(dev_ptr->sem))!=0)
	{
		pr_err("%s unable to access semaphore\n", MYDEV_NAME);
		retval=  -ERESTARTSYS;
	}
	/* Enter criticle region */
	if(lbuf < 0 || *ppos<0 || lbuf + *ppos > dev_ptr->disk_size)
	{
		retval = -EINVAL;
		goto out;
	}
	copy_from_user(((dev_ptr->ramdisk) + (*ppos)),buf, lbuf);
	(*ppos)+=lbuf;
	retval = lbuf;
	if(dev_ptr->size < *ppos)
	{
		dev_ptr->size = *ppos;
	}
	pr_info("File sixe in write: %d",dev_ptr->size );
	/* End criticle region */
out:up(&(dev_ptr->sem));
	return retval;
}

loff_t mycdrv_lseek(struct file *file_ptr, loff_t off, int type) {

	asp_mycdev *dev_ptr;
	loff_t new_pos;

	dev_ptr = (asp_mycdev *)file_ptr->private_data;
	
	if(down_interruptible(&(dev_ptr->sem))!=0)
	{
		pr_err("%s unable to access semaphore\n", MYDEV_NAME);
		new_pos = -EINVAL;
	}

	switch(type){
		case 0:
			new_pos = off;		
			break;
		case 1:
			new_pos = file_ptr->f_pos + off;
			break;
		case 2:
			new_pos = dev_ptr->size + off;
			break;
		default:
			new_pos = -EINVAL;
		goto out;
	}

	if(new_pos < 0){
		new_pos = -EINVAL;
		goto out;
	}
	if(new_pos >=dev_ptr->disk_size)
	{
		dev_ptr->ramdisk=krealloc(dev_ptr->ramdisk,new_pos+PAGE_SIZE,GFP_KERNEL);
		(dev_ptr->disk_size) = new_pos+PAGE_SIZE;
		memset((void *)((dev_ptr->ramdisk)+dev_ptr->size), '0', new_pos - (dev_ptr->size) );
		(dev_ptr->size)=new_pos;
	}
	else if(new_pos>dev_ptr->size)
	{
		memset((void *)((dev_ptr->ramdisk)+dev_ptr->size), '0', new_pos - (dev_ptr->size) );
		(dev_ptr->size)=new_pos;
	}
	
	file_ptr->f_pos = new_pos;

out:up(&(dev_ptr->sem));
	return new_pos;
	
}

long mycdrv_ioctl(struct file *file_ptr, unsigned int in_cmd, unsigned long dir) {

	asp_mycdev *dev_ptr = (asp_mycdev *)file_ptr->private_data;
	if(in_cmd != ASP_CLEAR_BUFF)
	{
		return -EINVAL;
	}
	if(down_interruptible(&(dev_ptr->sem))!=0){
		pr_err("%s: not able to lock the semaphore\n", MYDEV_NAME);
	}
	memset((void *)dev_ptr->ramdisk, 0, dev_ptr->disk_size);
	file_ptr->f_pos=0;
	dev_ptr->size=0;	
	up(&(dev_ptr->sem));
	return 0;
}

static int setup_Singlecdev(asp_mycdev *dev, int minor, struct class *my_class)
{
	dev_t devno = MKDEV(major, minor);
	struct device *device_ptr = NULL;
	dev->devNo = minor;
	dev->dev.owner = THIS_MODULE;
	sema_init(&(dev->sem),1);
	cdev_init(&dev->dev, &fops);
	dev->ramdisk = (unsigned char*) kmalloc(ramdisk_size, GFP_KERNEL);
	dev->size = 0;
	dev->disk_size = ramdisk_size;
	cdev_add(&dev->dev, devno, 1);
	device_ptr = device_create(my_class, NULL, devno, NULL, MYDEV_NAME "%d", minor);

	return 0;
}

static void my_cleanup_module(void)
{
	int i;
	if (dev_pointer) 
	{
		for(i=0; i<NUM_DEVICES; i++)
		{
			device_destroy(MydevClass, MKDEV(major, i));
			kfree((dev_pointer[i].ramdisk));
			cdev_del(&(dev_pointer[i].dev));
		}
	}
	if (MydevClass)
	{
		class_destroy(MydevClass);
	}
	kfree(dev_pointer);
	unregister_chrdev_region(MKDEV(major, 0), NUM_DEVICES);
}


static int __init my_init(void)
{
	dev_t dev = 0;
	int i,result;
	result = 0;
	result = alloc_chrdev_region(&dev, 0, NUM_DEVICES,MYDEV_NAME);
	
	if(result<0)
	{
		pr_err(" Unable to be major number");
		return result;
	}
	major=MAJOR(dev);
	pr_info("Registering Device :%s , with major number :%d ",MYDEV_NAME,major);
	MydevClass = class_create(THIS_MODULE,MYDEV_NAME);

	dev_pointer = (asp_mycdev *)kmalloc(sizeof(asp_mycdev)*NUM_DEVICES,GFP_KERNEL);

	if(dev_pointer == NULL)
	{
		pr_err("Memmory allocation failure");
	}

	for( i=0; i<NUM_DEVICES; i++){
		result = setup_Singlecdev(&dev_pointer[i], i, MydevClass);
		if(result<0)
		{
			my_cleanup_module();
			return result;
		}
	}
	return 0;
}

static void __exit my_exit(void)
{
	my_cleanup_module();
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.llseek = mycdrv_lseek,
	.open = mycdrv_open,
	.release = mycdrv_release,
	.unlocked_ioctl = mycdrv_ioctl,
};

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Akash Someshwar Rao");
MODULE_LICENSE("GPL v2");
