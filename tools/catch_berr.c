#include <vme/vme.h>
#include <vme/vme_api.h>
#include <signal.h>
#include <stdio.h>

#define LEVEL             VME_INTERRUPT_BERR
#define VECTOR            0x0


int main()
{
        vme_bus_handle_t bus_handle;
        vme_interrupt_handle_t interrupt_handle;
        int data;

        if (vme_init(&bus_handle)) {
                perror("Error initializing the VMEbus");
                return -1;
        }

        if (vme_interrupt_attach(bus_handle, &interrupt_handle, LEVEL,
                                 VECTOR, VME_INTERRUPT_BLOCKING, &data)) {
                perror("Error attaching to the interrupt");
                vme_term(bus_handle);
                return -1;
        }

        /* For VMEbus interrupts, the returned data is (level << 8) & vector */
        printf("VMEbus interrupt occured on level %d, vector 0x%x\n",
               data >> 8, data & 0xff);

        if (vme_term(bus_handle)) {
                perror("Error terminating");
                return -1;
        }

        return 0;
}
