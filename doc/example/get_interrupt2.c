
#include <vme/vme.h>
#include <vme/vme_api.h>
#include <signal.h>
#include <stdio.h>

#define LEVEL             VME_INTERRUPT_VIRQ3
#define VECTOR            0x10

int done = 0;


void handler(int sig, siginfo_t * siginfo, void *extra)
{
	/* For VMEbus interrupts, the returned data is (level << 8) & vector */
	printf("VMEbus interrupt occured on level %d, vector 0x%x\n",
	       siginfo->si_value.sival_int >> 8,
	       siginfo->si_value.sival_int & 0xff);

	done = 1;
}


int main()
{
	vme_bus_handle_t bus_handle;
	vme_interrupt_handle_t interrupt_handle;
	struct sigevent event;
	struct sigaction action;

	if (vme_init(&bus_handle)) {
		perror("Error initializing the VMEbus");
		return -1;
	}

	/* Set up sigevent struct */
	event.sigev_signo = SIGIO;
	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_value.sival_int = 0;

	/* Install the signal handler */
	sigemptyset(&action.sa_mask);
	action.sa_sigaction = handler;
	action.sa_flags = SA_SIGINFO;

	if (sigaction(SIGIO, &action, NULL)) {
		perror("Error installing signal handler");
		vme_term(bus_handle);
		return -1;
	}

	if (vme_interrupt_attach(bus_handle, &interrupt_handle, LEVEL,
				 VECTOR, VME_INTERRUPT_SIGEVENT, &event)) {
		perror("Error attaching to the interrupt");
		vme_term(bus_handle);
		return -1;
	}

	/* Loop here until we get the signal */
	while (!done) ;

	if (vme_interrupt_release(bus_handle, interrupt_handle)) {
		perror("Error releasing the interrupt");
		vme_term(bus_handle);
		return -1;
	}

	if (vme_term(bus_handle)) {
		perror("Error terminating");
		return -1;
	}

	return 0;
}
