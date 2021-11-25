
/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2002-2006 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   o Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.
   o Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   o Neither the name of GE Fanuc nor the names of its contributors may be used
     to endorse or promote products derived from this software without specific
     prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/

#ifndef ARCH
#include <linux/config.h>
#ifdef CONFIG_SMP
#define __SMP__
#endif
#endif

#ifdef ARCH
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#else
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/version.h>
#endif

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#include <linux/interrupt.h>
#endif

#include "vme/vme.h"
#include "vme/vme_api.h"

#ifdef ARCH
#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif
#include "hw_diag.h"
#endif

#ifdef DEBUG
#define DPRINTF(x...)  printk(KERN_DEBUG "VME: "x)
#else
#define DPRINTF(x...)
#endif

#define UNIVERSE_MAJOR   221	/* Major device number */
#define MODULE_NAME "vme_universe"

EXPORT_SYMBOL(vme_init);
EXPORT_SYMBOL(vme_term);


int vme_open(struct inode *inode, struct file *file_ptr);
int vme_close(struct inode *inode, struct file *file_ptr);
int vme_mmap(struct file *fp, struct vm_area_struct *vma);
int vme_ioctl(struct inode *inode, struct file *file_ptr, unsigned int cmd,
	      unsigned long arg);

static struct file_operations file_ops = {
      ioctl:vme_ioctl,
      mmap:vme_mmap,
      open:vme_open,
      release:vme_close
};

extern int universe_init(void);
extern void universe_term(void);
extern int vmic_init(void);
extern void vmic_term(void);
extern int ctl_init(void);
extern void ctl_term(void);
extern int master_init(void);
extern void master_term(void);
extern void reclaim_master_handles(vme_bus_handle_t bus_handle);
extern int slave_init(void);
extern void slave_term(void);
extern void reclaim_slave_handles(vme_bus_handle_t bus_handle);
extern int interrupt_init(void);
extern void interrupt_term(void);
extern void reclaim_interrupt_handles(vme_bus_handle_t bus_handle);
extern void reclaim_dma_handles(vme_bus_handle_t bus_handle);
extern int vrai_init(void);
extern void vrai_term(void);
extern void reclaim_vrai_handles(vme_bus_handle_t bus_handle);
extern int location_monitor_init(void);
extern void location_monitor_term(void);
extern void reclaim_location_monitor_handles(vme_bus_handle_t bus_handle);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
extern void universe_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
extern int universe_isr(int irq, void *dev_id, struct pt_regs *regs);
#endif

static char *version = VME_UNIVERSE_VERSION;
static int if_version = VME_IF_VERSION;
uint32_t pci_lo_bound = 0, pci_hi_bound = ~0;	/* Set these variables to the
						   safe range of addresses for
						   dynamically created VMEbus
						   master windows. If you add
						   support for a new board, be
						   sure to check that you set
						   these to safe values.
						 */
extern struct pci_dev *universe_pci_dev;
static struct proc_dir_entry *proc_entry;


#ifdef ARCH
#ifdef CONFIG_DEVFS_FS
devfs_handle_t devfs_handle;
#endif
#endif

MODULE_AUTHOR("GE Fanuc <www.gefanuc.com/embedded>");
MODULE_DESCRIPTION("Tundra Universe II VMEbus driver");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

MODULE_PARM(pci_lo_bound, "i");
MODULE_PARM(pci_hi_bound, "i");



/*============================================================================
 * Initialize the VMEbus driver interface.
 */
int vme_init(vme_bus_handle_t * handle)
{
	/* The purpose of the bus handle is to keep track of resources so they
	   can be freed if the caller forgets to do it.  Since this function
	   is only called by other modules, we don't bother to keep track of
	   it's resources. It would be nice if we could, but I haven't figured
	   out a clean way to implement it. Bottom line is, the handle is
	   avaible if a suitable implementation is found, but for now, modules
	   are expected to clean up after themseleves.
	 */
	*handle = (void *) 1;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	MOD_INC_USE_COUNT;
#endif	
	return 0;
}


