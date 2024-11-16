//
// Created by Owner on 2024/11/12.
//

#include <iostream>
#include <cstdint>
#include <chrono>
#include <cassert>
#include <unordered_set>
#include "Lcg.h"
#include "debug.h"
#include <cmath>

//https://chatgpt.com/share/6732e1c2-5f30-8013-a93b-e0ea233db0ed

const uint32_t JUMP_MULTIPLIERS[] = {0x5D588B65, 0x8356D5D9, 0x766ED1F1, 0x434764E1, 0xE28E8DC1, 0x549A2B81, 0x06989701,
                                     0xB6422E01, 0x2CC85C01, 0x3AA0B801, 0xF9817001, 0x0402E001, 0x4C05C001, 0xA80B8001,
                                     0x90170001, 0x202E0001, 0x405C0001, 0x80B80001, 0x01700001, 0x02E00001, 0x05C00001,
                                     0x0B800001, 0x17000001, 0x2E000001, 0x5C000001, 0xB8000001, 0x70000001, 0xE0000001,
                                     0xC0000001, 0x80000001, 0x00000001};
const uint32_t JUMP_INCREMENTS[] = {0x00269EC3, 0x719F22B2, 0xE72DA594, 0x2AEC59E8, 0xDF1DFED0, 0x9173A9A0, 0xD47A0340,
                                    0x0EDEC680, 0x52688D00, 0x5F7D1A00, 0xE9AA3400, 0x7E146800, 0xA728D000, 0xFA51A000,
                                    0xA4A34000, 0x09468000, 0x128D0000, 0x251A0000, 0x4A340000, 0x94680000, 0x28D00000,
                                    0x51A00000, 0xA3400000, 0x46800000, 0x8D000000, 0x1A000000, 0x34000000, 0x68000000,
                                    0xD0000000, 0xA0000000, 0x40000000};


int position = 0;
uint32_t NowSeed = 0;

const uint64_t aN1 = 1963054081;
const uint64_t cSum1 = 1255585536;

void Lcg::randMainJumpFlexible(uint32_t jumps) {
    //jumps++;// なんか1つ前にジャンプしちゃう
    uint64_t aN = 1;
    uint64_t cSum = 0;

    for (int i = 0; jumps > 0; ++i) {
        if (jumps & 1) {
            aN = (aN * JUMP_MULTIPLIERS[i]) % MODULUS;
            cSum = (cSum * JUMP_MULTIPLIERS[i] + JUMP_INCREMENTS[i]) % MODULUS;
        }
        jumps >>= 1;
    }

    NowSeed = static_cast<uint32_t>((aN * NowSeed + cSum) % MODULUS);
    position += jumps;
}

uint32_t Lcg::randMainNop() {
    position++;
    NowSeed = static_cast<uint32_t>((static_cast<uint64_t>(NowSeed) * MULTIPLIER + INCREMENT) % MODULUS);
    return NowSeed;
}

// LCGで86785ステップ先の乱数を計算
void Lcg::randMainJump86785() {
    position += 86785;
    NowSeed = static_cast<uint32_t>((aN1 * NowSeed + cSum1) % MODULUS);
}

int Lcg::randMain(int max) {
    assert(max > 0);

    NowSeed = NowSeed * 0x5D588B65 + 0x269EC3;
    auto tmp = NowSeed >> 0x10;
    uint32_t result = (tmp * max) >> 0x10;
#ifdef DEBUG
    std::cout << position << ": " << std::hex << max << "/" << result << std::dec << std::endl;
#endif
    position++;
    assert(result < max);
    return static_cast<int>(result);
}

void Lcg::randInit(uint32_t seed) {
    NowSeed = seed;
    position = 0;
}

int Lcg::getPosition() {
    return position;
}

// LCGの逆計算をして1つ前のシードを求める
void Lcg::previousSeed() {
    static const uint32_t INVERSE_MULTIPLIER = Lcg::modularInverse(MULTIPLIER, MODULUS);
    // currentSeedから1つ前のシードを逆計算
    uint64_t temp = (static_cast<uint64_t>(NowSeed) + MODULUS - INCREMENT) % MODULUS;
    NowSeed = static_cast<uint32_t>((INVERSE_MULTIPLIER * temp) % MODULUS);
}

// 拡張ユークリッド法を使用して逆元を求める
uint32_t Lcg::modularInverse(uint32_t a, uint64_t m) {
    int64_t t = 0, newt = 1;
    int64_t r = static_cast<int64_t>(m), newr = a;
    while (newr != 0) {
        int64_t quotient = r / newr;
        t = t - quotient * newt;
        std::swap(t, newt);
        r = r - quotient * newr;
        std::swap(r, newr);
    }
    if (t < 0) t += m;
    return static_cast<uint32_t>(t);
}

uint32_t Lcg::getNowSeed() {
    return NowSeed;
}

int Lcg::randIntRange(int base, int diff1, int diff2) {
    auto test = randMain(diff1 + diff2 + 1);
    auto tmp2 =  ((100 - diff1) + test);
    return static_cast<int>(std::floor((base * tmp2) / 100));
}
