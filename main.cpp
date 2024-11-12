#include <iostream>
#include <cstdint>

#include <cassert>
#include <unordered_set>
#include <chrono>
#include "Lcg.h"
#include "debug.h"
#include "DeteUtility.h"

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
            return Lcg::randMain(2) + 1;  // 実行時に評価される
        case 5:
            return Lcg::randMain(2) + 2;  // 実行時に評価される
        case 6:
            return Lcg::randMain(3) + 2;  // 実行時に評価される
        case 7:
            return Lcg::randMain(4) + 4;  // 実行時に評価される
        default:
            return 0;
    }
}


constexpr int mem[0xf8] = {
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
int mem_active[0xf8] = {0};


template<size_t offset>
constexpr size_t BYTE_OFFSET() {
    static_assert((offset) % 2 == 0, "Offset must be a multiple of 8");
    return ((offset) / 2);
}

#define DynamicOffset(X) ((X) / 2)
#define CheckDynamicOffset(X) (assert((X) % 2 == 0))

#define CONDITIONAL_ASSIGN(mem_active, mem, offset)             \
    do {                                                        \
        constexpr int byteOffset = BYTE_OFFSET<offset>();       \
        int newValue = getMemConst(mem[byteOffset]);            \
        if (mem[byteOffset] != 0&&mem[byteOffset] != 1&&mem[byteOffset] != 2&&mem[byteOffset] != 3&&mem[byteOffset] != 8) {                      \
            mem_active[byteOffset] = newValue;                  \
        }                                                       \
    } while (0)


constexpr int encmem[0xb] = {
        mem[DynamicOffset(0 * 0xc + 0x1c)],
        mem[DynamicOffset(1 * 0xc + 0x1c)],
        mem[DynamicOffset(2 * 0xc + 0x1c)],
        mem[DynamicOffset(3 * 0xc + 0x1c)],
        mem[DynamicOffset(4 * 0xc + 0x1c)],
        mem[DynamicOffset(5 * 0xc + 0x1c)],
        mem[DynamicOffset(6 * 0xc + 0x1c)],
        mem[DynamicOffset(7 * 0xc + 0x1c)],
        mem[DynamicOffset(8 * 0xc + 0x1c)],
        mem[DynamicOffset(9 * 0xc + 0x1c)],
        mem[DynamicOffset(0xc4)],
};

// リストを静的に生成し、要素数カウンタも返す constexpr 関数
template<int size>
constexpr auto gen_mon_list() {
    constexpr int max_size = 100;  // 最大サイズ（encmem に合わせて調整）
    std::array<int, max_size> list = {};  // std::array でリストを作成
    int counter = 0;

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < encmem[i]; ++j) {
            list[counter++] = i;
        }
    }

    return std::make_pair(list, counter);  // 配列とカウンタのペアを返す
}

constexpr int encmem1G[4] = {3, 0, 0, 2};//FUN_020356a8_read_enc2

template<int size>
constexpr auto gen_mon_list1G() {
    constexpr int max_size = 100;  // 最大サイズ（encmem に合わせて調整）
    std::array<int, max_size> list = {};  // std::array でリストを作成
    int counter = 0;

    for (int i = 0; i < size; ++i) {
        if (encmem1G[i] == 0) {
            break;
        }
        for (int j = 0; j < encmem1G[i]; ++j) {
            list[counter++] = i;
        }
    }

    return std::make_pair(list, counter);  // 配列とカウンタのペアを返す
}

// マクロでリストとサイズを生成
#define GEN_MON_LIST(size) gen_mon_list<size>()
#define GEN_MON_LIST1G(size) gen_mon_list1G<size>()

// グローバルスコープでリストとサイズを生成
constexpr auto mon_data = GEN_MON_LIST(10);
constexpr auto &mon_list = mon_data.first;
constexpr int mon_size = mon_data.second;

constexpr auto mon_data1G = GEN_MON_LIST1G(3);
constexpr auto &mon_list1G = mon_data1G.first;
constexpr int mon_size1G = mon_data1G.second;


constexpr auto mon_dataOtomo = GEN_MON_LIST(5);
constexpr auto &mon_list_Otomo = mon_dataOtomo.first;
constexpr int mon_sizeO_tomo = mon_dataOtomo.second;


