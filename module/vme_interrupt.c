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


#include <linux/config.h>
#ifdef CONFIG_SMP
#define __SMP__
#endif

#ifdef  ARCH
#ifdef  MODVERSIONS
#include <linux/modversions.h>
#endif
#endif

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "vme/universe.h"
#include "vme/vme.h"
#include "vme/vme_api.h"
#include "vme/vmivme.h"


#ifdef DEBUG
#define DPRINTF(x...)  printk(KERN_DEBUG "VME: "x)
#else
#define DPRINTF(x...)
#endif


EXPORT_SYMBOL_NOVERS(vme_interrupt_attach);
EXPORT_SYMBOL_NOVERS(vme_interrupt_release);
EXPORT_SYMBOL_NOVERS(vme_interrupt_generate);
EXPORT_SYMBOL_NOVERS(vme_interrupt_irq);
EXPORT_SYMBOL_NOVERS(vme_interrupt_clear);
EXPORT_SYMBOL_NOVERS(vme_interrupt_asserted);
EXPORT_SYMBOL_NOVERS(vme_interrupt_vector);

#define VME_INTERRUPT_MAGIC 0x10e30002
#define VME_INTERRUPT_MAGIC_NULL 0x0

struct __vme_interrupt {
	int level;
	int count;
	vme_interrupt_handle_t handles;
};

struct _vme_interrupt_handle {
	int magic;
	struct __vme_interrupt *interrupt;
	uint8_t vector;
	int flags;
	union {
		int data;
		siginfo_t siginfo;
	} notify;
	pid_t pid;
	void *id;
	vme_interrupt_handle_t next;
	vme_interrupt_handle_t prev;
};


static struct __vme_interrupt vme_interrupt[UNIV_MAX_IRQ + 1];
wait_queue_head_t interrupt_wq[UNIV_IRQS];
static rwlock_t interrupt_rwlock = RW_LOCK_UNLOCKED;
static struct proc_dir_entry *interrupt_proc_entry;
static struct tasklet_struct bh_tasklet[UNIV_MAX_IRQ + 1];
static int interrupts_generated[UNIV_VIRQS] = { 0 };
extern void *vmic_base;
extern int vmic_reg_type;
extern void *universe_base;
extern struct pci_dev *universe_pci_dev;


/*============================================================================
 * Hook for display proc page info
 * WARNING: If the amount of data displayed exceeds a page, then we need to
 * change how this page gets registered.
 */
