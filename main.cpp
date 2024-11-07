#include <iostream>
#include <cstdint>
#include <chrono>

uint32_t randMain();

void randInit(uint32_t seed);

uint32_t modularInverse(uint32_t a, uint32_t m);

uint32_t previousSeed(uint32_t currentSeed);

uint32_t randMainRaw(uint32_t seed);

// LCGのパラメータ
// LCGのパラメータ
const uint32_t MULTIPLIER = 0x5D588B65;
const uint32_t INCREMENT = 0x269EC3;
const uint64_t MODULUS = static_cast<uint64_t>(0xFFFFFFFF) + 1;  // オーバーフロー防止のため64ビットで計算

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

// モジュラ指数法: (base^exp) % mod を効率的に計算
uint64_t modExp(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp /= 2;
    }
    return result;
}

void generateConstants(uint32_t maxSteps) {
    // a^N % m の定数生成
    printf("const uint64_t a_powers[%d] = {", maxSteps);
    for (uint32_t i = 1; i <= maxSteps; ++i) {
        uint64_t aN = modExp(MULTIPLIER, i, MODULUS);
        printf("%llu%s", aN, (i < maxSteps ? ", " : ""));
    }
    printf("};\n");

    // c * (a^(N-1) + a^(N-2) + ... + 1) % m の定数生成
    printf("const uint64_t c_sums[%d] = {", maxSteps);
    for (uint32_t N = 1; N <= maxSteps; ++N) {
        uint64_t sum = 0;
        uint64_t factor = 1;
        for (uint32_t i = 0; i < N; ++i) {
            sum = (sum + factor) % MODULUS;
            factor = (factor * MULTIPLIER) % MODULUS;
        }
        uint64_t cSum = (sum * INCREMENT) % MODULUS;
        printf("%llu%s", cSum, (N < maxSteps ? ", " : ""));
    }
    printf("};\n");
}

// 除算を使った計算
inline int div_day_adjust(int day) {
    return day + (day / 0xA) * 6;
}

uint32_t randMainJump(uint32_t seed, uint32_t N) {
    if (N == 0 || N > 100) return seed;  // 範囲外チェック
    uint64_t aN = a_powers[N - 1];       // N = 1 の場合 a_powers[0]
    uint64_t cSum = c_sums[N - 1];
    return static_cast<uint32_t>((aN * seed + cSum) % MODULUS);
}

std::uint32_t encodeDate(int year1, int month, int day) {
    using namespace std::chrono;
    if (year1 < 2000){
        throw std::out_of_range("Invalid Date input");
    }

    // 年月日の変換 (C++20では、西暦を直接扱える)
    auto date = year{year1} / month / day;

    // 曜日の計算
    std::chrono::weekday weekday{date};; // 0: 日曜日〜6: 土曜日

    int year2 = year1 - 2000;

    //10 = 0x10
    month = div_day_adjust(month);
    day = div_day_adjust(day);
    year2 = div_day_adjust(year2);


    // 年を2桁に短縮してビットシフトで16進数相当の整数を作成
    std::uint32_t encodedDate =
            ((year2 % 0xff) << 0) |    // 年の下2桁を最下位バイトに格納
            ((month & 0xFF) << 8) |    // 月を8ビット左シフト
            ((day & 0xFF) << 16) |    // 日を16ビット左シフト
            ((weekday.c_encoding() & 0xFF) << 24); // 曜日を24ビット左シフト

    return encodedDate;
}

// 時、分、秒のスキップを考慮したエンコード
std::uint32_t encodeTime(int hour, int minute, int second) {
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        throw std::out_of_range("Invalid time input");
    }

    std::uint32_t encodedTime = 0;

    // 時 (1-19までと20-23までを分けて処理)
    if (hour <= 11) {
        hour = div_day_adjust(hour);
        encodedTime |= hour;  // 時を24ビット左シフト
    } else if(hour <= 19){
        encodedTime |= (hour - 12 + 0x52);  // 20時から23時は0x60から始まる
    }else{
        encodedTime |= (hour - 20 + 0x60);  // 20時から23時は0x60から始まる
    }

    minute = div_day_adjust(minute);
    second = div_day_adjust(second);

    encodedTime |= (minute << 8);  // 分を16ビット左シフト
    encodedTime |= (second << 16);  // 秒を8ビット左シフト

    return encodedTime;
}

