#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE_LEFT_PATH   "/dev/car/left_ir"
#define DEVICE_RIGHT_PATH  "/dev/car/right_ir"

int main() {
    int fd_left, fd_right;
    char read_buffer[128];
    ssize_t bytes_read;
    char write_data[] = "Test Data for Left IR";

    // Left IR Device Open
    fd_left = open(DEVICE_LEFT_PATH, O_RDWR);
    if (fd_left < 0) {
        perror("Failed to open left IR device");
        return -1;
    }

    // Right IR Device Open
    fd_right = open(DEVICE_RIGHT_PATH, O_RDWR);
    if (fd_right < 0) {
        perror("Failed to open right IR device");
        close(fd_left);
        return -1;
    }

    // Write to Left IR Device
    printf("Writing to Left IR Device: %s\n", write_data);
    ssize_t bytes_written = write(fd_left, write_data, strlen(write_data));
    if (bytes_written < 0) {
        perror("Failed to write to left IR device");
        close(fd_left);
        close(fd_right);
        return -1;
    }
    printf("Written %zd bytes to Left IR device\n", bytes_written);

    // Read from Left IR Device
    printf("Reading from Left IR Device\n");
    bytes_read = read(fd_left, read_buffer, sizeof(read_buffer) - 1);
    if (bytes_read < 0) {
        perror("Failed to read from left IR device");
        close(fd_left);
        close(fd_right);
        return -1;
    }
    read_buffer[bytes_read] = '\0';  // Null-terminate the read data
    printf("Read from Left IR Device: %s\n", read_buffer);

    // Read from Right IR Device
    printf("Reading from Right IR Device\n");
    bytes_read = read(fd_right, read_buffer, sizeof(read_buffer) - 1);
    if (bytes_read < 0) {
        perror("Failed to read from right IR device");
        close(fd_left);
        close(fd_right);
        return -1;
    }
    read_buffer[bytes_read] = '\0';  // Null-terminate the read data
    printf("Read from Right IR Device: %s\n", read_buffer);

    // Close the devices
    close(fd_left);
    close(fd_right);

    return 0;
}

