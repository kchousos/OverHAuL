/*
    Sensor Smoother (Test Program)
    Author: Brian Khuu (2025)

    # MIT License

    Copyright (c) 2025 Brian Khuu

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

 */
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include "sensor_smoother.h"

#define NUM_STEPS 200

int main() {
    sensor_smoother_simple_moving_average_t simple_state = {0};
    float buffer[10] = {0};
    simple_state.buffer = buffer;
    simple_state.buffer_size = sizeof(buffer) / sizeof(buffer[0]);

    sensor_smoother_exponential_moving_average_t exponential_state = {0};
    exponential_state.alpha = 0.1;

    FILE *file_csv = fopen("test.csv", "w+");

    if (file_csv == NULL) {
        fprintf(stderr, "Could not open file\n");
        return 1;
    }

    // Write header row to CSV file
    fprintf(file_csv, "Time (ms),Input,SMA,EMA\n");

    double input_values[NUM_STEPS] = {0};
    for (int i = 0; i < NUM_STEPS; i++) {
        double sine_value = sin((double)(i / 8.0) / 3.14);
        double noise = ((double)rand() / RAND_MAX * 0.2) - 0.1;
        input_values[i] = sine_value + noise;

        float sma = sensor_smoother_simple_moving_average(&simple_state, input_values[i]);
        float ema = sensor_smoother_exponential_moving_average(&exponential_state, input_values[i]);

        // Write row to CSV file with time and averages values
        fprintf(file_csv, "%u,%f,%f,%f\n", i, input_values[i], sma, ema);
    }

    fclose(file_csv);

    return 0;
}