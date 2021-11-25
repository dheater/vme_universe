
#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>


#define LEVEL             VME_INTERRUPT_MBOX3
#define VME_ADDRESS       0x0
#define ADDRESS_SPACE     VME_A16


int main()
{
	vme_bus_handle_t bus_handle;
	vme_vrai_handle_t vrai_handle;
	vme_interrupt_handle_t interrupt_handle;
	int data;

	if (vme_init(&bus_handle)) {
		perror("Error initializing the VMEbus");
		return -1;
	}
	if (vme_register_image_create(bus_handle, &vrai_handle, VME_ADDRESS,
					  ADDRESS_SPACE, 0)) {
		perror("Error creating VMEbus register image");
		vme_term(bus_handle);
		return -1;
	}

	if (vme_interrupt_attach(bus_handle, &interrupt_handle, LEVEL,
				     0, VME_INTERRUPT_BLOCKING, &data)) {
		perror("Error attaching to the interrupt");
		vme_register_image_release(bus_handle, vrai_handle);
		vme_term(bus_handle);
		return -1;
	}

	printf("Mailbox interrupt occurred, data is 0x%x\n", data);

	if (vme_register_image_release(bus_handle, vrai_handle)) {
		perror("Error releasing VMEbus register image");
		vme_term(bus_handle);
		return -1;
	}

	if (vme_term(bus_handle)) {
		perror("Error terminating");
		return -1;
	}

	return 0;
}
