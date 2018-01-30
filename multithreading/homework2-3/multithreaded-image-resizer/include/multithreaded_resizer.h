#ifndef MULTITHREADED_RESIZER_H
#define MULTITHREADED_RESIZER_H

#include <string>
#include <vector>
#include <thread>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class MultithreadedResizer
{
public:
    MultithreadedResizer();
    ~MultithreadedResizer();

    cv::Mat resize_image_single_thread(const std::string& input_image_path,
                                       unsigned int output_width,
                                       unsigned int output_height,
                                       const std::string& output_image_path);

    cv::Mat resize_image_std_thread(const std::string& input_image_path,
                                    unsigned int output_width,
                                    unsigned int output_height,
                                    const std::string& output_image_path);

    cv::Mat resize_image_std_async(const std::string& input_image_path,
                                   unsigned int output_width,
                                   unsigned int output_height,
                                   const std::string& output_image_path);

    // Resize the directory of images.
    void resize_images_std_async(const std::string& input_images_dir,
                                 unsigned int output_width,
                                 unsigned int output_height,
                                 const std::string& output_images_dir);

    cv::Mat get_input_image() { return input_image_; }
    cv::Mat get_output_image() { return output_image_; }

    static void show_image(const cv::Mat& image);
    static unsigned int get_cores_number();

    const std::string JPG_EXTENSION = ".jpg";

private:
    cv::Mat input_image_;
    unsigned int input_image_width_;
    unsigned int input_image_height_;

    cv::Mat output_image_;
    unsigned int output_image_width_;
    unsigned int output_image_height_;

    std::vector<std::vector<cv::Mat>> chunks_;

    unsigned int chunk_width_;
    unsigned int chunk_height_;

    unsigned int columns_to_split_;
    unsigned int rows_to_split_;

    void read_image(const std::string& image_path);
    void save_image(const cv::Mat& image, const std::string& path);

    // Main image processing fuction.
    static void process_chunk(cv::Mat& input_image, unsigned int chunk_width, unsigned int chunk_height,
                              unsigned int new_chunk_width, unsigned int new_chunk_height,
                              unsigned int x, unsigned int y, cv::Mat& output_image);

    /* Utility functions (they were needed for testing etc.) */

    void split_image(const unsigned int columns, const unsigned int rows);
    static void separate_chunk(cv::Mat& input_image, unsigned int chunk_width, unsigned int chunk_height,
                               unsigned int x, unsigned int y, std::vector<std::vector<cv::Mat>>& chunks);

    void resize_chunks(unsigned int chunk_width, unsigned int chunk_height);
    static void resize_chunk(cv::Mat& chunk, const unsigned int new_width, const unsigned int new_height);

    void merge_chunks();
    static void merge_chunk(cv::Mat& chunk, cv::Mat& output_image, unsigned int x, unsigned int y);

    void show_chunks();
};

#endif // MULTITHREADED_RESIZER_H