static int read_interrupt_proc_page(char *buf, char **start, off_t offset,
				    int len, int *eof_unused, void *data_unused)
{
	unsigned long last_berr_address = 0;
	int last_berr_am = 0;
	int level, nbytes = 0;
	uint32_t reg;

	reg = readl(universe_base + UNIV_LINT_EN);
	nbytes += sprintf(buf + nbytes, "VMEbus interrupt enable=%#x\n\n", reg);

	reg = readl(universe_base + UNIV_LINT_STAT);
	nbytes += sprintf(buf + nbytes, "VMEbus interrupt status=%#x\n\n", reg);

	nbytes += sprintf(buf + nbytes,
			  "Number of VMEbus interrupts generated\n");
	for (level = 0; level < UNIV_VIRQS; ++level) {
		nbytes += sprintf(buf + nbytes, "  Level %d count = %d\n",
				  level + 1, interrupts_generated[level]);
	}

	nbytes += sprintf(buf + nbytes, "\n");

	reg = readl(universe_base + UNIV_STATID) >> 24;
	nbytes += sprintf(buf + nbytes,
			  "Last interrupt vector generated = %#x\n\n", reg);

	nbytes += sprintf(buf + nbytes,
			  "Number of VMEbus interrupts received\n");
	for (level = 0; level <= UNIV_MAX_IRQ; ++level) {
		nbytes += sprintf(buf + nbytes, "  Level %d count = %d\n",
				  level, vme_interrupt[level].count);
		switch (level) {
		case VME_INTERRUPT_VIRQ1:
		case VME_INTERRUPT_VIRQ2:
		case VME_INTERRUPT_VIRQ3:
		case VME_INTERRUPT_VIRQ4:
		case VME_INTERRUPT_VIRQ5:
		case VME_INTERRUPT_VIRQ6:
		case VME_INTERRUPT_VIRQ7:
			reg = readl(universe_base + UNIV_V_STATID(level));
			nbytes += sprintf(buf + nbytes,
					  "    Last vector = %#x\n", reg);
			break;
		case VME_INTERRUPT_DMA:
			reg = readl(universe_base + UNIV_DGCS);
			nbytes += sprintf(buf + nbytes,
					  "   Last DGCS register value = %#x\n",
					  reg);
			break;
		case VME_INTERRUPT_MBOX0:
		case VME_INTERRUPT_MBOX1:
		case VME_INTERRUPT_MBOX2:
		case VME_INTERRUPT_MBOX3:
			reg = readl(universe_base +
				    UNIV_MBOX(level - VME_INTERRUPT_MBOX0));
			nbytes += sprintf(buf + nbytes,
					  "    Last mailbox value = %#x\n",
					  reg);
			break;
		}
	}

	nbytes += sprintf(buf + nbytes, "\n");

	switch (vmic_reg_type) {
	case VMIVME_PLX:
		last_berr_address = readl(vmic_base + VMIVMEP_B_ADDRESS);
		last_berr_am =
		    VMIVMEP_BERR_AM(readl(vmic_base + VMIVMEP_B_INT_STATUS));
		break;
	case VMIVME_ISA:
		last_berr_address = readl(vmic_base + VMIVMEI_VBAR);
		last_berr_am =
		    readl(vmic_base + VMIVMEI_VBAMR) & VMIVMEI_VBAMR__AM;
		break;
	case VMIVME_FPGA:
		last_berr_address = readl(vmic_base + VMIVMEF_VBAR);
		last_berr_am =
		    readl(vmic_base + VMIVMEF_VBAMR) & VMIVMEF_VBAMR__AM;
		break;
	default:
		last_berr_address = readl(universe_base + UNIV_VAERR);
		last_berr_am =
		    UNIV_BERR_AM(readl(universe_base + UNIV_V_AMERR));
		break;
	}
	nbytes += sprintf(buf + nbytes, "Last bus error address = %#lx\n",
			  last_berr_address);
	nbytes += sprintf(buf + nbytes,
			  "Last bus error address modifier = %#x\n",
			  last_berr_am);

	return nbytes;
}


/*============================================================================
 * Insert an interrupt handle into the interrupt handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void insert_interrupt_handle(struct __vme_interrupt *_interrupt,
					   vme_interrupt_handle_t handle)
{
	handle->interrupt = _interrupt;
	handle->next = _interrupt->handles;
	handle->prev = NULL;
	if (handle->next) {
		handle->next->prev = handle;
	}

	_interrupt->handles = handle;
}


/*============================================================================
 * Remove an interrupt handle from the interrupt handles list
 * NOTE: The calling process should already have acquired any necessary locks
 */
static inline void remove_interrupt_handle(vme_interrupt_handle_t handle)
{
	handle->magic = VME_INTERRUPT_MAGIC_NULL;

	if (handle->prev)	/* This is not the first node */
		handle->prev->next = handle->next;
	else			/* This was the first node */
		handle->interrupt->handles = handle->next;

	if (handle->next)	/* If this is not the last node */
		handle->next->prev = handle->prev;

	/* If there are no handles, and this is not the DMA, VOWN or BERR
	   interrupt, then disable the interrupt level.
	 */
	if (!handle->interrupt->handles
	    && (VME_INTERRUPT_DMA != handle->interrupt->level)
	    && (VME_INTERRUPT_VOWN != handle->interrupt->level)
	    && (VME_INTERRUPT_BERR != handle->interrupt->level)) {
		writel(readl(universe_base + UNIV_LINT_EN) &
		       ~(1 << handle->interrupt->level),
		       universe_base + UNIV_LINT_EN);
	}
}


/*============================================================================
 * Recover all interrupt handles owned by the current process.
 */