/*============================================================================
 * Cleanup the VMEbus driver interface.
 */
int vme_term(vme_bus_handle_t handle)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	MOD_DEC_USE_COUNT;
#endif	
	return 0;
}

/*============================================================================
 * Hook to the open file operation
 */
int vme_open(struct inode *inode, struct file *file_ptr)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	MOD_INC_USE_COUNT;
#endif	
	return 0;
}

/*============================================================================
 * Hook to the close file operation
 */
int vme_close(struct inode *inode, struct file *file_ptr)
{
	/* Free any remaining resources
	 */
	reclaim_master_handles((vme_bus_handle_t) file_ptr);
	reclaim_slave_handles((vme_bus_handle_t) file_ptr);
	reclaim_dma_handles((vme_bus_handle_t) file_ptr);
	reclaim_interrupt_handles((vme_bus_handle_t) file_ptr);
	reclaim_vrai_handles((vme_bus_handle_t) file_ptr);
	reclaim_location_monitor_handles((vme_bus_handle_t) file_ptr);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	MOD_DEC_USE_COUNT;
#endif	
	return 0;
}

#ifdef ARCH
static void 
vme_vma_open(struct vm_area_struct *vma)
{
    printk(KERN_INFO "vme: vma_open\n");
}

static void
vme_vma_close(struct vm_area_struct *vma)
{
    printk(KERN_INFO "vme: vma_close\n");
}

struct page * 
vme_vma_nopage(struct vm_area_struct *vma, unsigned long address, int unused)
{
    unsigned long               offset;
    struct page                 *page;
    uint32_t                    base;
    
    page = NOPAGE_SIGBUS;

    offset = (address - vma->vm_start);

#ifdef NOT_YET
    if(offset/* >= window_size*/)
    {
        printk(KERN_ERR "bogus: vma mapping is too big\n");
    }
    else
    {
        
        bogus = vma->vm_private_data;
#endif 
        base = vma->vm_pgoff << PAGE_SHIFT;
        /* Not sure if this is enough checking or too much ?? */
        if((base >= VMALLOC_START) && (base < VMALLOC_END))
        {
            //printk(KERN_INFO "vme: page is in VMALLOC (%08x)\n", base);
            page = vmalloc_to_page((void *)base);
        }
        else
        {
            /* massive assumption that we are a logical page and not
             * ioremap or some other region
             */
            //printk(KERN_INFO "vme: page is logical (%08x)\n", base);
            page = virt_to_page((void *)base);
        }
        get_page(page);
#ifdef NOT_YET
    }
#endif
    return page;
}

struct vm_operations_struct vme_vm_opts = {
    .open   = vme_vma_open,
    .close  = vme_vma_close,
    .nopage = vme_vma_nopage,
};

#endif

/*============================================================================
 * Hook to the mmap file operation
 */
