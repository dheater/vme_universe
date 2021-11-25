#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>


#define LEVEL             VME_INTERRUPT_LM2
#define VME_ADDRESS       0x90000000
#define ADDRESS_SPACE     VME_A32


int main()
{
        vme_bus_handle_t bus_handle;
        vme_lm_handle_t lm_handle;
        vme_interrupt_handle_t interrupt_handle;
        int data;

        if (vme_init(&bus_handle)) {
                perror("Error initializing the VMEbus");
                return -1;
        }
        if (vme_location_monitor_create(bus_handle, &lm_handle, VME_ADDRESS,
                                        ADDRESS_SPACE, 0, 0)) {
                perror("Error creating VMEbus location monitor image");
                vme_term(bus_handle);
                return -1;
        }

        if (vme_interrupt_attach(bus_handle, &interrupt_handle, LEVEL, 0,
                                 VME_INTERRUPT_BLOCKING, &data)) {
                perror("Error attaching to the interrupt");
                vme_location_monitor_release(bus_handle, lm_handle);
                vme_term(bus_handle);
                return -1;
        }

        printf("Location monitor interrupt\n");
        if (vme_location_monitor_release(bus_handle, lm_handle)) {
                perror("Error releasing VMEbus location monitor image");
                vme_term(bus_handle);
                return -1;
        }

        if (vme_term(bus_handle)) {
                perror("Error terminating");
                return -1;
        }

        return 0;
}