void reclaim_interrupt_handles(vme_bus_handle_t bus_handle)
{
	vme_interrupt_handle_t ptr, tptr;
	unsigned long irqflags;
	int ii;

	write_lock_irqsave(&interrupt_rwlock, irqflags);

	for (ii = 0; ii <= UNIV_MAX_IRQ; ++ii) {
		ptr = vme_interrupt[ii].handles;
		while (ptr) {
			if (ptr->id == bus_handle) {
				tptr = ptr;
				ptr = ptr->next;
				remove_interrupt_handle(tptr);
				kfree(tptr);
			} else {
				ptr = ptr->next;
			}
		}
	}

	write_unlock_irqrestore(&interrupt_rwlock, irqflags);
}


/*============================================================================
 * Send a signal to the user process associated with this handle
 */
static inline void __vme_send_signal(vme_interrupt_handle_t handle, int level)
{
	int set_cap;

	/* This is a terrible hack to get kill_proc to work!! The problem
	   is that the bad signal test uses current struct; but current struct
	   is not valid in interrupt context. This means that bad_signal may
	   fail causing kill_proc to fail (see signal.c in the kernel
	   source).  My solution is to raise the current effective capabilty
	   to allow kill. -DLH
	 */
	set_cap = cap_raised(current->cap_effective, CAP_KILL);
	cap_raise(current->cap_effective, CAP_KILL);

#ifdef ARCH
	if (kill_proc_info(handle->notify.siginfo.si_signo,
			   &handle->notify.siginfo, handle->pid)) {
#else
	if (kill_proc(handle->pid, handle->notify.siginfo.si_signo,
			   (int)&handle->notify.siginfo)) {
#endif
		printk(KERN_ERR "VME: Error occurred while sending signal to "
		       "process %d for interrupt level %d\n", handle->pid,
		       level);
	}
}


/*============================================================================
 * Bottom half of the Universe ISR
 */
void universe_isr_bh(unsigned long level)
{
	vme_interrupt_handle_t handle;
	unsigned long irqflags;
	int vector = 0;
	int data = 0;
	int am;
	int index;
	int flush;

	switch (level) {
	case VME_INTERRUPT_VIRQ1:
	case VME_INTERRUPT_VIRQ2:
	case VME_INTERRUPT_VIRQ3:
	case VME_INTERRUPT_VIRQ4:
	case VME_INTERRUPT_VIRQ5:
	case VME_INTERRUPT_VIRQ6:
	case VME_INTERRUPT_VIRQ7:
		vector = readl(universe_base + UNIV_V_STATID(level));
		data = (level << 8) | vector;
		break;
	case VME_INTERRUPT_DMA:
		data = readl(universe_base + UNIV_DGCS);
		break;
	case VME_INTERRUPT_BERR:
		switch (vmic_reg_type) {
		case VMIVME_PLX:
			data = readl(vmic_base + VMIVMEP_B_ADDRESS);
			am = VMIVMEP_BERR_AM(readl(vmic_base +
						   VMIVMEP_B_INT_STATUS));
			writel(VMIVMEP_B_INT_STATUS__BERR_S,
			       vmic_base + VMIVMEP_B_INT_STATUS);
			writew(readl(vmic_base + VMIVMEP_COMM) |
			       VMIVMEP_COMM__BERRIM, vmic_base + VMIVMEP_COMM);
			break;
		case VMIVME_ISA:
			data = readl(vmic_base + VMIVMEI_VBAR);
			am = readw(vmic_base + VMIVMEI_VBAMR);
			writew(readw(vmic_base + VMIVMEI_COMM) |
			       VMIVMEI_COMM__BERRST, vmic_base + VMIVMEI_COMM);
			writew(readw(vmic_base + VMIVMEI_COMM) |
			       VMIVMEI_COMM__BERRI, vmic_base + VMIVMEI_COMM);
			break;
		case VMIVME_FPGA:
			data = readl(vmic_base + VMIVMEF_VBAR);
			am = readl(vmic_base + VMIVMEF_VBAMR);
			writel(readl(vmic_base + VMIVMEF_COMM) |
			       VMIVMEF_COMM__BERRST, vmic_base + VMIVMEF_COMM);
			writel(readl(vmic_base + VMIVMEF_COMM) |
			       VMIVMEF_COMM__BERRI, vmic_base + VMIVMEF_COMM);
			break;
		default:
			data = readl(universe_base + UNIV_VAERR);
			am = UNIV_BERR_AM(readl(universe_base + UNIV_V_AMERR));
			break;
		}

		/* Make sure the returned address is consistant with the
		   address modifier.
		 */
		switch (0x30 & am) {
		case 0x00:	/* A32 */
			break;
		case 0x20:	/* A16 */
			data &= 0xFFFF;
			break;
		case 0x30:	/* A24 */
			data &= 0xFFFFFF;
			break;
		default:
			printk(KERN_ERR
			       "VME: My, what a strange BERR AM you have!\n");
			break;
		}
		break;
	case VME_INTERRUPT_MBOX0:
	case VME_INTERRUPT_MBOX1:
	case VME_INTERRUPT_MBOX2:
	case VME_INTERRUPT_MBOX3:
		index = level - VME_INTERRUPT_MBOX0;
		data = readl(universe_base + UNIV_MBOX(index));
		break;
	}

	/* Clear and re-enable the interrupt
	 */
	write_lock_irqsave(&interrupt_rwlock, irqflags);

	writel(1 << level, universe_base + UNIV_LINT_STAT);
	flush = readl(universe_base + UNIV_LINT_STAT);

	writel(readl(universe_base + UNIV_LINT_EN) | (1 << level),
	       universe_base + UNIV_LINT_EN);
	flush = readl(universe_base + UNIV_LINT_EN);

	write_unlock_irqrestore(&interrupt_rwlock, irqflags);

	/* Notify user processes
	 */
	handle = vme_interrupt[level].handles;

	DPRINTF("Responding to interrupt level %ld, vector %#x, data %#x\n",
		level, vector, data);

	while (handle) {
		if (vector == handle->vector) {
			if (VME_INTERRUPT_SIGEVENT == handle->flags) {
				handle->notify.siginfo.si_value.sival_int =
				    data;
				__vme_send_signal(handle, level);
			} else {
				/* This is a blocking call. Update the data in
				   it's handle. We'll wake the process later.
				 */
				handle->notify.data = data;
			}
		}

		handle = handle->next;
	}

	/* Wake all processes blocked waiting for this interrupt
	 */
	index = UNIV_INTERRUPT_INDEX(level, vector);
	wake_up_interruptible_all(&interrupt_wq[index]);
}


/*============================================================================
 * Universe ISR
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
void universe_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
int universe_isr(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
	int intstat, level;
	int handled = 0;

	/* VMIC specific bus error handling. If a bus error is detected, disable
	   the bus error interrupt logic, clear the interrupt, and schedule the
	   bottom half.
	 */
	switch (vmic_reg_type) {
	case VMIVME_PLX:
		if (readl(vmic_base + VMIVMEP_B_INT_STATUS) &
		    VMIVMEP_B_INT_STATUS__BERR_S) {
			writew(readl(vmic_base + VMIVMEP_COMM) &
			       ~VMIVMEP_COMM__BERRIM, vmic_base + VMIVMEP_COMM);
			++vme_interrupt[VME_INTERRUPT_BERR].count;
			tasklet_schedule(&bh_tasklet[VME_INTERRUPT_BERR]);
			handled = 1;
		}
		break;
	case VMIVME_ISA:
		if (readw(vmic_base + VMIVMEI_COMM) & VMIVMEI_COMM__BERRST) {
			writew(readl(vmic_base + VMIVMEI_COMM) &
			       ~VMIVMEI_COMM__BERRI, vmic_base + VMIVMEI_COMM);
			++vme_interrupt[VME_INTERRUPT_BERR].count;
			tasklet_schedule(&bh_tasklet[VME_INTERRUPT_BERR]);
			handled = 1;
		}
		break;
	case VMIVME_FPGA:
		if (readl(vmic_base + VMIVMEF_COMM) & VMIVMEF_COMM__BERRST) {
			writel(readl(vmic_base + VMIVMEF_COMM) &
			       ~VMIVMEF_COMM__BERRI, vmic_base + VMIVMEF_COMM);
			++vme_interrupt[VME_INTERRUPT_BERR].count;
			tasklet_schedule(&bh_tasklet[VME_INTERRUPT_BERR]);
			handled = 1;
		}
		break;
	}

	/* WARNING: Looping to process all pending interrupts is wrong because 
	   when we have reserved interrupts, we do not clear the source, and
	   thus end up in an endless loop here.
	 */
	intstat = readl(universe_base + UNIV_LINT_STAT) &
	    readl(universe_base + UNIV_LINT_EN);
	if (intstat) {
		/* This algorithm returns the enabled and active interrupt
		   level that is in the highest bit position.
		 */
		for (level = UNIV_MAX_IRQ; !(intstat & (1 << level)); --level) ;
		++vme_interrupt[level].count;

		/* If this interrupt level is reserved, we don't do anything.
		   If not reserved and there are handles attached, schedule 
		   the bottom half notification. The DMA, VOWN and BERR
		   interrupts are special cases where the bottom half must be
		   run even if there are no handles. Any other interrupts are
		   simply cleared. -DLH
		 */
		if (vme_interrupt[level].handles &&
		    (VME_INTERRUPT_RESERVE ==
		     vme_interrupt[level].handles->flags)) {
			/* Do nothing for reserved interrupts
			 */
		} else if ((VME_INTERRUPT_DMA == level)
			   || (VME_INTERRUPT_BERR == level)
			   || (VME_INTERRUPT_VOWN == level)
			   || vme_interrupt[level].handles) {
			/* Interrupts processed by the bottom half. The
			   interrupt gets disabled here and will be re-enabled
			   in the bottom half when the processing is completed.
			 */
			writel(readl(universe_base + UNIV_LINT_EN) &
			       ~(1 << level), universe_base + UNIV_LINT_EN);

			tasklet_schedule(&bh_tasklet[level]);
			handled = 1;
		} else {
			/* Clear the interrupt - logically, we should never
			   get here, but just in case....
			 */
			writel(1 << level, universe_base + UNIV_LINT_STAT);
			handled = 1;
		}
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	return (IRQ_RETVAL(handled));
#endif
}


/*============================================================================
 * Associate this process with a VME interrupt level.
 *
 * NOTES: For some interrupts, a data value is returned either by way of the
 * reply parameter for blocking interrupts, or the signal value structure
 * member if POSIX real-time signals are used. In the case of VIRQ1-7, the
 * value is the interrupt vector; in the case of a DMA interrupt, the value is
 * the status of the DGCS register; in the case of BERR, the value is the
 * address of the bus error; in the case of a mailbox interrupt the value is
 * the mailbox data.  Only VME_INTERRUPT_VIRQ, VME_INTERRUPT_MBOX, and
 * VME_INTERRUPT_LM levels may be reserved.
 */
int vme_interrupt_attach(vme_bus_handle_t bus_handle,
			 vme_interrupt_handle_t * handle, int level, int vector,
			 int flags, void *data)
{
	DECLARE_WAITQUEUE(wait, current);
	struct sigevent *event = (struct sigevent *) data;
	int index = UNIV_INTERRUPT_INDEX(level, vector);
	unsigned long irqflags;

	if (NULL == handle) {
		return -EINVAL;
	}

	switch (level) {
	case VME_INTERRUPT_VIRQ1:
	case VME_INTERRUPT_VIRQ2:
	case VME_INTERRUPT_VIRQ3:
	case VME_INTERRUPT_VIRQ4:
	case VME_INTERRUPT_VIRQ5:
	case VME_INTERRUPT_VIRQ6:
	case VME_INTERRUPT_VIRQ7:
		break;
	case VME_INTERRUPT_VOWN:
	case VME_INTERRUPT_DMA:
	case VME_INTERRUPT_LERR:
	case VME_INTERRUPT_BERR:
	case VME_INTERRUPT_SW_IACK:
	case VME_INTERRUPT_SW_INT:
	case VME_INTERRUPT_SYSFAIL:
	case VME_INTERRUPT_ACFAIL:
		/* These interrupt levels may not be reserved
		 */
		if (VME_INTERRUPT_RESERVE == flags)
			return -EPERM;
		/* Intentional fall through */
	case VME_INTERRUPT_MBOX0:
	case VME_INTERRUPT_MBOX1:
	case VME_INTERRUPT_MBOX2:
	case VME_INTERRUPT_MBOX3:
	case VME_INTERRUPT_LM0:
	case VME_INTERRUPT_LM1:
	case VME_INTERRUPT_LM2:
	case VME_INTERRUPT_LM3:
		vector = 0;
		break;
	default:
		DPRINTF("Invalid interrupt level %d\n", level);
		return -EINVAL;
	}

	switch (flags) {
	case VME_INTERRUPT_BLOCKING:
		break;
	case VME_INTERRUPT_SIGEVENT:
		/* Kernel modules cannot use signals
		 */
		if (1 == (int) bus_handle)
			return -EINVAL;
		break;
	case VME_INTERRUPT_RESERVE:
		/* User space code cannot use reserved interrupts
		 */
		if (1 != (int) bus_handle)
			return -EINVAL;
		break;
	default:
		DPRINTF("Invalid interrupt flags %d\n", flags);
		return -EINVAL;
	}

	if (NULL == (*handle = (vme_interrupt_handle_t)
		     kmalloc(sizeof (struct _vme_interrupt_handle),
			     GFP_KERNEL)))
		return -ENOMEM;

	(*handle)->magic = VME_INTERRUPT_MAGIC;
	(*handle)->interrupt = &vme_interrupt[level];
	(*handle)->pid = current->pid;
	(*handle)->id = bus_handle;
	(*handle)->vector = vector;
	(*handle)->flags = flags;

	if (VME_INTERRUPT_SIGEVENT == flags) {
		/* Only signal events are currently supported
		 */
		if (SIGEV_SIGNAL == event->sigev_notify) {
			memset(&((*handle)->notify.siginfo), 0,
			       sizeof (siginfo_t));
			(*handle)->notify.siginfo.si_signo = event->sigev_signo;
			(*handle)->notify.siginfo.si_code = SI_QUEUE;
		} else {
			printk(KERN_ERR
			       "VME: Interrupt event type not supported\n");
			kfree(*handle);
			return -EINVAL;
		}
	}

	write_lock_irqsave(&interrupt_rwlock, irqflags);

	/* VME_INTERRUPT_RESERVE must be exclusive.
	 */
	if ((VME_INTERRUPT_RESERVE == flags) && vme_interrupt[level].handles) {
		write_unlock_irqrestore(&interrupt_rwlock, irqflags);
		kfree(*handle);
		return -EBUSY;
	}

	/* Add the handle to the linked list
	 */
	insert_interrupt_handle(&vme_interrupt[level], *handle);

	if (VME_INTERRUPT_BLOCKING == flags) {
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&interrupt_wq[index], &wait);
	}

	/* Enable the interrupt
	 */
	writel(readl(universe_base + UNIV_LINT_EN) | (1 << level),
	       universe_base + UNIV_LINT_EN);

	write_unlock_irqrestore(&interrupt_rwlock, irqflags);

	DPRINTF("Successfully attached to interrupt\n"
		"     level=%d, vector=%#x flags=%#x, pid=%d\n", level,
		vector, flags, current->pid);

	/* If this is a blocking interrupt, handle it and return it's status
	 */
	if (VME_INTERRUPT_BLOCKING == flags) {
		schedule();
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&interrupt_wq[index], &wait);
		*(int *) data = (*handle)->notify.data;
		vme_interrupt_release(bus_handle, *handle);
	}

	return 0;
}


