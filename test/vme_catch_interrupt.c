
/*
===============================================================================
                            COPYRIGHT NOTICE

    Copyright (C) 2001-2003 GE Fanuc
    International Copyright Secured.  All Rights Reserved.

-------------------------------------------------------------------------------
    Description: VMEbus interrupt handler example
-------------------------------------------------------------------------------

===============================================================================
*/


#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/universe.h>
#include <vme/vme.h>
#include <vme/vme_api.h>
#include "vme_test.h"


static volatile int level;	/* This must be declared globally so the signal
				   handler can use it */
static volatile int done = 0;	/* Let us know that an exit has been
				   requested */


/* Print message for the interrupt
 */
void print_intr_msg(int lvl, unsigned int data)
{

	switch (lvl) {
	case VME_INTERRUPT_VOWN:
		fprintf(stderr, "Vown interrupt\n");
		break;
	case VME_INTERRUPT_VIRQ1:
	case VME_INTERRUPT_VIRQ2:
	case VME_INTERRUPT_VIRQ3:
	case VME_INTERRUPT_VIRQ4:
	case VME_INTERRUPT_VIRQ5:
	case VME_INTERRUPT_VIRQ6:
	case VME_INTERRUPT_VIRQ7:
		fprintf(stderr, "VME Interrupt: level = %d, vector = 0x%x\n",
			lvl, data & 0xff);
		break;
	case VME_INTERRUPT_DMA:
		fprintf(stderr, "DMA interrupt: data = 0x%x\n", data);
		break;
	case VME_INTERRUPT_LERR:
		fprintf(stderr, "PCI BERR interrupt\n");
		break;
	case VME_INTERRUPT_BERR:
		fprintf(stderr, "BERR: address = 0x%x\n", data);
		break;
	case VME_INTERRUPT_SW_IACK:
		fprintf(stderr, "Interrupt acknowledge interrupt\n");
		break;
	case VME_INTERRUPT_SW_INT:
		fprintf(stderr, "Software interrupt\n");
		break;
	case VME_INTERRUPT_SYSFAIL:
		fprintf(stderr, "System fail interrupt\n");
		break;
	case VME_INTERRUPT_ACFAIL:
		fprintf(stderr, "AC failure interrupt\n");
		break;
	case VME_INTERRUPT_MBOX0:
	case VME_INTERRUPT_MBOX1:
	case VME_INTERRUPT_MBOX2:
	case VME_INTERRUPT_MBOX3:
		fprintf(stderr, "Mailbox interrupt level %d: data = 0x%x\n",
			lvl - VME_INTERRUPT_MBOX0, data);
		break;
	case VME_INTERRUPT_LM0:
	case VME_INTERRUPT_LM1:
	case VME_INTERRUPT_LM2:
	case VME_INTERRUPT_LM3:
		fprintf(stderr, "Location monitor interrupt level %d\n",
			lvl - VME_INTERRUPT_LM0);
		break;
	}
}


/* Handler for interrupts.  We are anticipating the use of real-time
   signals here.
 */
static void handler(int sig, siginfo_t * siginfo, void *extra)
{
	print_intr_msg(level, siginfo->si_value.sival_int);
}


/* Signal handler to trap termination
 */
static void ctlc_handler(int sig)
{
	done = 1;
}


int main(int argc, char **argv)
{
	vme_bus_handle_t bus_handle;
	vme_interrupt_handle_t handle;
	struct sigevent event;
	struct sigaction handler_act;
	int vector, intstat, flags, c;

	/* Default values
	 */
	level = VME_INTERRUPT_VIRQ1;
	flags = VME_INTERRUPT_BLOCKING;
	vector = 0;

	/* Parse the argument list
	 */
	while (-1 != (c = getopt(argc, argv, "l:f:v:")))
		switch (c) {
		case 'l':	/* Interrupt level */
			if (strtolvl(optarg, (int *) &level)) {
				fprintf(stderr, "Invalid interrupt level\n");
				return -1;
			}
			break;
		case 'f':	/* Interrupt flags */
			if (strtoflags(optarg, &flags)) {
				fprintf(stderr, "Invalid flags\n");
				return -1;
			}
			break;
		case 'v':	/* Interrupt vector */
			vector = strtol(optarg, NULL, 0);
			if (vector > 0xff) {
				fprintf(stderr, "Invalid vector");
				return -1;
			}
			break;
		default:
			fprintf(stderr, "USAGE: vme_catch_interrupt [-f flags] "
				"[-l level] [-v vector]");
			return -1;
		}

	/* Initialize
	   WARNING: If you call signal before calling vme_init, vme_init()
	   may fail!
	 */
	if (vme_init(&bus_handle)) {
		perror("vme_init");
		return -1;
	}

	/* Trap terminate signal so we can exit cleanly.
	 */
	signal(SIGINT, ctlc_handler);
	signal(SIGTERM, ctlc_handler);

	/* Process the interrupt according to the flags
	 */
	if (VME_INTERRUPT_SIGEVENT == flags) {	/* VME_INTERRUPT_SIGEVENT */
		/* Set up sigevent struct
		 */
		event.sigev_signo = SIGIO;
		event.sigev_notify = SIGEV_SIGNAL;
		event.sigev_value.sival_int = 0;

		/* Install the signal handler */
		sigemptyset(&handler_act.sa_mask);
		handler_act.sa_sigaction = handler;
		handler_act.sa_flags = SA_SIGINFO;

		if (sigaction(SIGIO, &handler_act, NULL)) {
			perror("sigaction");
			vme_term(bus_handle);
			return -1;
		}

		/* Attach the signal to the interrupt
		 */
		if (vme_interrupt_attach(bus_handle, &handle, level, vector,
					 flags, &event)) {
			perror("vme_interrupt_attach");
			vme_term(bus_handle);
			return -1;
		}

		/* Loop until we get a terminate signal
		 */
		while (!done) ;

		/* Detach from the interrupt
		 */
		if (vme_interrupt_release(bus_handle, handle)) {
			perror("vme_interrupt_release");
			vme_term(bus_handle);
			return -1;
		}
	} else {		/* VME_INTERRUPT_BLOCKING */

		/* Attach the signal to the interrupt, this call will block
		   until the interrupt is received
		 */
		if (vme_interrupt_attach(bus_handle, &handle, level, vector,
					 flags, &intstat)) {
			/* If done is set, then someone sent us a kill signal,
			   and we will exit without printing an error message
			 */
			if (!done)
				perror("vme_interrupt_attach");
			vme_term(bus_handle);
			return -1;
		}

		/* Print the interrupt message
		 */
		print_intr_msg(level, intstat);
	}

	if (vme_term(bus_handle)) {
		perror("vme_term");
		return -1;
	}

	return 0;
}
