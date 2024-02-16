#ifndef OPENCV_ONNX_LEAP_EURO_FILTER_H
#define OPENCV_ONNX_LEAP_EURO_FILTER_H

#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <thread>

constexpr const float F32_PI = 3.14159265358979323846f;

using namespace std::chrono;

class OneEuroFilter {
public:
    OneEuroFilter() = default;

    [[maybe_unused]]
    explicit OneEuroFilter(float x0, float dx0 = 0.0, float min_cutoff = 1.0, float beta = 0.0, float d_cutoff = 1.0) :
            min_cutoff(min_cutoff), beta(beta), d_cutoff(d_cutoff), x_prev(x0), dx_prev(dx0), t_prev(std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::system_clock::now().time_since_epoch()).count()) {}

    float operator()(float x) {
        float t = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        float t_e = fabs(t - t_prev);

        float a_d = smoothing_factor(t_e, d_cutoff);
        float dx = (x - x_prev) / t_e;
        float dx_hat = exponential_smoothing(a_d, dx, dx_prev);

        float cutoff = min_cutoff + beta * abs(dx_hat);
        float a = smoothing_factor(t_e, cutoff);
        float x_hat = exponential_smoothing(a, x, x_prev);

        x_prev = x_hat;
        dx_prev = dx_hat;
        t_prev = t;

        return x_hat;
    }

private:
    static float smoothing_factor(float t_e, float cutoff) {
        float r = 2 * F32_PI * cutoff * t_e;
        return r / (r + 1);
    }

    static float exponential_smoothing(float a, float x0, float x1) {
        return a * x0 + (1 - a) * x1;
    }

    float min_cutoff;
    float beta;
    float d_cutoff;
    float x_prev;
    float dx_prev;
    float t_prev;
};

#endif //OPENCV_ONNX_LEAP_EURO_FILTER_H