/*============================================================================
 * Uninstall an interrupt handler.
 */
int vme_interrupt_release(vme_bus_handle_t bus_handle,
			  vme_interrupt_handle_t handle)
{
	unsigned long irqflags;

        if ((NULL == handle) || (VME_INTERRUPT_MAGIC != handle->magic)) {
                return -EINVAL;
        }

	write_lock_irqsave(&interrupt_rwlock, irqflags);

	remove_interrupt_handle(handle);

	write_unlock_irqrestore(&interrupt_rwlock, irqflags);

	kfree(handle);

	return 0;
}


/*============================================================================
 * Generate a VMEbus interrupt.
 */
int vme_interrupt_generate(vme_bus_handle_t bus_handle, int level, int vector)
{
	/* We only allow generation of software interrupts
	 */
	if ((VME_INTERRUPT_VIRQ1 > level) || (VME_INTERRUPT_VIRQ7 < level))
		return -EINVAL;

	/* Generated software interrupts must be on even vectors
	 */
	if (1 & vector)
		return -EINVAL;

	write_lock(&interrupt_rwlock);

	/* Load the vector
	 */
	writel(vector << 24, universe_base + UNIV_STATID);

	/* Generate the interrupt by clearing it's enable bit then reaserting
	   the enable. The Universe chip will clear the interrupt status
	   (software interrupts are ROAK).
	 */
	writel(readl(universe_base + UNIV_VINT_EN) & ~(1 << (24 + level)),
	       universe_base + UNIV_VINT_EN);
	writel(readl(universe_base + UNIV_VINT_EN) | (1 << (24 + level)),
	       universe_base + UNIV_VINT_EN);

	++interrupts_generated[level - VME_INTERRUPT_VIRQ1];

	write_unlock(&interrupt_rwlock);

	return 0;
}


