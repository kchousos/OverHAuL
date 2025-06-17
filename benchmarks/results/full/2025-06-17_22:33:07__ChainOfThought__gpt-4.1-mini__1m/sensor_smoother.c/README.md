# sensor_smoother.c

<versionBadge>![Version 0.2.1](https://img.shields.io/badge/version-0.2.1-blue.svg)</versionBadge>
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CI/CD Status Badge](https://github.com/mofosyne/sensor_smoother.c/actions/workflows/ci.yml/badge.svg)](https://github.com/mofosyne/sensor_smoother.c/actions)

![](test.png)

A small collection of sensor smoothing functions designed for embedded devices and other resource-constrained environments.

Pull requests and contributions are welcome!

## Design Considerations

* Minimal runtime checks are enabled by default to catch common misconfigurations (e.g., null buffers or invalid alpha).
* Runtime checks can be disabled via compile-time flags to eliminate overhead when inputs are guaranteed valid.
* The simple moving average requires the user to supply and manage the buffer, giving flexibility in buffer sizing and memory management.
* The exponential moving average maintains internal state and requires setting a smoothing factor alpha between 0 and 1 (exclusive).
* No dynamic memory allocation used.
* The library uses `float` (single precision) as it is the most common size for sensor data in embedded platforms.
    - Users needing `double` precision can modify the source code accordingly, and contributions to support this are welcome.

## Usage

### Simple Moving Average

The simple moving average function maintains a circular buffer of recent input values and returns their average. The user must allocate and manage this buffer.

```c
sensor_smoother_simple_moving_average_t sma_state = {0};
sma_state.buffer = buffer; // pre-allocated float array
sma_state.buffer_size = sizeof(buffer) / sizeof(buffer[0]);

float smoothed_value = sensor_smoother_simple_moving_average(&sma_state, input);
```

The function performs runtime validation by default:
* Returns the input value unchanged if the buffer is null or buffer size is zero.
    * Use this flag `DISABLE_SENSOR_SMOOTHER_SIMPLE_MOVING_AVERAGE_CHECKS` to disable null buffer and zero-size checks in the simple moving average function for performance reasons.

### Exponential Moving Average

The exponential moving average uses a smoothing factor `alpha` between 0 (exclusive) and 1 (exclusive) to weight new input values. Smaller alpha values smooth more aggressively.

```c
sensor_smoother_exponential_moving_average_t ema_state = {0};
ema_state.alpha = 0.1f;  // Smoothing factor: 0 < alpha < 1

float smoothed_value = sensor_smoother_exponential_moving_average(&ema_state, input);
```

The function performs runtime validation by default:
* Returns the input value unchanged if alpha is not strictly between 0 and 1.
    * Use this flag `DISABLE_SENSOR_SMOOTHER_EXPONENTIAL_MOVING_AVERAGE_CHECKS` to disable alpha range checks in the exponential moving average function for performance reasons.

## Installation Via Clib

You can install this library using [clib](https://github.com/clibs/clib):

```sh
clib install mofosyne/sensor_smoother.c
```

This will download `sensor_smoother.c` and `sensor_smoother.h` into your project’s `deps/` directory that you can then include into your codebase.

