#include <cstdint>
#include <cstdio>

const uint32_t JUMP_MULTIPLIERS[] = { 0x5D588B65, 0x8356D5D9, 0x766ED1F1, 0x434764E1, 0xE28E8DC1, 0x549A2B81, 0x06989701, 0xB6422E01, 0x2CC85C01, 0x3AA0B801, 0xF9817001, 0x0402E001, 0x4C05C001, 0xA80B8001, 0x90170001, 0x202E0001, 0x405C0001, 0x80B80001, 0x01700001, 0x02E00001, 0x05C00001, 0x0B800001, 0x17000001, 0x2E000001, 0x5C000001, 0xB8000001, 0x70000001, 0xE0000001, 0xC0000001, 0x80000001, 0x00000001 };
const uint32_t JUMP_INCREMENTS[] = { 0x00269EC3, 0x719F22B2, 0xE72DA594, 0x2AEC59E8, 0xDF1DFED0, 0x9173A9A0, 0xD47A0340, 0x0EDEC680, 0x52688D00, 0x5F7D1A00, 0xE9AA3400, 0x7E146800, 0xA728D000, 0xFA51A000, 0xA4A34000, 0x09468000, 0x128D0000, 0x251A0000, 0x4A340000, 0x94680000, 0x28D00000, 0x51A00000, 0xA3400000, 0x46800000, 0x8D000000, 0x1A000000, 0x34000000, 0x68000000, 0xD0000000, 0xA0000000, 0x40000000 };

// MODULUSとLCGのパラメータ
const uint64_t MODULUS = static_cast<uint64_t>(0xFFFFFFFF) + 1;
const uint32_t MULTIPLIER = 0x5D588B65;
const uint32_t INCREMENT = 0x269EC3;

uint32_t randMainJumpFlexible(uint32_t seed, uint32_t jumps) {
    uint64_t aN = 1;
    uint64_t cSum = 0;

    for (int i = 0; jumps > 0; ++i) {
        if (jumps & 1) {
            aN = (aN * JUMP_MULTIPLIERS[i]) % MODULUS;
            cSum = (cSum * JUMP_MULTIPLIERS[i] + JUMP_INCREMENTS[i]) % MODULUS;
        }
        jumps >>= 1;
    }

    return static_cast<uint32_t>((aN * seed + cSum) % MODULUS);
}

uint32_t randMainRaw(uint32_t seed) {
    return static_cast<uint32_t>((static_cast<uint64_t>(seed) * MULTIPLIER + INCREMENT) % MODULUS);
}

bool verify(uint32_t seed, uint32_t jumps, uint32_t expectedResult) {
    uint32_t testSeed = seed;
    for (uint32_t i = 0; i < jumps; ++i) {
        testSeed = randMainRaw(testSeed);
    }
    return testSeed == expectedResult;
}

int main() {
    uint32_t baseSeed = 0x7EA056A0;
    uint32_t jumps = 1000000000;

    uint32_t resultJump = randMainJumpFlexible(baseSeed, jumps);
    bool isValid = verify(baseSeed, jumps, resultJump);

    if (isValid) {
        printf("正確に計算されました！%u steps ahead result: %u\n", jumps, resultJump);
    } else {
        printf("計算に誤りがあります。\n");
    }

    return 0;
}