std::vector<uint32_t> getPreviousSeeds(uint32_t currentSeed, int steps) {
    std::vector<uint32_t> seeds;
    seeds.push_back(currentSeed);
    for (int i = 0; i < steps; ++i) {
        currentSeed = previousSeed(currentSeed);
        seeds.push_back(currentSeed);
    }
    return seeds;
}


//#define back 1
//#define back2 1
//#define back3 1

// TIP Press <shortcut actionId="Debug"/> to start debugging your code.
// We have set one <icon src="AllIcons.Debugger.Db_set_breakpoint"/>
// breakpoint for you, but you can always add more by pressing
// <shortcut actionId="ToggleLineBreakpoint"/>.
// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    auto t0 = std::chrono::high_resolution_clock::now();
//
//    uint32_t test1 = 0x7fa978ca - 0x1090123 - 0x102107;
//    std::cout << std::hex << test1 << std::dec << std::endl;
//
    //uint32_t base1 = 0x7EA056A0;
    uint32_t base1 = 0x7e9056a0;
////
//    std::uint32_t encodedDate2 = encodeTime(8, 19, 10);
//    std::cout << std::hex << encodedDate2 << std::endl;
    //return 0;
//    //2178380071, 2092/2/27 19:18:10, 0x327022e, 101859

    for (int y = 2000; y < 2099; ++y) {
        for (int m = 1; m < 12; ++m) {
            for (int d = 0; d < 28; ++d) {
                for (int h = 0; h < 24; ++h) {
                    for (int min = 0; min < 60; ++min) {
                        std::uint32_t encodedDate2 = encodeDate(y, m, d);
                        std::uint32_t encodeTime2 = encodeTime(h, min, 10);
                        uint32_t seed = base1 + encodedDate2 + encodeTime2;
                        uint32_t slotStart = randMainJump(seed, 22);
                        randInit(slotStart);
                        int j = 0;
                        for (; j < 5; ++j) {
                            auto rand = randMain();
                            if (rand != 0) {
                                break;
                            }
                        }
                        if (j == 5) {
                            std::cout << seed << ", " << y << "/" << m << "/" << d << " " << h << ":" << min  << ":10"  << ", " << std::hex << "0x" << encodedDate2 << ", " << encodeTime2 << std::dec << std::endl;
                            //return 0;
                        }
                    }
                }
            }
        }
    }
    return 0;

#ifdef back3
    uint32_t base = 0x7EA056A0;
    int year = 2024, month = 10, day = 10;
    std::uint32_t encodedDate = encodeDate(year, month, day);
    std::uint32_t encodeTime1 = encodeTime(17, 20, 57);
    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << encodedDate << std::endl;
    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << encodeTime1 << std::endl;
#endif

#ifdef back2
    // 初期シード
    uint32_t seedA = 0x322F3F16;
    uint32_t seedB = 0x4C8D3D2C;

    // 過去に23回戻る
    int steps = 23;
    auto pastSeedsA = getPreviousSeeds(seedA, steps);
    auto pastSeedsB = getPreviousSeeds(seedB, steps);

    // 距離の最小ペアを求める
    uint32_t minDistance = std::numeric_limits<uint32_t>::max();
    uint32_t closestSeedA = 0, closestSeedB = 0;

    for (size_t i = 18; i < pastSeedsA.size(); ++i) {
        for (size_t j = 18; j < pastSeedsB.size(); ++j) {
            uint32_t distance = std::abs(static_cast<int64_t>(pastSeedsA[i]) - static_cast<int64_t>(pastSeedsB[j]));
            if (distance < minDistance) {
                minDistance = distance;
                closestSeedA = pastSeedsA[i];
                closestSeedB = pastSeedsB[j];
            }
        }
    }

    // 結果の表示
    std::cout << "Closest seeds found with minimum distance:" << std::endl;
    std::cout << "Seed A: 0x" << std::hex << closestSeedA << std::dec << " (steps back from initial seed A)" << std::endl;
    std::cout << "Seed B: 0x" << std::hex << closestSeedB << std::dec << " (steps back from initial seed B)" << std::endl;
    std::cout << "Minimum Distance: 0x" << std::hex << minDistance << std::dec << std::endl;

