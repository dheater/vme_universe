// Here is a simple piece of code to try compiling and running.
// We'll call it hello_vme.c

#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>

int main()
{
        vme_bus_handle_t bus_handle;

        if (vme_init(&bus_handle)) {
                perror("Cannot initialize the VMEbus");
                return -1;
        }
	printf("Bus handle value %d\n", (int)bus_handle);
        printf("We're ready to start accessing the bus now!\n");

        vme_term(bus_handle);
        return 0;
}