constexpr int64_t enc_walk[0x1f] = {
        0xfffffb57 - 0xffffffffLL,
        0xfffffc2a - 0xffffffffLL,
        0xfffffcb4 - 0xffffffffLL,
        0xfffffd1f - 0xffffffffLL,
        0xfffffd79 - 0xffffffffLL,
        0xfffffdc8 - 0xffffffffLL,
        0xfffffe0f - 0xffffffffLL,
        0xfffffe50 - 0xffffffffLL,
        0xfffffe8d - 0xffffffffLL,
        0xfffffec7 - 0xffffffffLL,
        0xfffffefe - 0xffffffffLL,
        0xffffff34 - 0xffffffffLL,
        0xffffff68 - 0xffffffffLL,
        0xffffff9b - 0xffffffffLL,
        0xffffffcd - 0xffffffffLL,
        0,
        0x00000032,
        0x00000064,
        0x00000097,
        0x000000cb,
        0x00000101,
        0x00000138,
        0x00000172,
        0x000001af,
        0x000001f0,
        0x00000237,
        0x00000286,
        0x000002e0,
        0x0000034b,
        0x000003d5,
        0x000004a8,
};

bool FUN_02035740(int param2, int param3) {
    auto counter = 0;
    auto param1Counter = 0;
    do {
        if (counter < param3) {
            auto int_couner = mem_active[DynamicOffset(param1Counter + 0xec)];
            if (int_couner == 0) {
                auto var2 = counter << 1;
                CheckDynamicOffset(var2 + 0xe4);
                CheckDynamicOffset(param2 * 0xc + 0x2);
                CheckDynamicOffset(var2 + 0xec);
                CheckDynamicOffset(var2 + 0xf4);
                CheckDynamicOffset(var2 + 0xf4);
                mem_active[DynamicOffset(var2 + 0xe4)] = mem_active[DynamicOffset(param2 * 0xc + 0x20)]; // モンスター id
                mem_active[DynamicOffset(var2 + 0xec)] = mem_active[DynamicOffset(var2 + 0xec)] + 1; // モンスター数
                mem_active[DynamicOffset(var2 + 0xf4)] = param2;
                return true;
            }//45 = 3
            auto pre = mem_active[DynamicOffset(param1Counter + 0xf4)];
            auto count1 = (mem_active[DynamicOffset(param2 * 0xc + 0x24)] & 0xffff);
            if (param2 == pre &&
                int_couner < count1) {
                auto var2 = counter << 1;
                mem_active[DynamicOffset(0xec + var2)] = mem_active[DynamicOffset(0xec + var2)] + 1;
                return true;
            }
        }
        counter++;
        param1Counter += 2;
    } while (4 > counter);
    return false;
}