/*============================================================================
 * Get the irq assigend to the VMEbus bridge
 */
int vme_interrupt_irq(vme_bus_handle_t bus_handle, int *irq)
{
	if (!universe_pci_dev)
		return -EPERM;

	*irq = universe_pci_dev->irq;

	return 0;
}


/*============================================================================
 * Enable a VMEbus interrupt.
 *
 * NOTES: The handle used with this function must have been allocated with the
 * flag VME_INTERRUPT_RESERVE.
 */
int vme_interrupt_enable(vme_bus_handle_t bus_handle,
			 vme_interrupt_handle_t handle)
{
	unsigned long irqflags;

        if ((NULL == handle) || (VME_INTERRUPT_MAGIC != handle->magic) ||
		(VME_INTERRUPT_RESERVE != handle->flags)) {
                return -EINVAL;
        }

	write_lock_irqsave(&interrupt_rwlock, irqflags);

	writel(readl(universe_base + UNIV_LINT_EN) |
	       (1 << handle->interrupt->level), universe_base + UNIV_LINT_EN);

	write_unlock_irqrestore(&interrupt_rwlock, irqflags);

	return 0;
}


/*============================================================================
 * Disable a VMEbus interrupt.
 *
 * NOTES: The handle used with this function must have been allocated with the
 * flag VME_INTERRUPT_RESERVE.
 */
