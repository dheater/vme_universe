
#include <vme/vme.h>
#include <vme/vme_api.h>
#include <signal.h>
#include <stdio.h>

#define LEVEL             VME_INTERRUPT_VIRQ3
#define VECTOR            0x10


int main()
{
	vme_bus_handle_t bus_handle;

	if (vme_init(&bus_handle)) {
		perror("Error initializing the VMEbus");
		return -1;
	}

	if (vme_interrupt_generate(bus_handle, LEVEL, VECTOR)) {
		perror("Error generating the interrupt");
		vme_term(bus_handle);
		return -1;
	}

	if (vme_term(bus_handle)) {
		perror("Error terminating");
		return -1;
	}

	return 0;
}
