#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>

#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "include/multithreaded_resizer.h"

MultithreadedResizer::MultithreadedResizer()
{
}

MultithreadedResizer::~MultithreadedResizer()
{
}

cv::Mat MultithreadedResizer::resize_image_single_thread(const std::string& input_image_path,
                                                         unsigned int output_width,
                                                         unsigned int output_height,
                                                         const std::string& output_image_path)
{
    // Read input image.
    read_image(input_image_path);

    //std::chrono::high_resolution_clock::time_point st_start = std::chrono::high_resolution_clock::now();

    output_image_width_ = output_width;
    output_image_height_ = output_height;

    cv::resize(input_image_, output_image_, cv::Size(output_image_width_, output_image_height_));

    //std::chrono::high_resolution_clock::time_point st_finish = std::chrono::high_resolution_clock::now();

    //auto st_duration = std::chrono::duration_cast<std::chrono::microseconds>(st_finish - st_start).count();

    //std::cout << "Single-threaded duration: " << st_duration << " microseconds." << std::endl;

    save_image(output_image_, output_image_path);

    return output_image_;
}

cv::Mat MultithreadedResizer::resize_image_std_thread(const std::string& input_image_path,
                                                      unsigned int output_width,
                                                      unsigned int output_height,
                                                      const std::string& output_image_path)
{
    // Read input image.
    read_image(input_image_path);

    //std::chrono::high_resolution_clock::time_point mt_start = std::chrono::high_resolution_clock::now();

    output_image_width_ = output_width;
    output_image_height_ = output_height;

    // Make the output image ready to write.
    output_image_ = cv::Mat(output_image_height_, output_image_width_, CV_8UC3, cv::Scalar(0, 0, 0));

    columns_to_split_ = get_cores_number() / 2;
    rows_to_split_ = get_cores_number() / 2;

    chunk_width_ = input_image_width_ / columns_to_split_;
    chunk_height_ = input_image_height_ / rows_to_split_;

    unsigned int new_chunk_width = output_image_width_ / columns_to_split_;
    unsigned int new_chunk_height = output_image_height_ / rows_to_split_;

    std::vector<std::thread> threads;
    threads.reserve(columns_to_split_ * rows_to_split_);

    for (unsigned int y = 0; y < rows_to_split_; ++y)
    {
        for (unsigned int x = 0; x < columns_to_split_; ++x)
        {
            threads.push_back(std::thread(&MultithreadedResizer::process_chunk,
                                          std::ref(input_image_), chunk_width_, chunk_height_,
                                          new_chunk_width, new_chunk_height, x, y,
                                          std::ref(output_image_)));
        }
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    //std::chrono::high_resolution_clock::time_point mt_finish = std::chrono::high_resolution_clock::now();

    //auto mt_duration = std::chrono::duration_cast<std::chrono::microseconds>(mt_finish - mt_start).count();

    //std::cout << "Multi-threaded (std::thread) implementation duration: " << mt_duration << " microseconds." << std::endl;

    save_image(output_image_, output_image_path);

    return output_image_;
}

cv::Mat MultithreadedResizer::resize_image_std_async(const std::string& input_image_path, unsigned int output_width,
                                                     unsigned int output_height, const std::string& output_image_path)
{
    // Read input image.
    read_image(input_image_path);

    //std::chrono::high_resolution_clock::time_point async_start = std::chrono::high_resolution_clock::now();

    output_image_width_ = output_width;
    output_image_height_ = output_height;

    // Make the output image ready to write.
    output_image_ = cv::Mat(output_image_height_, output_image_width_, CV_8UC3, cv::Scalar(0, 0, 0));

    columns_to_split_ = get_cores_number() / 2;
    rows_to_split_ = get_cores_number() / 2;

    chunk_width_ = input_image_width_ / columns_to_split_;
    chunk_height_ = input_image_height_ / rows_to_split_;

    unsigned int new_chunk_width = output_image_width_ / columns_to_split_;
    unsigned int new_chunk_height = output_image_height_ / rows_to_split_;

    std::vector<std::future<void>> results;

    for (unsigned int y = 0; y < rows_to_split_; ++y)
    {
        for (unsigned int x = 0; x < columns_to_split_; ++x)
        {
            std::future<void> result = std::async(std::launch::async, &MultithreadedResizer::process_chunk,
                                                  std::ref(input_image_), chunk_width_, chunk_height_,
                                                  new_chunk_width, new_chunk_height, x, y,
                                                  std::ref(output_image_));

            results.push_back(std::move(result));
        }
    }

    for (auto& result : results)
    {
        result.get();
    }

    //std::chrono::high_resolution_clock::time_point async_finish = std::chrono::high_resolution_clock::now();

    //auto async_duration = std::chrono::duration_cast<std::chrono::microseconds>(async_finish - async_start).count();

    //std::cout << "Multi-threaded (std::async) implementation duration: " << async_duration << " microseconds." << std::endl;

    // Save image.
    save_image(output_image_, output_image_path);

    return output_image_;
}

void MultithreadedResizer::resize_images_std_async(const std::string& input_images_dir, unsigned int output_width,
                                                   unsigned int output_height, const std::string& output_images_dir)
{
    bool is_output_dir_exists = boost::filesystem::exists(output_images_dir);

    for (auto& file : boost::filesystem::directory_iterator(input_images_dir))
    {
        if (file.path().extension() == JPG_EXTENSION)
        {
            if (!is_output_dir_exists)
            {
                if (boost::filesystem::create_directory(output_images_dir))
                {
                    std::cout << "Directory " << output_images_dir << " was created." << std::endl;

                    is_output_dir_exists = true;
                }
                else
                {
                    std::cout << "Failed to create directory: " << output_images_dir << std::endl;
                    std::terminate();
                }
            }

            std::string output_image_name = "output_" + file.path().filename().string();
            std::string output_image_path = boost::filesystem::path(output_images_dir + "/" + output_image_name).string();

            resize_image_std_async(file.path().string(), output_width, output_height, output_image_path);
        }
    }
}

void MultithreadedResizer::process_chunk(cv::Mat& input_image, unsigned int chunk_width, unsigned int chunk_height,
                                         unsigned int new_chunk_width, unsigned int new_chunk_height,
                                         unsigned int x, unsigned int y, cv::Mat& output_image)
{
    // Get chunk from input image.
    cv::Rect roi(chunk_width * x, chunk_height * y, chunk_width, chunk_height);  // Region of interest.

    cv::Mat chunk = input_image(roi);

    // Resize chunk.
    cv::resize(chunk, chunk, cv::Size(new_chunk_width, new_chunk_height));

    // Merge chunk to output image.
    chunk.copyTo(output_image(cv::Rect(new_chunk_width * x, new_chunk_height * y, new_chunk_width, new_chunk_height)));
}

void MultithreadedResizer::split_image(const unsigned int columns, const unsigned int rows)
{
    columns_to_split_ = columns;
    rows_to_split_ = rows;

    chunks_.clear();
    chunks_.resize(rows_to_split_);

    chunk_width_ = input_image_width_ / columns_to_split_;
    chunk_height_ = input_image_height_ / rows_to_split_;

//    std::cout << "Chunk width: " << chunk_width_ << std::endl;
//    std::cout << "Chunk height: " << chunk_height_ << std::endl;

    std::vector<std::thread> threads;
    threads.reserve(columns_to_split_ * rows_to_split_);

    for (unsigned int y = 0; y < rows_to_split_; ++y)
    {
        for (unsigned int x = 0; x < columns_to_split_; ++x)
        {
//            std::cout << "Chunk " << "[" << y << "]" << "[" << x << "]" << " width: " << chunk.size().width << std::endl;
//            std::cout << "Chunk " << "[" << y << "]" << "[" << x << "]" << " height: " << chunk.size().height << std::endl;

            chunks_[y].resize(columns_to_split_);

            threads.push_back(std::thread(&MultithreadedResizer::separate_chunk, std::ref(input_image_),
                                          chunk_width_, chunk_height_, x, y, std::ref(chunks_)));
        }
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}

void MultithreadedResizer::separate_chunk(cv::Mat& input_image, unsigned int chunk_width, unsigned int chunk_height,
                                          unsigned int x, unsigned int y, std::vector<std::vector<cv::Mat>>& chunks)
{
    cv::Rect roi(chunk_width * x, chunk_height * y, chunk_width, chunk_height);  // Region of interest.

    cv::Mat chunk = input_image(roi);

    chunks[y][x] = chunk;
}

void MultithreadedResizer::resize_chunks(unsigned int chunk_width, unsigned int chunk_height)
{
    chunk_width_ = chunk_width;
    chunk_height_ = chunk_height;

    std::vector<std::thread> threads;
    threads.reserve(columns_to_split_ * rows_to_split_);

    for (unsigned int y = 0; y < rows_to_split_; ++y)
    {
        for (unsigned int x = 0; x < columns_to_split_; ++x)
        {
            threads.push_back(std::thread(&MultithreadedResizer::resize_chunk, std::ref(chunks_[y][x]),
                                          chunk_width_, chunk_height_));
        }
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}

void MultithreadedResizer::resize_chunk(cv::Mat& chunk, const unsigned int new_width, const unsigned int new_height)
{
    cv::resize(chunk, chunk, cv::Size(new_width, new_height));
}

void MultithreadedResizer::merge_chunks()
{
    output_image_ = cv::Mat(output_image_height_, output_image_width_, CV_8UC3, cv::Scalar(0, 0, 0));

    std::vector<std::thread> threads;
    threads.reserve(columns_to_split_ * rows_to_split_);

    for (unsigned int y = 0; y < rows_to_split_; ++y)
    {
        for (unsigned int x = 0; x < columns_to_split_; ++x)
        {
            threads.push_back(std::thread(&MultithreadedResizer::merge_chunk, std::ref(chunks_[y][x]),
                                          std::ref(output_image_), x, y));
        }
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}

void MultithreadedResizer::merge_chunk(cv::Mat& chunk, cv::Mat& output_image, unsigned int x, unsigned int y)
{
    chunk.copyTo(output_image(cv::Rect(chunk.size().width * x, chunk.size().height * y, chunk.size().width, chunk.size().height)));
}

void MultithreadedResizer::read_image(const std::string& image_path)
{
    input_image_ = cv::imread(image_path.c_str(), CV_LOAD_IMAGE_COLOR);

    if (!input_image_.empty())
    {
        input_image_width_ = input_image_.size().width;
        input_image_height_ = input_image_.size().height;

        //std::cout << "Image width: " << input_image_width_ << std::endl;
        //std::cout << "Image height: " << input_image_height_ << std::endl;
    }
    else
    {
        std::cerr << "Failed to read image: " << image_path << std::endl;
        std::terminate();
    }
}

void MultithreadedResizer::save_image(const cv::Mat& image, const std::string& path)
{
    if (cv::imwrite(path, image))
    {
        //std::cout << "Saved to: " << path << std::endl;
    }
    else
    {
        std::cout << "Failed to save to: " << path << std::endl;
        std::terminate();
    }
}

void MultithreadedResizer::show_image(const cv::Mat& image)
{
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);
    cv::imshow("Display window", image);
    cv::waitKey(0);
}

unsigned int MultithreadedResizer::get_cores_number()
{
    return std::thread::hardware_concurrency();
}

void MultithreadedResizer::show_chunks()
{
    for (unsigned int y = 0; y < chunks_.size(); ++y)
    {
        for (unsigned int x = 0; x < chunks_[y].size(); ++x)
        {
            cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);
            cv::imshow("Display window", chunks_[y][x]);
            cv::waitKey(0);
        }
    }
}