int vme_interrupt_disable(vme_bus_handle_t bus_handle,
			  vme_interrupt_handle_t handle)
{
	unsigned long irqflags;

        if ((NULL == handle) || (VME_INTERRUPT_MAGIC != handle->magic) ||
		(VME_INTERRUPT_RESERVE != handle->flags)) {
                return -EINVAL;
        }

	write_lock_irqsave(&interrupt_rwlock, irqflags);

	writel(readl(universe_base + UNIV_LINT_EN) &
	       ~(1 << handle->interrupt->level), universe_base + UNIV_LINT_EN);

	write_unlock_irqrestore(&interrupt_rwlock, irqflags);

	return 0;
}


/*============================================================================
 * Clear a VMEbus interrupt.
 *
 * NOTES: The handle used with this function must have been allocated with the
 * flag VME_INTERRUPT_RESERVE.
 */
int vme_interrupt_clear(vme_bus_handle_t bus_handle,
			vme_interrupt_handle_t handle)
{
	unsigned long irqflags;

        if ((NULL == handle) || (VME_INTERRUPT_MAGIC != handle->magic) ||
		(VME_INTERRUPT_RESERVE != handle->flags)) {
                return -EINVAL;
        }

	write_lock_irqsave(&interrupt_rwlock, irqflags);

	writel(1 << handle->interrupt->level, universe_base + UNIV_LINT_STAT);

	write_unlock_irqrestore(&interrupt_rwlock, irqflags);

	return 0;
}


