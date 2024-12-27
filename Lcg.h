//
// Created by Owner on 2024/11/12.
//

#ifndef DQ5_DS_RANDS_LCG_H
#define DQ5_DS_RANDS_LCG_H


#include <cstdint>

class Lcg {
public:
    static void randMainJumpFlexible(uint32_t jumps);

    static uint32_t randMainNop();

    static void randMainJump86785();

    static int randMain(int max);

    static void randInit(uint32_t seed);

    static int getPosition();

    static int randIntRange(int base, int diff1, int diff2);

    static uint32_t getNowSeed();

    static void previousSeed();

private:
    static const uint32_t MULTIPLIER = 0x5D588B65;
    static const uint32_t INCREMENT = 0x269EC3;
    static const uint64_t MODULUS =
            static_cast<uint64_t>(static_cast<uint64_t>(0xffffffff)) + 1;  // オーバーフロー防止のため64ビットで計算


    static uint32_t modularInverse(uint32_t a, uint64_t m);
};


#endif //DQ5_DS_RANDS_LCG_H