int vme_mmap(struct file *file_ptr, struct vm_area_struct *vma)
{

#ifdef ARCH
    int                 status;
    unsigned long       prot;

    status = 0;

    /*
     * In theory we deal with at least 4 types of memory: logical, 
     * physical (RAM), vmalloc and PCI. The logic here is almost certainly
     * flawed and we should really us the handle from the window to do this
     * more sensibly but at the moment I make the following assumptions:
     *
     * 1) We never get a physical address here
     * 2) PCI memory is < CONFIG_KERNEL_START (normally 0xC0000000)
     * 2) PCI memory is < VMALLOC_START
     */
    if(((vma->vm_pgoff << PAGE_SHIFT) < CONFIG_KERNEL_START) &&
       ((vma->vm_pgoff << PAGE_SHIFT) < VMALLOC_START))
    {
        /* here be dragons */
        prot = pgprot_val(vma->vm_page_prot);
        prot |= _PAGE_NO_CACHE | _PAGE_GUARDED;
        vma->vm_page_prot = __pgprot(prot);

        /* Assumption : this is PCI memory */
        /* this prototype is different in 2.6 and some vendor kernels
         * where 2.6 functionality has been backported */
        io_remap_page_range(vma->vm_start, vma->vm_pgoff << PAGE_SHIFT,                                vma->vm_end - vma->vm_start, vma->vm_page_prot);
    }
    else
    {
        /* Assumption: this is a kernel virtual or logical address, the page
         *             fault handler deals with both types.
         */
        vma->vm_ops = &vme_vm_opts;
        vma->vm_flags |= VM_RESERVED | VM_IO;

        /* here be dragons */
        prot = pgprot_val(vma->vm_page_prot);
        prot |= _PAGE_NO_CACHE | _PAGE_GUARDED;
        vma->vm_page_prot = __pgprot(prot);

        /* This is ugly as sin - the mmap should pass the handle 
         * we don't know at this point if we are a master or a slave and it
         * would make life easier.
         */
        vma->vm_private_data = NULL;
    }

    return status;

#else

	DPRINTF("Attempting to map %#lx bytes of memory at "
		"physical address %#lx\n", vma->vm_end - vma->vm_start,
		vma->vm_pgoff << PAGE_SHIFT);

#ifdef CONFIG_PPC32
//       vma->vm_page_prot.pgprot |= _PAGE_NO_CACHE | _PAGE_GUARDED;
//       DPRINTF("PowerPC protection flags set.\n");
#endif

	/* Don't swap these pages out
	 */
	vma->vm_flags |= VM_RESERVED | VM_LOCKED | VM_IO | VM_SHM;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
	return remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
				vma->vm_end - vma->vm_start, vma->vm_page_prot);

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,3) || defined RH9BRAINDAMAGE
	return remap_page_range(vma, vma->vm_start, vma->vm_pgoff << PAGE_SHIFT,
				vma->vm_end - vma->vm_start, vma->vm_page_prot);

#else
	return remap_page_range(vma->vm_start, vma->vm_pgoff << PAGE_SHIFT,
				vma->vm_end - vma->vm_start, vma->vm_page_prot);

#endif
#endif
}


/*============================================================================
 * Hook to the ioctl file operation
 */