/*============================================================================
 * Test to see if an interrupt level is asserted.
 *
 * NOTES: The handle used with this function must have been allocated with the
 * flag VME_INTERRUPT_RESERVE.
 */
int vme_interrupt_asserted(vme_bus_handle_t bus_handle,
			   vme_interrupt_handle_t handle)
{
	int asserted;
	unsigned long irqflags;

        if ((NULL == handle) || (VME_INTERRUPT_MAGIC != handle->magic) ||
		(VME_INTERRUPT_RESERVE != handle->flags)) {
                return -EINVAL;
        }

	read_lock_irqsave(&interrupt_rwlock, irqflags);

	asserted = readl(universe_base + UNIV_LINT_STAT) &
	    readl(universe_base + UNIV_LINT_EN) &
	    (1 << handle->interrupt->level);

	read_unlock_irqrestore(&interrupt_rwlock, irqflags);

	return asserted;
}


/*============================================================================
 * VME interrupt vector
 *
 * NOTES: The handle used with this function must have been allocated with the
 * flag VME_INTERRUPT_RESERVE.
 */
int vme_interrupt_vector(vme_bus_handle_t bus_handle,
			 vme_interrupt_handle_t handle, int *vector)
{
	int level = handle->interrupt->level;
	unsigned long irqflags;

        if ((NULL == handle) || (VME_INTERRUPT_MAGIC != handle->magic) ||
		(VME_INTERRUPT_RESERVE != handle->flags) || 
		(VME_INTERRUPT_VIRQ1 > level) || 
		(VME_INTERRUPT_VIRQ7 < level) ||
		(NULL == vector)) {
		return -EINVAL;
	}

	read_lock_irqsave(&interrupt_rwlock, irqflags);

	*vector = readl(universe_base + UNIV_V_STATID(level));

	read_unlock_irqrestore(&interrupt_rwlock, irqflags);

	return (UNIV_V_STATID__ERR & *vector ? -EIO : 0);
}


