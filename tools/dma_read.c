#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>

//#define VME_ADDRESS       0x08000000
#define VME_ADDRESS       0x88040000
#define ADDRESS_MODIFIER  VME_A32SD
#define NBYTES            0x40

int main()
{
        vme_bus_handle_t bus_handle;
        vme_dma_handle_t dma_handle;
        uint8_t *ptr;
        int ii;

        if (vme_init(&bus_handle)) {
                perror("Error initializing the VMEbus");
                return -1;
        }

        if (vme_dma_buffer_create(bus_handle, &dma_handle, NBYTES, 0, NULL)) {
                perror("Error creating the buffer");
                vme_term(bus_handle);
                return -1;
        }

        ptr = vme_dma_buffer_map(bus_handle, dma_handle, 0);
        if (!ptr) {
                perror("Error mapping the buffer");
                vme_dma_buffer_release(bus_handle, dma_handle);
                vme_term(bus_handle);
                return -1;
        }

        /* Transfer the data */
        if (vme_dma_read(bus_handle, dma_handle, 0, VME_ADDRESS,
                         ADDRESS_MODIFIER, NBYTES, 0)) {
                perror("Error reading data");
                vme_dma_buffer_unmap(bus_handle, dma_handle);
                vme_dma_buffer_release(bus_handle, dma_handle);
                vme_term(bus_handle);
                return -1;
        }

        /* Print the VMEbus data */
        for (ii = 0; ii < NBYTES; ++ii, ++ptr) {
                printf("%.2x ", *ptr);
                /* Add a newline every 16 bytes */
                if (!((ii + 1) % 0x10))
                        printf("\n");
        }
        printf("\n");

        if (vme_dma_buffer_unmap(bus_handle, dma_handle)) {
                perror("Error unmapping the buffer");
                vme_dma_buffer_release(bus_handle, dma_handle);
                vme_term(bus_handle);
                return -1;
        }

        if (vme_dma_buffer_release(bus_handle, dma_handle)) {
                perror("Error releasing the buffer");
                vme_term(bus_handle);
                return -1;
        }

        if (vme_term(bus_handle)) {
                perror("Error terminating");
                return -1;
        }

        return 0;
}

