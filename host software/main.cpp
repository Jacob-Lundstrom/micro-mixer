#include <stdio.h>
#include <libusb.h>


int main(int argc, char** argv)
{
    int r;
    ssize_t cnt;
    libusb_device_handle* dev_handle;
    libusb_context* ctx = NULL;
    unsigned char buf[64];
    int transferred;
    const int endpoint = 0x01;
    const int vendor_id = 0x04D8;
    const int product_id = 0x003F;

    // Initialize libusb
    r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "Error initializing libusb: %s\n", libusb_error_name(r));
        return 1;
    }

    // Open the device
    dev_handle = libusb_open_device_with_vid_pid(ctx, vendor_id, product_id);
    if (dev_handle == NULL) {
        fprintf(stderr, "Error opening device\n");
        libusb_exit(ctx);
        return 1;
    }

    // Claim the interface
    r = libusb_claim_interface(dev_handle, 0);
    if (r < 0) {
        fprintf(stderr, "Error claiming interface: %s\n", libusb_error_name(r));
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return 1;
    }

    // Read data from the endpoint
    r = libusb_bulk_transfer(dev_handle, endpoint, buf, sizeof(buf), &transferred, 1000);
    if (r < 0) {
        fprintf(stderr, "Error reading from endpoint: %s\n", libusb_error_name(r));
        libusb_release_interface(dev_handle, 0);
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return 1;
    }

    // Print the data
    printf("Read %d bytes from endpoint:\n", transferred);
    for (int i = 0; i < transferred; i++) {
        printf("%02x ", buf[i]);
    }
    printf("\n");

    // Release the interface and close the device
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(ctx);

    return 0;
}
