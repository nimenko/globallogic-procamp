#include <iostream>
#include <string>
#include <chrono>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "include/multithreaded_resizer.h"

int main()
{
    std::string input_image_path = "../multithreaded-image-resizer/test/input_image.jpg";
    std::string output_image_path = "../multithreaded-image-resizer/test/output_image.jpg";

    std::string input_images_dir_path = "../multithreaded-image-resizer/test/images";
    std::string output_images_dir_path = "../multithreaded-image-resizer/test/images/output-images";

    MultithreadedResizer resizer;

    std::chrono::high_resolution_clock::time_point single_thread_start = std::chrono::high_resolution_clock::now();
    resizer.resize_image_single_thread(input_image_path, 160, 90, output_image_path);
    std::chrono::high_resolution_clock::time_point single_thread_finish = std::chrono::high_resolution_clock::now();

    auto single_thread_duration = std::chrono::duration_cast<std::chrono::microseconds>(single_thread_finish - single_thread_start).count();
    std::cout << "Single-threaded duration: " << single_thread_duration << " microseconds." << std::endl;

    std::chrono::high_resolution_clock::time_point std_thread_start = std::chrono::high_resolution_clock::now();
    resizer.resize_image_std_thread(input_image_path, 160, 90, output_image_path);
    std::chrono::high_resolution_clock::time_point std_thread_finish = std::chrono::high_resolution_clock::now();

    auto std_thread_duration = std::chrono::duration_cast<std::chrono::microseconds>(std_thread_finish - std_thread_start).count();
    std::cout << "Multi-threaded (std::thread) duration: " << std_thread_duration << " microseconds." << std::endl;

    std::chrono::high_resolution_clock::time_point std_async_start = std::chrono::high_resolution_clock::now();
    resizer.resize_image_std_async(input_image_path, 160, 90, output_image_path);
    std::chrono::high_resolution_clock::time_point std_async_finish = std::chrono::high_resolution_clock::now();

    auto std_async_duration = std::chrono::duration_cast<std::chrono::microseconds>(std_async_finish - std_async_start).count();
    std::cout << "Multi-threaded (std::async) duration: " << std_async_duration << " microseconds." << std::endl;

    std::chrono::high_resolution_clock::time_point std_async_dir_start = std::chrono::high_resolution_clock::now();
    resizer.resize_images_std_async(input_images_dir_path, 160, 90, output_images_dir_path);
    std::chrono::high_resolution_clock::time_point std_async_dir_finish = std::chrono::high_resolution_clock::now();

    auto std_async_dir_duration = std::chrono::duration_cast<std::chrono::microseconds>(std_async_dir_finish - std_async_dir_start).count();
    std::cout << "Multi-threaded (std::async) duration of images directory processing: " << std_async_dir_duration << " microseconds." << std::endl;

    return 0;
}
