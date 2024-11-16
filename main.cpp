#include <cstdint>
#include <cstdio>
#include <iostream>


uint64_t now_seed = 0x316DF;

const int ARRAY_SIZE = 100000;
double precalculatedValues[ARRAY_SIZE];

constexpr uint64_t MULTIPLIER = 0x5d588b656c078965;
constexpr uint64_t INCREMENT = 0x269ec3;
constexpr uint64_t MODULO = 0xFFFFFFFFFFFFFFFF;

// 総当たり回数カウント
uint64_t totalMatches = 0;

// 線形合同法 (LCG)
uint64_t lcg_rand(uint64_t seed) {
    return (seed * MULTIPLIER + INCREMENT) & MODULO;
}

// 高速化した calculatePercent
int fastCalculatePercent(uint64_t input) {
    // 上位32ビットを取得
    uint64_t output = input >> 32;
    // 100倍して 2^32 で割る（整数部分を取得）
    return static_cast<int>((output * 100) >> 32);
}

int main() {
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        now_seed = lcg_rand(now_seed);
        precalculatedValues[i] = fastCalculatePercent(now_seed);
        if (i <= 12){
            continue;
        }
        if (precalculatedValues[i-3*4] > 15){
            continue;
        }
        if (precalculatedValues[i-3*3] > 14){
            continue;
        }
        if (precalculatedValues[i-3*2] > 14){
            continue;
        }
        if (precalculatedValues[i-3*1] > 14){
            continue;
        }
        if (precalculatedValues[i] > 14){
            continue;
        }
        std::cout << i << std::endl;
        //break;
    }
    return 0;
}
