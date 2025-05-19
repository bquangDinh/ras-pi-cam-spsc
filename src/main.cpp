#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "headers/SPSCQueue.h"

#include <iostream>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <array>
#include <csignal>

using namespace rigtorp;
using namespace cv;
using namespace std;

// #define DISABLE_LOGGING // Uncomment to disable logging

#define LOG(str) cout << str << endl
#define LOG_ERR(str) cerr << str << endl

#ifdef DISABLE_LOGGING
#define LOG(str)
#define LOG_ERR(str)
#endif

constexpr int QUEUE_SIZE = 10;                     // Size of the queue
constexpr int FRAME_WIDTH = 940;                  // Width of the camera frame
constexpr int FRAME_HEIGHT = 720;                  // Height of the camera frame
constexpr const char *WINDOW_NAME = "Camera Feed"; // Name of the window

array<Mat, QUEUE_SIZE> frames; // An array to hold frames pool

// Single Producer Single Consumer Queue that holds frame indices
SPSCQueue<int> queue(QUEUE_SIZE);

// Flag to control the running state of the program
std::atomic<bool> running(true);

inline int nextFrameIndex(int current)
{
    return (current + 1) % QUEUE_SIZE; // Circular buffer
}

/**
 * @brief Function to capture frames from the camera and push them to the queue
 */
void frameCaptureThread(VideoCapture &cap)
{
    LOG("Frame capture thread started.");

    int currentFrameIndex = 0; // Index of the current frame

    while (running)
    {
        Mat &frame = frames[currentFrameIndex]; // Get the current frame

        cap >> frame; // Capture a new frame

        if (frame.empty())
        {
            LOG_ERR("Empty frame captured, skipping...");
            continue; // Skip if the frame is empty
        }

        queue.push(currentFrameIndex); // Push the frame index to the queue

        currentFrameIndex = nextFrameIndex(currentFrameIndex); // Update the frame index
    }

    LOG("Frame capture thread finished.");
}

void signalHandler(int signum)
{
    LOG("Signal received: " << signum);
    running = false; // Set the running flag to false
}

int main()
{
    VideoCapture cap(0, CAP_V4L2); // Open the default camera

    if (!cap.isOpened())
    {
        cerr << "Error: Could not open camera." << endl;
        return -1;
    }

    // Set up interrupt signal handler
    signal(SIGINT, signalHandler); // Handle Ctrl+C

    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));

    cap.set(CAP_PROP_FRAME_WIDTH, FRAME_WIDTH); // Set frame width

    cap.set(CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT); // Set frame height

    cap.set(CAP_PROP_FPS, 30); // Set frame rate

    namedWindow(WINDOW_NAME, WINDOW_NORMAL); // Create a window

    resizeWindow(WINDOW_NAME, FRAME_WIDTH, FRAME_HEIGHT);

    setWindowProperty(WINDOW_NAME, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);

    // Start the frame capture thread
    thread captureThread(frameCaptureThread, ref(cap));

    double fps = 0.0f;

    auto lastFrameTime = chrono::high_resolution_clock::now(); // Get the current time

    int frameIndex;
    int frameCount = 0;

    while (running)
    {
        while (!queue.front()); // Wait for a frame index to be available

        frameCount++; // Increment the frame count

        auto currentFrameTime = chrono::high_resolution_clock::now(); // Get the current time

        auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(currentFrameTime - lastFrameTime).count(); // Calculate elapsed time

        if (elapsedTime >= 1000)
        {
            fps = (frameCount * 1000.0) / elapsedTime; // Calculate FPS
            frameCount = 0; // Reset the frame count
            lastFrameTime = currentFrameTime; // Update the last frame time
        }

        frameIndex = *queue.front(); // Get the front index

        queue.pop(); // Remove the index from the queue

        if (frameIndex < 0 || frameIndex >= QUEUE_SIZE)
        {
            continue; // Skip if the index is out of bounds
        }

        Mat &frame = frames[frameIndex]; // Get the frame using the index

        // Draw the FPS on the frame
        putText(frame, "FPS: " + to_string(static_cast<int>(fps)), Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);

        imshow(WINDOW_NAME, frame); // Display the frame

        //TODO: Control this using Raspberry Pi GPIO
        if (waitKey(1) >= 0)
        {
            running = false; // Stop if a key is pressed
        }
    }

    captureThread.join(); // Wait for the capture thread to finish

    cap.release(); // Release the camera

    destroyAllWindows(); // Close all OpenCV windows

    return 0;
}