#endif

#ifdef back
    // 初期パラメータ
    //uint32_t initialSeed = 0x322f3f16;
    //uint32_t initialSeed = 0x4c8d3d2c;
    uint32_t initialSeed = 0x546cbb49;

    //uint32_t test = 0x7fa978ca;
    uint32_t test1 = 0x7fa978ca - 0x1090123 - 0x00102107;
    std::cout << std::hex << test1 << std::dec << std::endl;


    //ADBD4907
    // 過去に20回〜23回戻る
    int steps = 23;
    auto pastSeeds = getPreviousSeeds(initialSeed, steps);

    // 過去のシードを表示
    std::cout << "Previous 20-23 seeds:" << std::endl;
    for (int i = 0; i <= 25; ++i) {
        std::cout << "Seed " << i << " steps back: 0x" << std::hex << pastSeeds[i] << std::dec << std::endl;
    }

#endif

#ifndef back
#ifndef back2
#ifndef back3
    //00-09-BF-6C-ED-92
//    uint32_t test2[10] = {
//        0x0,
//        0x92ed0000 ^ 0,
//        0x86000000 ^ (0x6cBF0900 ^ 0 ^ 6),
//        0x30010102, //03061124 = 24-11-6 3???? 曜日
//        0x00010000,//7-39-36 = 00363907
//        0x0,
//        0x0,
//        0x00002fff,
//    };
//
//    uint32_t collector = 0;
//    for (int i = 0; i < 8; ++i) {
//        collector += test2[i];
//    }
//    std::cout << collector << std::endl;

    int size = 2;
//    uint32_t input[500] = {
//            2, 11, 3, 15, 9,
//            9, 9, 15, 4, 11,
//            8, 4, 12, 5, 7,
//    };

// リール1の5と14は同じ
//    uint32_t input[500] = {
//            14, 14, 3, 13, 6,
//            9, 5, 1, 10, 15,
//    };

    uint32_t input[500] = {
            3, 11, 5, 4, 9,
            10, 14, 14, 12, 1,
    };

    //841957142

    auto max = size * 5;
    for (uint32_t couter = 0; couter < UINT32_MAX; couter++) {//UINT32_MAX
        randInit(couter);
        int j = 0;
        for (; j < max; ++j) {
            auto rand = randMain();
            if (rand != input[j]){
                break;
            }
        }
        if (j == max){
            std::cout << "0x" << std::hex << couter << std::dec << std::endl;
        }
    }


#endif
#endif
#endif
//
//    auto t1 = std::chrono::high_resolution_clock::now();
//    auto elapsed_time =
//            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
//    std::cout << "elapsed time: " << double(elapsed_time) / 1000 << " ms" << std::endl;
//

    return 0;
}

uint32_t NowSeed = 0x7079cffe;


uint32_t randMain() {
    NowSeed = NowSeed * 0x5D588B65 + 0x269EC3;
    auto tmp = NowSeed >> 0x10;
    return (tmp * 0x10) >> 0x10;
}

inline void randInit(uint32_t seed) {
    NowSeed = seed;
}

// 拡張ユークリッド法を使用して逆元を求める
uint32_t modularInverse(uint32_t a, uint64_t m) {
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

// LCGの逆計算をして1つ前のシードを求める
uint32_t previousSeed(uint32_t currentSeed) {
    static const uint32_t INVERSE_MULTIPLIER = modularInverse(MULTIPLIER, MODULUS);
    // currentSeedから1つ前のシードを逆計算
    uint64_t temp = (static_cast<uint64_t>(currentSeed) + MODULUS - INCREMENT) % MODULUS;
    return static_cast<uint32_t>((INVERSE_MULTIPLIER * temp) % MODULUS);
}

// テスト用のLCG関数
uint32_t randMainRaw(uint32_t seed) {
    return static_cast<uint32_t>((static_cast<uint64_t>(seed) * MULTIPLIER + INCREMENT) % MODULUS);
}


