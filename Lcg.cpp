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

const uint64_t a_powers[100] = {1566083941, 2203506137, 1324822941, 1986974193, 2643373845, 1922267721, 3584877005,
                                1128752353, 3108566981, 1963571129, 3492736765, 2923407569, 1151276405, 1419902505,
                                2617346349, 3800993217, 2693642277, 144555417, 3452297309, 2296120241, 111579093,
                                3792135689, 2305434765, 2721118881, 968903813, 3508692857, 3076097469, 2481414801,
                                2342952501, 870003177, 1185690605, 1419389825, 3888788709, 2637641049, 2891407133,
                                2853520753, 2465251477, 96788937, 3478992717, 1749393505, 1340404037, 3574310713,
                                2449271933, 2703830097, 2569434869, 872150441, 669167277, 3049183553, 2677670309,
                                408894745, 1767353821, 938907441, 3191423317, 2034757001, 1399986701, 2376160801,
                                314762757, 478246649, 3846513469, 1147209233, 3949633461, 1828735337, 2316368237,
                                110663425, 3112771173, 3394360537, 1081265309, 559675633, 961286677, 3238824265,
                                2928987341, 2338331617, 2120624837, 2621429433, 1398680061, 1558900689, 2512328821,
                                3219402025, 77796397, 2831377601, 1814268709, 1011547289, 3931421533, 2787207857,
                                2851418837, 177124617, 649154445, 463335841, 869973893, 1644349049, 1534329021,
                                4213369233, 7332149, 2527305961, 1006960365, 517685889, 4076160997, 4035359833,
                                1554610717, 388164721};
const uint64_t c_sums[100] = {2531011, 1906254514, 3539334397, 3878528404, 2901429799, 3184304422, 812766401, 720132584,
                              1945571147, 1723162714, 3522592837, 3603214588, 3933816367, 2386744398, 3130702729,
                              3743284944, 2010584787, 2431594754, 2698054285, 3436336996, 1660314935, 2587454070,
                              2083238225, 394432184, 291951963, 433665194, 4276138965, 3265935564, 3695561535,
                              2377319326, 3205355033, 2440276384, 2665638627, 3565947730, 2478611485, 3846960436,
                              3810629703, 4252909510, 1772505569, 456585096, 1336014699, 1210864890, 2478302053,
                              4225969308, 3147820111, 3384131310, 1532374185, 2457731184, 211393267, 3932860834,
                              1415686573, 2118614788, 3249727319, 2956992790, 3103432305, 2464578648, 3503912315,
                              2315501898, 3506769653, 1534889068, 967354719, 256902718, 694992697, 3564766016,
                              1913497347, 3913294834, 522793789, 1882322132, 2427146855, 3622047334, 33664769,
                              976106792, 3640545163, 2289334682, 3541120645, 2906871868, 222364271, 2957433230,
                              1661636041, 2422518288, 2853146387, 2692833858, 1413232845, 3996669604, 4023114103,
                              1183778742, 1146387345, 4218100216, 1572963739, 1297317354, 568678933, 2143588364,
                              50984831, 3542038750, 1906874457, 368774368, 3375387427, 3898829970, 126073437,
                              1447535732};

int position = 0;
uint32_t NowSeed = 0;

const uint64_t aN1 = 1963054081;
const uint64_t cSum1 = 1255585536;

void Lcg::randMainJumpFlexible(uint32_t jumps) {
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
}

void Lcg::randMainRaw() {
    NowSeed = static_cast<uint32_t>((static_cast<uint64_t>(NowSeed) * MULTIPLIER + INCREMENT) % MODULUS);
}

void Lcg::randMainJump(uint32_t N) {
    position += static_cast<int>(N);
    if (N == 0 || N > 100) return;  // 範囲外チェック
    uint64_t aN = a_powers[N - 1];       // N = 1 の場合 a_powers[0]
    uint64_t cSum = c_sums[N - 1];
    NowSeed = static_cast<uint32_t>((aN * NowSeed + cSum) % MODULUS);
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
