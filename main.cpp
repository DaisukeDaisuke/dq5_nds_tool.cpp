#include <iostream>
#include <cstdint>
#include <chrono>
#include <cassert>


uint32_t randMain();

void randInit(uint32_t seed);

uint32_t modularInverse(uint32_t a, uint32_t m);

uint32_t previousSeed(uint32_t currentSeed);

uint32_t randMainRaw(uint32_t seed);

//#define DEBUG 1

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
    if (year1 < 2000) {
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
    } else if (hour <= 19) {
        encodedTime |= (hour - 12 + 0x52);  // 20時から23時は0x60から始まる
    } else {
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

int position = 0;
uint32_t NowSeed = 2208399859;


int randMain(int max) {
    assert(max > 0);

    NowSeed = NowSeed * 0x5D588B65 + 0x269EC3;
    auto tmp = NowSeed >> 0x10;
    uint32_t result = (tmp * max) >> 0x10;
#ifdef DEBUG
    std::cout << position << ": " << std::hex <<  max << "/" << result << std::dec << std::endl;
#endif
    position++;
    assert(result < max);
    return static_cast<int>(result);
}

inline void randInit(uint32_t seed) {
    NowSeed = seed;
}

constexpr int getMemConst(uint32_t offset) {
    switch (offset) {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return 2;
        case 3:
            return 3;
        case 8:
            return 8;
        case 4:
            return randMain(2) + 1;  // 実行時に評価される
        case 5:
            return randMain(2) + 2;  // 実行時に評価される
        case 6:
            return randMain(3) + 2;  // 実行時に評価される
        case 7:
            return randMain(4) + 4;  // 実行時に評価される
        default:
            return 0;
    }
}


constexpr int mem[0x89] = {
        0x00000016,
        0x0000000d,
        0x00000003,
        00000000,
        0x00000001,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        0x0000000a,
        00000000,
        00000000,
        00000000,
        0x00000002, //14 = 0 * 0xc + 0x1c
        00000000,
        0x00000033,
        00000000,
        0x00000003,
        00000000,
        0x00000004, //20 = 1 * 0xc + 0x1c
        00000000,
        0x00000034,
        00000000,
        0x00000003,
        00000000,
        0x00000004,  //26 = 2 * 0xc + 0x1c
        00000000,
        0x00000037,
        00000000,
        0x00000003,
        00000000,
        0x00000004, //32 = 3 * 0xc + 0x1c
        00000000,
        0x0000002d,
        00000000,
        0x00000002,
        00000000,
        0x00000004, //38 = 4 * 0xc + 0x1c
        00000000,
        0x00000028,
        00000000,
        0x00000004,
        00000000,
        0x00000002, //44 = 5 * 0xc + 0x1c
        00000000,
        0x00000033,
        00000000,
        0x00000006,
        00000000,
        0x00000004, //50 = 6 * 0xc + 0x1c
        00000000,
        0x00000034,
        00000000,
        0x00000005,
        00000000,
        0x00000004, //56 = 7 * 0xc + 0x1c
        00000000,
        0x00000037,
        00000000,
        0x00000005,
        00000000,
        0x00000004, //62 = 8 * 0xc + 0x1c
        00000000,
        0x0000002d,
        00000000,
        0x00000004,
        00000000,
        0x00000004, //68 = 9 * 0xc + 0x1c
        00000000,
        0x000000de,
        00000000,
        0x00000008,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
        00000000,
};
int mem_active[0x89] = {0};


template<size_t offset>
constexpr size_t BYTE_OFFSET() {
    static_assert((offset) % 2 == 0, "Offset must be a multiple of 8");
    return ((offset) / 2);
}

#define DynamicOffset(X) (X / 2)

#define CONDITIONAL_ASSIGN(mem_active, mem, offset)             \
    do {                                                        \
        constexpr int byteOffset = BYTE_OFFSET<offset>();       \
        int newValue = getMemConst(mem[byteOffset]);            \
        if (mem[byteOffset] != 0&&mem[byteOffset] != 1&&mem[byteOffset] != 2&&mem[byteOffset] != 3&&mem[byteOffset] != 8) {                      \
            mem_active[byteOffset] = newValue;                  \
        }                                                       \
    } while (0)


constexpr int encmem[0xb] = {
        mem[DynamicOffset(0 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(1 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(2 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(3 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(4 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(5 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(6 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(7 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(8 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(9 * DynamicOffset(0xc) + 0x1c)],
        mem[DynamicOffset(0xc4)],
    };

// リストを静的に生成する constexpr 関数
template<int size, int total = 0>
constexpr auto gen_mon_list() {
    constexpr int max_size = 100;  // 最大サイズ（encmem に合わせて調整）
    std::array<int, max_size> list = {};  // std::array でリストを作成

    int counter = 0;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < encmem[i]; ++j) {
            list[counter++] = i;
        }
    }
    return list;
}

// マクロでリストの生成を呼び出す
#define GEN_MON_LIST(size) gen_mon_list<size>()


int mon_list_couner = 0;
int mon_list[100];


// マクロでリストの生成を呼び出す
#define GEN_MON_LIST(size) gen_mon_list<size>()

void processEnc() {
    memcpy(mem_active, mem, sizeof(mem_active));
    bool tomadoi = false;
    bool ikari = false;
    if (randMain(0x20) == 0) {
        tomadoi = true;
    } else if (randMain(0x20) == 0) {
        ikari = true;
    }

    CONDITIONAL_ASSIGN(mem_active, mem, 0x24);
    CONDITIONAL_ASSIGN(mem_active, mem, 0x30);
    CONDITIONAL_ASSIGN(mem_active, mem, 0x3c);
    CONDITIONAL_ASSIGN(mem_active, mem, 0x48);
    CONDITIONAL_ASSIGN(mem_active, mem, 0x54);
    CONDITIONAL_ASSIGN(mem_active, mem, 0x60);
    CONDITIONAL_ASSIGN(mem_active, mem, 0x6c);
    CONDITIONAL_ASSIGN(mem_active, mem, 0x78);
    CONDITIONAL_ASSIGN(mem_active, mem, 0x84);
    CONDITIONAL_ASSIGN(mem_active, mem, 0x90);


//    int size = 0;
//    for (int i = 0; i < 10; ++i) {
//        if (encmem[i] == 0){
//            break;
//        }
//        std::cout << encmem[i] << std::endl;
//        size = i;
//    }

//    for (int i = 0; i < mon_list_couner; ++i) {
//        std::cout << mon_list[i] << std::endl;
//    }

    constexpr auto list = GEN_MON_LIST(10);

    // 外部で抽選処理
    int result = list[randMain(100)];
    std::cout << "抽選結果: " << result << std::endl;
    //0x0203558c

}

// TIP Press <shortcut actionId="Debug"/> to start debugging your code.
// We have set one <icon src="AllIcons.Debugger.Db_set_breakpoint"/>
// breakpoint for you, but you can always add more by pressing
// <shortcut actionId="ToggleLineBreakpoint"/>.
// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    std::cin.tie(0)->sync_with_stdio(0);

    auto t0 = std::chrono::high_resolution_clock::now();
    uint32_t base1 = 0x7e9056a0;

    //mon_list_couner = gen_mon_list(mon_list);

    processEnc();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto elapsed_time =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "elapsed time: " << double(elapsed_time) / 1000 << " ms" << std::endl;

    std::cout << position << std::endl;

    return 0;
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