/*============================================================================
 * Initialize the VMEbus interrupts
 */
int interrupt_init(void)
{
	int ii;

	/* The DMA, VOWN and BERR interrupts should always be enabled
	 */
	writel(UNIV_LINT__DMA | UNIV_LINT__VOWN | UNIV_LINT__VERR,
	       universe_base + UNIV_LINT_EN);

	for (ii = 0; ii < UNIV_IRQS; ++ii)
		init_waitqueue_head(&interrupt_wq[ii]);

	for (ii = 0; ii <= UNIV_MAX_IRQ; ++ii) {
		memset(&vme_interrupt[ii], 0, sizeof (struct __vme_interrupt));
		vme_interrupt[ii].level = ii;
		vme_interrupt[ii].count = 0;
		vme_interrupt[ii].handles = NULL;
		tasklet_init(&bh_tasklet[ii], universe_isr_bh, ii);
	}

	interrupt_proc_entry = create_proc_entry("vme/interrupt",
						 S_IRUGO | S_IWUSR, NULL);
	if (!interrupt_proc_entry) {
		printk(KERN_WARNING
		       "VME: Failed to register interrupt proc page\n");
		/* Not a fatal error */
	} else {
		interrupt_proc_entry->read_proc = read_interrupt_proc_page;
	}

	return 0;
}


/*============================================================================
 * Cleanup the VMEbus interrupts for exit
 */
int interrupt_term(void)
{
	int ii;

	remove_proc_entry("vme/interrupt", NULL);

	for (ii = 0; ii < UNIV_MAX_IRQ; ++ii) {
		while (vme_interrupt[ii].handles) {
			vme_interrupt_release(NULL, vme_interrupt[ii].handles);
		}
	}

	return 0;
}
