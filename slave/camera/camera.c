#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

#define VIDEO_DEVICE "/dev/video0"
#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define IMAGE_FILE "captured_image.jpg"

int main() {
    int video_fd;
    struct v4l2_capability cap;
    struct v4l2_format format;
    struct v4l2_requestbuffers reqbuf;
    struct v4l2_buffer buffer;
    FILE *image_file;

    // Open the video device
    video_fd = open(VIDEO_DEVICE, O_RDWR);
    if (video_fd == -1) {
        perror("Failed to open video device");
        return EXIT_FAILURE;
    }

    // Check if the device is a video capture device
    if (ioctl(video_fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("Failed to query video device");
        close(video_fd);
        return EXIT_FAILURE;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "Video capture not supported\n");
        close(video_fd);
        return EXIT_FAILURE;
    }

    // Set the image format
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(video_fd, VIDIOC_G_FMT, &format) == -1) {
        perror("Failed to get video format");
        close(video_fd);
        return EXIT_FAILURE;
    }

    format.fmt.pix.width = IMAGE_WIDTH;
    format.fmt.pix.height = IMAGE_HEIGHT;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG; // Use JPEG format
    if (ioctl(video_fd, VIDIOC_S_FMT, &format) == -1) {
        perror("Failed to set video format");
        close(video_fd);
        return EXIT_FAILURE;
    }

    // Request a single buffer for capturing
    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    reqbuf.count = 1;
    if (ioctl(video_fd, VIDIOC_REQBUFS, &reqbuf) == -1) {
        perror("Failed to request buffer");
        close(video_fd);
        return EXIT_FAILURE;
    }

    // Capture an image
    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = 0;
    if (ioctl(video_fd, VIDIOC_QUERYBUF, &buffer) == -1) {
        perror("Failed to query buffer");
        close(video_fd);
        return EXIT_FAILURE;
    }

    void *buffer_start = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, video_fd, buffer.m.offset);
    if (buffer_start == MAP_FAILED) {
        perror("Failed to mmap");
        close(video_fd);
        return EXIT_FAILURE;
    }

    if (ioctl(video_fd, VIDIOC_QBUF, &buffer) == -1) {
        perror("Failed to queue buffer");
        close(video_fd);
        return EXIT_FAILURE;
    }

    if (ioctl(video_fd, VIDIOC_STREAMON, &buffer.type) == -1) {
        perror("Failed to start capture");
        close(video_fd);
        return EXIT_FAILURE;
    }

    // Wait for the capture to complete
    if (ioctl(video_fd, VIDIOC_DQBUF, &buffer) == -1) {
        perror("Failed to dequeue buffer");
        close(video_fd);
        return EXIT_FAILURE;
    }

    // Create and open the image file
    image_file = fopen(IMAGE_FILE, "wb");
    if (image_file == NULL) {
        perror("Failed to open image file");
        close(video_fd);
        return EXIT_FAILURE;
    }

    // Write the captured image data to the file
    fwrite(buffer_start, 1, buffer.length, image_file);
    fclose(image_file);

    // Stop streaming and clean up
    ioctl(video_fd, VIDIOC_STREAMOFF, &buffer.type);
    munmap(buffer_start, buffer.length);
    close(video_fd);

    printf("Image saved to %s\n", IMAGE_FILE);

    return EXIT_SUCCESS;
}
