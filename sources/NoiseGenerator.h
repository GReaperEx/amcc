#ifndef NOISE_GENERATOR
#define NOISE_GENERATOR

#include <cmath>
#include <random>
#include <cstdint>
#include <algorithm>

// This class is almost a copy-paste of this one:
// https://github.com/Reputeless/PerlinNoise/blob/master/PerlinNoise.hpp
class NoiseGenerator
{
public:
    explicit NoiseGenerator(unsigned seed) {
        reseed(seed);
    }

    void reseed(unsigned seed) {
        for (int i = 0; i < 256; ++i) {
            p[i] = i;
        }

        std::shuffle(p, p + 256, std::mt19937(seed));

        for (int i = 0; i < 256; ++i) {
            p[256 + i] = p[i];
        }
    }

    float noise(float x, float y, float z) const {
        int X = (int)(floorf(x)) & 255;
        int Y = (int)(floorf(y)) & 255;
        int Z = (int)(floorf(z)) & 255;

        x -= floorf(x);
        y -= floorf(y);
        z -= floorf(z);

        float u = fade(x);
        float v = fade(y);
        float w = fade(z);

        int A = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
            grad(p[BA], x - 1, y, z)),
            lerp(u, grad(p[AB], x, y - 1, z),
            grad(p[BB], x - 1, y - 1, z))),
            lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
            grad(p[BA + 1], x - 1, y, z - 1)),
            lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
            grad(p[BB + 1], x - 1, y - 1, z - 1))))*0.5f + 0.5f;
    }

private:
    int p[512];

    float fade(float t) const {
        return t*t*t*(t*(t*6 - 15) + 10);
    }

    float lerp(float t, float a, float b) const {
        return a + t*(b - a);
    }

    float grad(int hash, float x, float y, float z) const {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : h == 12 || h == 14 ? x : z;

        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};

#endif // NOISE_GENERATOR
