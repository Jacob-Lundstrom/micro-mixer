#include <stdio.h>
#include <libusb-1.0/libusb.h>

#define outEndpoint 0x01
#define inEndpoint 0x81

uint16_t VID = 0x04D8;
uint16_t PID = 0x003F;

libusb_context* ctx;
libusb_device* dev;
libusb_device_handle* dev_handle;
libusb_device_descriptor dev_descriptor;
int verbosity_level = 3;
int e;

unsigned char outDataBuffer[64];
unsigned char inDataBuffer[64];

int mixerUSBInit() {
    // Libusb initialization
    e = libusb_init(&ctx);
    if (e != 0) {
        fprintf(stderr, "Error initializing libusb: \n%s", libusb_error_name(e));
        libusb_exit(ctx);
        return -1;
    }

    libusb_set_debug(ctx, verbosity_level);


    // Handling and device enumeration
    dev_handle = libusb_open_device_with_vid_pid(ctx, VID, PID);
    if (dev_handle == NULL) {
        libusb_exit(ctx);
        return -1;
    }
    dev = libusb_get_device(dev_handle);

    int config;
    e = libusb_get_configuration(dev_handle, &config);
    if (e != 0)
    {
        printf("\nError in libusb_get_configuration\n");
        libusb_exit(ctx);
        libusb_close(dev_handle);
        return -1;
    }

    if (config != 1)
    {
        libusb_set_configuration(dev_handle, 1);
        if (e != 0)
        {
            printf("Error in libusb_set_configuration\n");
            libusb_exit(ctx);
            libusb_close(dev_handle);
            return -1;
        }
    }


    e = libusb_claim_interface(dev_handle, 0);
    if (e != 0) {
        fprintf(stderr, "Error claiming device interface: \n%s", libusb_error_name(e));
        libusb_exit(ctx);
        return -1;
    }

    // Get device Info
    e = libusb_get_device_descriptor(dev, &dev_descriptor);
    if (e != 0) {
        fprintf(stderr, "Error getting descriptor: \n%s", libusb_error_name(e));
        libusb_exit(ctx);
        return -1;
    }

    return 0;
}

unsigned char* mixerUSBReadAllData() {

    outDataBuffer[0] = 0x80; // Code for request all data
    e = libusb_interrupt_transfer(dev_handle, outEndpoint, outDataBuffer, sizeof(outDataBuffer), nullptr, 10000);
    if (e != 0) {
        fprintf(stderr, "Error during bulk transfer out: %s", libusb_error_name(e));
        return nullptr;
    }

    e = libusb_interrupt_transfer(dev_handle, inEndpoint, inDataBuffer, sizeof(inDataBuffer), nullptr, 1000000);
    if (e != 0) {
        fprintf(stderr, "Error during bulk transfer in: %s", libusb_error_name(e));
        return nullptr;
    }

    if (verbosity_level >= 3) {
        system("CLS");
        for (int i = 0; i < sizeof(inDataBuffer); i++) {
            printf("%02X", inDataBuffer[i]);
            printf(" ");
        }

        printf("\n\n");
    }

    return &inDataBuffer[0]; // should conain all of the recieved data
}

int mixerUSBDeInit() {
    // Close connections
    e = libusb_release_interface(dev_handle, 0);
    if (e != 0) {
        fprintf(stderr, "Error releasing interface: \n%s", libusb_error_name(e));
        return -1;
    }
    else {
        printf("Interface successfully released\n");
    }

    // Deinitialize Libusb
    libusb_exit(ctx);

    return 0;
}