int vme_ioctl(struct inode *inode, struct file *file_ptr, uint32_t cmd,
	      unsigned long arg)
{
	struct vmectl_window_t window;
	struct vmectl_dma_t dma;
	struct vmectl_interrupt_t interrupt;
	struct vmectl_rmw_t rmw;
	vme_bus_handle_t bus;
	vme_master_handle_t *master_p;
	vme_slave_handle_t *slave_p;
	vme_dma_handle_t *dma_p;
	vme_interrupt_handle_t *interrupt_p;
	int value;
	void *ptr;
	int status;

	/* There's a subtle trick here. Since the VMEbus handle
	   (vme_bus_handle_t) is just used as a key to keep track of who owns
	   a particular resource, we substitute the file pointer as the bus
	   handle in functions that have a bus handle parameter.
	 */
	bus = (vme_bus_handle_t) file_ptr;

	switch (cmd) {
	case VMECTL_VERSION:
		if (copy_to_user((void *) arg, &if_version, sizeof (int))) {
			return -EFAULT;
		}
		break;
	case VMECTL_MASTER_WINDOW_REQUEST:
		if (copy_from_user(&window, (void *) arg, sizeof (window))) {
			return -EFAULT;
		}

		master_p = (vme_master_handle_t *) & window.id;
		status = vme_master_window_create(bus, master_p, window.vaddr,
						  window.am,
						  window.size, window.flags,
						  window.paddr);
		if (status) {
			return status;
		}

		window.paddr = vme_master_window_phys_addr(bus, window.id);

		if (copy_to_user((void *) arg, &window, sizeof (window))) {
			return -EFAULT;
		}
		break;
	case VMECTL_MASTER_WINDOW_RELEASE:
		if (copy_from_user(&ptr, (void *) arg, sizeof (ptr))) {
			return -EFAULT;
		}

		return vme_master_window_release(bus, ptr);
		break;
	case VMECTL_MASTER_WINDOW_TRANSLATE:
		if (copy_from_user(&window, (void *) arg, sizeof (window))) {
			return -EFAULT;
		}

		return vme_master_window_translate(bus, window.id,
						   window.vaddr);
		break;
	case VMECTL_RMW:
		if (copy_from_user(&rmw, (void *) arg, sizeof (rmw))) {
			return -EFAULT;
		}

		return vme_read_modify_write(bus, rmw.id, rmw.offset, rmw.dw,
					     rmw.mask, rmw.cmp, rmw.swap);
		break;
	case VMECTL_SLAVE_WINDOW_REQUEST:
		if (copy_from_user(&window, (void *) arg, sizeof (window))) {
			return -EFAULT;
		}

		slave_p = (vme_slave_handle_t *) & window.id;
		status = vme_slave_window_create(bus, slave_p, window.vaddr,
						 window.am, window.size,
						 window.flags, window.paddr);
		if (status) {
			return status;
		}

		window.paddr = vme_slave_window_phys_addr(bus, window.id);

		if (copy_to_user((void *) arg, &window, sizeof (window))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SLAVE_WINDOW_RELEASE:
		if (copy_from_user(&ptr, (void *) arg, sizeof (ptr))) {
			return -EFAULT;
		}

		return vme_slave_window_release(bus, ptr);
		break;
	case VMECTL_DMA_BUFFER_ALLOC:
		if (copy_from_user(&dma, (void *) arg, sizeof (dma))) {
			return -EFAULT;
		}

		dma_p = (vme_dma_handle_t *) & dma.id;
		status = vme_dma_buffer_create(bus, dma_p, dma.size, dma.flags,
					       dma.paddr);
		if (status) {
			return status;
		}

		dma.paddr = vme_dma_buffer_phys_addr(bus, dma.id);
		if (!dma.paddr) {
			return -ENOMEM;
		}

		if (copy_to_user((void *) arg, &dma, sizeof (dma))) {
			return -EFAULT;
		}
		break;
	case VMECTL_DMA_BUFFER_FREE:
		if (copy_from_user(&ptr, (void *) arg, sizeof (ptr))) {
			return -EFAULT;
		}

		return vme_dma_buffer_release(bus, ptr);
		break;
	case VMECTL_DMA_READ:
		if (copy_from_user(&dma, (void *) arg, sizeof (dma))) {
			return -EFAULT;
		}

		return vme_dma_read(bus, dma.id, dma.offset, dma.vaddr, dma.am,
				    dma.size, dma.flags);
		break;
	case VMECTL_DMA_WRITE:
		if (copy_from_user(&dma, (void *) arg, sizeof (dma))) {
			return -EFAULT;
		}

		return vme_dma_write(bus, dma.id, dma.offset, dma.vaddr,
				     dma.am, dma.size, dma.flags);
		break;
	case VMECTL_INTERRUPT_ATTACH:
		if (copy_from_user(&interrupt.id, (void *) arg,
				   sizeof (interrupt))) {
			return -EFAULT;
		}

		interrupt_p = (vme_interrupt_handle_t *) & interrupt.id;
		status = vme_interrupt_attach(bus, interrupt_p, interrupt.level,
					      interrupt.vector, interrupt.flags,
					      &interrupt.int_data);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &interrupt, sizeof (interrupt))) {
			return -EFAULT;
		}
		break;
	case VMECTL_INTERRUPT_RELEASE:
		if (copy_from_user(&ptr, (void *) arg, sizeof (ptr))) {
			return -EFAULT;
		}

		return vme_interrupt_release(bus, ptr);
		break;
	case VMECTL_INTERRUPT_GENERATE:
		if (copy_from_user(&interrupt, (void *) arg,
				   sizeof (interrupt))) {
			return -EFAULT;
		}

		return vme_interrupt_generate(bus, interrupt.level,
					      interrupt.vector);
		break;
	case VMECTL_ACQUIRE_BUS_OWNERSHIP:
		return vme_acquire_bus_ownership(bus);
		break;
	case VMECTL_RELEASE_BUS_OWNERSHIP:
		return vme_release_bus_ownership(bus);
		break;
	case VMECTL_GET_BUS_OWNERSHIP:
		status = vme_get_bus_ownership(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_MAX_RETRY:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_max_retry(bus, value);
		break;
	case VMECTL_GET_MAX_RETRY:
		status = vme_get_max_retry(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_POSTED_WRITE_COUNT:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_posted_write_count(bus, value);
		break;
	case VMECTL_GET_POSTED_WRITE_COUNT:
		status = vme_get_posted_write_count(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_BUS_REQUEST_LEVEL:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_bus_request_level(bus, value);
		break;
	case VMECTL_GET_BUS_REQUEST_LEVEL:
		status = vme_get_bus_request_level(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_BUS_REQUEST_MODE:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_bus_request_mode(bus, value);
		break;
	case VMECTL_GET_BUS_REQUEST_MODE:
		status = vme_get_bus_request_mode(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_BUS_RELEASE_MODE:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_bus_release_mode(bus, value);
		break;
	case VMECTL_GET_BUS_RELEASE_MODE:
		status = vme_get_bus_release_mode(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_BUS_TIMEOUT:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_bus_timeout(bus, value);
		break;
	case VMECTL_GET_BUS_TIMEOUT:
		status = vme_get_bus_timeout(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_BUS_ARB_MODE:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_arbitration_mode(bus, value);
		break;
	case VMECTL_GET_BUS_ARB_MODE:
		status = vme_get_arbitration_mode(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_BUS_ARB_TIMEOUT:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_arbitration_timeout(bus, value);
		break;
	case VMECTL_GET_BUS_ARB_TIMEOUT:
		status = vme_get_arbitration_timeout(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_MEC:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_master_endian_conversion(bus, value);
		break;
	case VMECTL_GET_MEC:
		status = vme_get_master_endian_conversion(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_SEC:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_slave_endian_conversion(bus, value);
		break;
	case VMECTL_GET_SEC:
		status = vme_get_slave_endian_conversion(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_SET_ENDIAN_BYPASS:
		if (copy_from_user(&value, (void *) arg, sizeof (value))) {
			return -EFAULT;
		}

		return vme_set_endian_conversion_bypass(bus, value);
		break;
	case VMECTL_GET_ENDIAN_BYPASS:
		status = vme_get_endian_conversion_bypass(bus, &value);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &value, sizeof (value))) {
			return -EFAULT;
		}
		break;
	case VMECTL_ASSERT_SYSRESET:
		return vme_sysreset(bus);
		break;
	case VMECTL_VRAI_REQUEST:
		if (copy_from_user(&window, (void *) arg, sizeof (window))) {
			return -EFAULT;
		}

		status = vme_register_image_create(bus, (vme_vrai_handle_t *)
						   & window.id, window.vaddr,
						   window.am, window.flags);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &window, sizeof (window))) {
			return -EFAULT;
		}
		break;
	case VMECTL_VRAI_RELEASE:
		if (copy_from_user(&ptr, (void *) arg, sizeof (ptr))) {
			return -EFAULT;
		}

		return vme_register_image_release(bus, ptr);
		break;
	case VMECTL_LM_REQUEST:
		if (copy_from_user(&window, (void *) arg, sizeof (window))) {
			return -EFAULT;
		}

		status = vme_location_monitor_create(bus, (vme_lm_handle_t *)
						     & window.id, window.vaddr,
						     window.am, 0,
						     window.flags);
		if (status) {
			return status;
		}

		if (copy_to_user((void *) arg, &window, sizeof (window))) {
			return -EFAULT;
		}
		break;
	case VMECTL_LM_RELEASE:
		if (copy_from_user(&ptr, (void *) arg, sizeof (ptr))) {
			return -EFAULT;
		}

		return vme_location_monitor_release(bus, ptr);
		break;
	default:
		return -ENOTTY;
	}

	return 0;
}


/*============================================================================
 * Tundra Universe II driver module exit routine
 */
void cleanup_module(void)
{
	DPRINTF("exiting vme_universe module\n");
	
#ifdef ARCH
	remove_proc_entry("vme", NULL);

#ifndef CONFIG_DEVFS_FS
	unregister_chrdev(UNIVERSE_MAJOR, MODULE_NAME);
#else
	devfs_unregister(devfs_handle);
#endif

	location_monitor_term();
	vrai_term();
	interrupt_term();
	slave_term();
	master_term();
	vmic_term();
	universe_term();

	free_irq(universe_pci_dev->irq, universe_pci_dev);

#else	

	interrupt_term();
	slave_term();
	master_term();
  ctl_term();
  vmic_term();
	location_monitor_term();
	vrai_term();
  universe_term();

	remove_proc_entry("vme", NULL);
	free_irq(universe_pci_dev->irq, universe_pci_dev);
	unregister_chrdev(UNIVERSE_MAJOR, MODULE_NAME);
#endif

	printk(KERN_NOTICE "VME: exiting vme_universe module version: %s\n",
	       version);
}


/*===========================================================================
 * Tundra Universe II driver module initialization
 */
int init_module(void)
{

	proc_entry = proc_mkdir("vme", NULL);
	if (!proc_entry) {
		printk(KERN_WARNING
		       "VME: Failed to register vme proc directory\n");
		/* Not a fatal error */
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	if (!pci_present()) {
		printk(KERN_ERR "VME: PCI bios not found\n");
		return -1;
	}
#endif
	if (vmic_init()) {
		printk(KERN_ERR "VME: Failure initializing VMIC devices\n");
		cleanup_module();
		return -1;
	}

	if (universe_init()) {
		printk(KERN_ERR
		       "VME: Failure initializing Tundra Universe device\n");
		cleanup_module();
		return -1;
	}

	if (ctl_init()) {
		printk(KERN_ERR
		       "VME: Failure initializing control registers\n");
		cleanup_module();
		return -1;
	}

	if (master_init()) {
		printk(KERN_ERR "VME: Failure initializing master windows\n");
		cleanup_module();
		return -1;
	}

	if (slave_init()) {
		printk(KERN_ERR "VME: Failure initializing slave windows\n");
		cleanup_module();
		return -1;
	}

	if (interrupt_init()) {
		printk(KERN_ERR "VME: Failure initializing interrupts\n");
		cleanup_module();
		return -1;
	}

	if (vrai_init()) {
		printk(KERN_ERR
		       "VME: Failure initializing VMEbus register access image\n");
		cleanup_module();
		return -1;
	}

	if (location_monitor_init()) {
		printk(KERN_ERR
		       "VME: Failure initializing location monitor image\n");
		cleanup_module();
		return -1;
	}

	if (request_irq(universe_pci_dev->irq, universe_isr, SA_SHIRQ,
			MODULE_NAME, universe_pci_dev)) {
		printk(KERN_ERR " VME: Failure requesting irq %d\n",
		       universe_pci_dev->irq);
		cleanup_module();
		return -1;
	}

#ifdef ARCH
#ifndef CONFIG_DEVFS_FS
	if (register_chrdev(UNIVERSE_MAJOR, MODULE_NAME, &file_ops)) {
		printk(KERN_ERR "VME: Failed registering device\n");
		cleanup_module();
		return -1;
	}
#else
	printk(KERN_NOTICE "VME: registering devfs device\n");
	devfs_handle = devfs_register(NULL,
				"vme_ctl",
				DEVFS_FL_NONE,
				221,8,
				S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP,
				&file_ops,
				NULL);
#endif
#else
	if (register_chrdev(UNIVERSE_MAJOR, MODULE_NAME, &file_ops)) {
		printk(KERN_ERR "VME: Failed registering device\n");
		cleanup_module();
		return -1;
	}
#endif

	printk(KERN_NOTICE "VME: Driver compiled for %s system\n",
#ifdef __SMP__
	       "SMP"
#else
	       "UP"
#endif
	    );

	printk(KERN_NOTICE "VME: Installed VME Universe module version: %s\n",
	       version);

	return 0;
}
