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

double IntToFloat(int value) {
    return static_cast<double>(value);
}

double floatRand(double min, double max) {
    return min + (max - min) * 0;
}

double FUN_02075930(int kiyousa, double batomasu, double sukiru, double taisei,
                    int number_of_attack) {
    if (kiyousa >= 150){
        kiyousa = kiyousa - 150;
    }else{
        kiyousa = 0;
    }
    return taisei * (IntToFloat(kiyousa) * 0.01 + 2.0 + batomasu + sukiru) / IntToFloat(number_of_attack);
}

double FUN_021da720(int PlayerLevel) {
    auto level = static_cast<double>(PlayerLevel);
    double base = (level * level / 23) + 100;
    base = base + base * floatRand(-0.05, 0.05);
    auto guarantee = 217 * floatRand(0.9, 1.1);
    if (guarantee > base) {
        return base;
    }
    return base;
}


int main() {
    std::cout << FUN_021da720(99) << std::endl;
    return 0;
}
