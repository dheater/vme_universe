
#include <linux/modversions.h>
#include <linux/config.h>
#ifdef CONFIG_SMP
#define __SMP__
#endif

#include <linux/module.h>
#include <asm/uaccess.h>
#include "vme/vme.h"
#include "vme/vme_api.h"


#define MODULE_NAME  "vme_example"
MODULE_AUTHOR("GE Fanuc <www.gefanuc.com/embedded>");
MODULE_DESCRIPTION("VME module example");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

int vme_open(struct inode *inode, struct file *file_ptr);
int vme_close(struct inode *inode, struct file *file_ptr);
ssize_t vme_read(struct file *file_ptr, char *buffer, size_t size,
		 loff_t * off);
ssize_t vme_write(struct file *file_ptr, const char *buffer, size_t size,
		  loff_t * off);

static struct file_operations file_ops = {
	open:vme_open,
	release:vme_close,
	read:vme_read,
	write:vme_write,
};


static vme_bus_handle_t bus;
static vme_master_handle_t window;
static vme_interrupt_handle_t interrupt;
static char *ptr;
static int irq, major;


#define VME_ADDRESS          0x08000000
#define ADDRESS_MODIFIER     VME_A32SD
#define NBYTES               0x40
#define LEVEL                VME_INTERRUPT_VIRQ3
#define VECTOR               0x10


/*============================================================================
 * Hook to the open file operation
 */
int vme_open(struct inode *inode, struct file *file_ptr)
{
	MOD_INC_USE_COUNT;
	return 0;
}


/*============================================================================
 * Hook to the close file operation
 */
int vme_close(struct inode *inode, struct file *file_ptr)
{
	MOD_DEC_USE_COUNT;
	return 0;
}


/*============================================================================
 * Hook to the read file operation
 */
ssize_t vme_read(struct file * file_ptr, char *buffer, size_t size,
		 loff_t * off)
{
	int nbytes, bytes_rem;

	/* Calculate how many bytes we can read (all|some|none)?
	 */
	nbytes = (size > NBYTES - *off) ? NBYTES - *off : size;
	if (0 >= nbytes)
		return 0;

	/* Transfer the data
	 */
	bytes_rem = copy_to_user(buffer, ptr + *off, nbytes);
	if (bytes_rem)
		return -EFAULT;

	nbytes -= bytes_rem;

	/* Update the file position
	 */
	*off += nbytes;

	/* Return the actual number of bytes transferred
	 */
	return nbytes;
}


/*============================================================================
 * Hook to the write file operation
 */
ssize_t vme_write(struct file * file_ptr, const char *buffer, size_t size,
		  loff_t * off)
{
	int nbytes, bytes_rem;

	/* Calculate how many bytes we can read (all|some|none)?
	 */
	nbytes = (size > NBYTES - *off) ? NBYTES - *off : size;
	if (0 >= nbytes)
		return 0;

	/* Transfer the data
	 */
	bytes_rem = copy_from_user(ptr + *off, buffer, nbytes);
	if (bytes_rem)
		return -EFAULT;

	nbytes -= bytes_rem;

	/* Update the file position
	 */
	*off += nbytes;

	/* Return the actual number of bytes transferred
	 */
	return nbytes;
}


/*============================================================================
 * Interrupt service routine
 */
void vme_isr(int irq, void *dev_id, struct pt_regs *regs)
{
	int vector;

	/* Is the interrupt on our level? If vme_interrupt_asserted returns
	   true, then the interrupt was on the level that we reserved and we
	   should do something clever.
	 */
	if (vme_interrupt_asserted(bus, interrupt)) {
		vme_interrupt_vector(bus, interrupt, &vector);
		printk(KERN_NOTICE MODULE_NAME ":Interrupt vector 0x%x\n",
		       vector);

		/* Clear the interrupt
		 */
		vme_interrupt_clear(bus, interrupt);
	}
}


/*===========================================================================
 * Driver module initialization
 */
int init_module(void)
{
	if (vme_init(&bus)) {
		printk(KERN_ERR MODULE_NAME ": Failed vme_init\n");
		return -1;
	}

	if (vme_master_window_create(bus, &window, VME_ADDRESS,
				     ADDRESS_MODIFIER, NBYTES, VME_CTL_PWEN,
				     NULL)) {
		printk(KERN_ERR MODULE_NAME
		       ":Failed vme_master_window_create\n");
		goto error_create;
	}

	ptr = vme_master_window_map(bus, window, 0);
	if (NULL == ptr) {
		printk(KERN_ERR MODULE_NAME ":Failed vme_master_window_map\n");
		goto error_map;
	}

	/* Get the IRQ of the VMEbus bridge device
	 */
	if (vme_interrupt_irq(bus, &irq)) {
		printk(KERN_ERR MODULE_NAME ":Failed vme_interrupt_irq\n");
		goto error_irq;
	}

	if (request_irq(irq, vme_isr, SA_SHIRQ, MODULE_NAME, &file_ops)) {
		printk(KERN_ERR MODULE_NAME ":Failed reqest_irq\n");
		goto error_irq;
	}

	/* The VME_INTERRUPT_RESERVE flags tells the VMEbus bridge driver to not
	   do anything with interrupts on the level we specify here (not just
	   level/vector, but the entire level). Therefore, be sure to always do
	   the request_irq before vme_interrupt_attach, otherwise you will have
	   a race condition where noone handles the interrupt, and your system
	   could lock up.
	 */
	if (vme_interrupt_attach(bus, &interrupt, LEVEL, VECTOR,
				 VME_INTERRUPT_RESERVE, NULL)) {
		printk(KERN_ERR MODULE_NAME ":Failed vme_interrupt_attach\n");
		goto error_interrupt_attach;
	}

	/* We let the kernel pick a character device major number for us.
	 */
	major = register_chrdev(0, MODULE_NAME, &file_ops);
	if ((-EBUSY == major) || (-EINVAL == major)) {
		printk(KERN_ERR MODULE_NAME ":Failed register_chrdev\n");
		goto error_register;
	}

	return 0;

      error_register:
	vme_interrupt_release(bus, interrupt);

      error_interrupt_attach:
	free_irq(irq, &file_ops);

      error_irq:
	vme_master_window_unmap(bus, window);

      error_map:
	vme_master_window_release(bus, window);

      error_create:
	vme_term(bus);

	return -1;
}


/*============================================================================
 * Driver module exit routine
 */
void cleanup_module(void)
{
	unregister_chrdev(major, MODULE_NAME);
	vme_interrupt_release(bus, interrupt);
	free_irq(irq, &file_ops);
	vme_master_window_unmap(bus, window);
	vme_master_window_release(bus, window);
	vme_term(bus);
}