void processEnc() {
//    if (position == 141) {
//        std::cout << "!!" << std::endl;
//    }
    memcpy(mem_active, mem, sizeof(mem_active));
    auto tomadoi = false;
    if (Lcg::randMain(0x20) == 0) {
        tomadoi = true;
    } else {
        Lcg::randMain(0x20);
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
    //0x0203558c
    auto rand = Lcg::randMain(mon_size);
    int selected = mon_list[rand];
    mem_active[DynamicOffset(0x18)] = selected;


    if (selected == 10) {
        // FUN_02035598
        //todo 拡張データーロード
    } else if (selected > 4 && selected < 10) {
        //FUN_0203562c
        mem_active[DynamicOffset(0xe4)] = mem_active[DynamicOffset(selected * 0xc + 0x20)]; // モンスターid
        mem_active[DynamicOffset(0xec)] = mem_active[DynamicOffset(selected * 0xc + 0x24)]; // モンスター数
#ifdef DEBUG
        mem_active[DynamicOffset(0xe6)] = 0;
        mem_active[DynamicOffset(0xee)] = 0;
        mem_active[DynamicOffset(0xe8)] = 0;
        mem_active[DynamicOffset(0xf0)] = 0;
        mem_active[DynamicOffset(0xea)] = 0;
        mem_active[DynamicOffset(0xf2)] = 0;
#endif
    } else if (selected < 5) {
        //FUN_0203567c 0203567c
        //todo

        // FUN_020356a8_read_enc2
        auto rand1 = Lcg::randMain(mon_size1G);
        auto test = mon_list1G[rand1] + 2;
        while (FUN_02035740(selected, test)) {
            auto rand2 = Lcg::randMain(mon_sizeO_tomo);
            selected = mon_list_Otomo[rand2];
        }
    }
}

int stepCounter = 0;

bool EmulationMain(uint32_t seed) {
    Lcg::randInit(seed);
    processEnc();
    stepCounter = 0x1e00;
    auto rand = Lcg::randMain(31);
    stepCounter += static_cast<int>(enc_walk[rand]);

    int counter = 0;
    while (stepCounter >= 0) {
        processEnc();
        stepCounter -= 529;
        counter++;
#ifdef DEBUG
        std::cout << "=======" << std::endl;
#endif
    }

    processEnc();
    processEnc();

    auto enc1GId = mem_active[DynamicOffset(0xe4)];
    auto enc1GCount = mem_active[DynamicOffset(0xec)];

    auto enc2GId = mem_active[DynamicOffset(0xe4 + 1 * 2)];
    auto enc2GCount = mem_active[DynamicOffset(0xec + 1 * 2)];

    auto enc3GId = mem_active[DynamicOffset(0xe4 + 2 * 2)];
    auto enc3GCount = mem_active[DynamicOffset(0xec + 2 * 2)];

    auto count = (enc1GCount+enc2GCount+enc3GCount) * 2;
    Lcg::randMainJump86785();
    Lcg::randMainJump(count);

    auto tomadoi = false;
    if (Lcg::randMain(0x20) == 0) {
        tomadoi = true;
    } else {
        Lcg::randMain(0x20);
    }

    if (tomadoi && enc1GId == 45 && enc1GCount == 2 && enc2GId == 0) {//&&enc2GCount == 2&&enc3GId == 45&&enc3GCount == 2 && enc2GCount == 1
        return true;
    }
    return false;
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


    std::unordered_set<uint32_t> known_values;


#ifndef DEBUG
    for (int y = 2000; y < 2099; ++y) {
        for (int m = 1; m < 12; ++m) {
            for (int d = 0; d < 28; ++d) {
                for (int h = 0; h < 24; ++h) {
                    for (int min = 0; min < 60; ++min) {
                        std::uint32_t encodedDate2 = DeteUtility::encodeDate(y, m, d);
                        std::uint32_t encodeTime2 = DeteUtility::encodeTime(h, min, 10);
                        uint32_t seed = base1 + encodedDate2 + encodeTime2;
                        if (!known_values.contains(seed)) {
                            known_values.insert(seed);
                            if (EmulationMain(seed)) {
                                std::cout << seed << ", " << y << "/" << m << "/" << d << " " << h << ":" << min  << ":10"  << ", " << std::hex << "0x" << encodedDate2 << ", " << encodeTime2 << std::dec << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }
#else
    EmulationMain(2225166851);

    std::cout << std::dec << mem_active[DynamicOffset(0xe4)] << "," << mem_active[DynamicOffset(0xec)] << std::endl;
    std::cout << std::dec << mem_active[DynamicOffset(0xe4 + 1 * 2)] << "," << mem_active[DynamicOffset(0xec + 1 * 2)]
              << std::endl;
    std::cout << std::dec << mem_active[DynamicOffset(0xe4 + 2 * 2)] << "," << mem_active[DynamicOffset(0xec + 2 * 2)]
              << std::endl;
    std::cout << std::dec << mem_active[DynamicOffset(0xe4 + 3 * 2)] << "," << mem_active[DynamicOffset(0xec + 3 * 2)]
              << std::endl;
#endif
    auto t1 = std::chrono::high_resolution_clock::now();
    auto elapsed_time =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "elapsed time: " << double(elapsed_time) / 1000 << " ms" << std::endl;
    std::cout << "step: " << std::hex << stepCounter << std::dec << std::endl;
    std::cout << "pos: " << Lcg::getPosition() << std::endl;

    return 0;
}

