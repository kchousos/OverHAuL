#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sensor_smoother.h"

// Max buffer size for SMA to avoid excessive allocations.
#define MAX_SMA_BUFFER_SIZE 64

// Helper to read a size_t from data safely.
static size_t read_size_t(const uint8_t **data, size_t *size)
{
    size_t val = 0;
    if (*size >= sizeof(size_t))
    {
        memcpy(&val, *data, sizeof(size_t));
        *data += sizeof(size_t);
        *size -= sizeof(size_t);
    }
    return val;
}

// Helper to read a float from data safely.
static float read_float(const uint8_t **data, size_t *size)
{
    float val = 0.0f;
    if (*size >= sizeof(float))
    {
        memcpy(&val, *data, sizeof(float));
        *data += sizeof(float);
        *size -= sizeof(float);
    }
    return val;
}

// Helper to read an int from data safely.
static int read_int(const uint8_t **data, size_t *size)
{
    int val = 0;
    if (*size >= sizeof(int))
    {
        memcpy(&val, *data, sizeof(int));
        *data += sizeof(int);
        *size -= sizeof(int);
    }
    return val;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 10) // minimal size to hold some data for struct fields and calls
        return 0;

    // Parse SMA struct fields from input:
    // 1) buffer_size (size_t) capped
    size_t buffer_size = read_size_t(&data, &size);
    if (buffer_size > MAX_SMA_BUFFER_SIZE)
        buffer_size = MAX_SMA_BUFFER_SIZE;

    // Allocate buffer or set to NULL if zero to test edge case
    float *buffer = NULL;
    if (buffer_size > 0)
    {
        buffer = (float *)malloc(sizeof(float) * buffer_size);
        if (!buffer)
            return 0;
        // Fill buffer from input floats or zero if insufficient data
        for (size_t i = 0; i < buffer_size; i++)
        {
            if (size >= sizeof(float))
            {
                memcpy(&buffer[i], data, sizeof(float));
                data += sizeof(float);
                size -= sizeof(float);
            }
            else
            {
                buffer[i] = 0.0f;
            }
        }
    }

    sensor_smoother_simple_moving_average_t sma = {0};
    sma.buffer = buffer;
    sma.buffer_size = buffer_size;

    // buffer_count and write_index from input but capped within buffer_size
    size_t buffer_count = read_size_t(&data, &size);
    if (buffer_count > buffer_size)
        buffer_count = buffer_size;
    size_t write_index = read_size_t(&data, &size);
    if (buffer_size > 0)
        write_index = write_index % buffer_size;
    else
        write_index = 0;

    sma.buffer_count = buffer_count;
    sma.write_index = write_index;

    // Parse EMA struct fields alpha (float) and init (int)
    float alpha = read_float(&data, &size);
    int init = read_int(&data, &size);
    if (init != 0 && init != 1)
        init = 0;

    sensor_smoother_exponential_moving_average_t ema = {0};
    ema.alpha = alpha;
    ema.init = init;
    ema.lastOutput = 0.0f;

    // Use remaining data as float input values to feed smoothing functions
    // For each float value, call both smoothing functions
    while (size >= sizeof(float))
    {
        float input_val = read_float(&data, &size);

        // Call simple moving average
        (void)sensor_smoother_simple_moving_average(&sma, input_val);

        // Call exponential moving average
        (void)sensor_smoother_exponential_moving_average(&ema, input_val);
    }

    free(buffer);
    return 0;
}