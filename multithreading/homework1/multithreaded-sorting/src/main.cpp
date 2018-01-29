#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <thread>
#include <pthread.h>
#include <semaphore.h>
#include <chrono>

const unsigned int ARRAYS_NUMBER = 10;
const unsigned int ARRAYS_SIZE = 10;

std::vector<int*> arrays (ARRAYS_NUMBER);
std::vector<pthread_t> threads (ARRAYS_NUMBER);

sem_t semaphore;

void delete_arrays()
{
    for (auto it = arrays.begin(); it != arrays.end(); ++it)
    {
        if ((*it) != nullptr)
        {
            delete (*it);
        }
    }
}

void create_and_fill_arrays()
{
    delete_arrays();

    // Random array content.
    srand(time(nullptr));

    for (unsigned int i = 0; i < ARRAYS_NUMBER; ++i)
    {
        int* new_array = new int[ARRAYS_SIZE];  // Create array.

        for (unsigned int i = 0; i < ARRAYS_SIZE; ++i)
        {
            new_array[i] = rand() % 100 + 1;
        }

        arrays.at(i) = new_array;
    }
}

void* sort_array(void* array)
{
    sem_wait(&semaphore);
    //std::cout << "Semaphore is locked." << std::endl;
    int* arr = (int*) array;

    std::sort(arr, arr + ARRAYS_SIZE);
    //std::cout << "Semaphore is unlocked." << std::endl;
    sem_post(&semaphore);

    return nullptr;
}

void sort_arrays()
{
    auto arr_it = arrays.begin();
    auto thr_it = threads.begin();

    while (arr_it != arrays.end() || thr_it != threads.end())
    {
        pthread_create(&(*thr_it), nullptr, sort_array, &(**arr_it));

        ++arr_it;
        ++thr_it;

        //std::cout << "Thread created." << std::endl;
    }

    for (auto it = threads.begin(); it != threads.end(); ++it)
    {
        pthread_join(*it, nullptr);
    }
}

void sort_arrays_single_thread()
{
    auto arr_it = arrays.begin();

    while (arr_it != arrays.end())
    {
        std::sort(*arr_it, *(arr_it) + ARRAYS_SIZE);

        ++arr_it;
    }
}

void print_arrays()
{
    std::cout << "Arrays:" << std::endl << std::endl;

    for (auto it = arrays.begin(); it != arrays.end(); ++it)
    {
        for (unsigned int i = 0; i < ARRAYS_SIZE; ++i)
        {
            std::cout << (*it)[i] << " ";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

unsigned int get_cores_number()
{
    return std::thread::hardware_concurrency();
}

int main()
{
    sem_init(&semaphore, 0, get_cores_number());

    std::chrono::high_resolution_clock::time_point st_start = std::chrono::high_resolution_clock::now();
    create_and_fill_arrays();
    //print_arrays();
    sort_arrays_single_thread();
    //print_arrays();
    std::chrono::high_resolution_clock::time_point st_finish = std::chrono::high_resolution_clock::now();

    std::chrono::high_resolution_clock::time_point mt_start = std::chrono::high_resolution_clock::now();
    create_and_fill_arrays();
    //print_arrays();
    sort_arrays();
    //print_arrays();
    std::chrono::high_resolution_clock::time_point mt_finish = std::chrono::high_resolution_clock::now();

    auto st_duration = std::chrono::duration_cast<std::chrono::microseconds> (st_finish - st_start).count();
    std::cout << "Singlethreaded duration: " << st_duration << " microseconds." << std::endl;

    auto mt_duration = std::chrono::duration_cast<std::chrono::microseconds> (mt_finish - mt_start).count();
    std::cout << "Multithreaded duration: " << mt_duration << " microseconds." << std::endl << std::endl;

    return 0;
}
