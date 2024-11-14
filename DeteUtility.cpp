//
// Created by Owner on 2024/11/12.
//

#include <cstdint>
#include <chrono>
#include "DeteUtility.h"

// 除算を使った計算
inline int DeteUtility::div_day_adjust(int day) {
    return day + (day / 0xA) * 6;
}

std::uint32_t DeteUtility::encodeDate(int year1, int month, int day) {
    using namespace std::chrono;
    if (year1 < 2000||year1 > 2099) {
        throw std::out_of_range("Invalid Date input");
    }

    // 年月日の変換 (C++20では、西暦を直接扱える)
    auto date = year{year1} / month / day;

    // 曜日の計算
    std::chrono::weekday weekday{date};; // 0: 日曜日〜6: 土曜日

    int year2 = year1 - 2000;

    //10 = 0x10
    month = DeteUtility::div_day_adjust(month);
    day = DeteUtility::div_day_adjust(day);
    year2 = DeteUtility::div_day_adjust(year2);


    // 年を2桁に短縮してビットシフトで16進数相当の整数を作成
    std::uint32_t encodedDate =
            ((year2 % 0xff) << 0) |    // 年の下2桁を最下位バイトに格納
            ((month & 0xFF) << 8) |    // 月を8ビット左シフト
            ((day & 0xFF) << 16) |    // 日を16ビット左シフト
            ((weekday.c_encoding() & 0xFF) << 24); // 曜日を24ビット左シフト

    return encodedDate;
}

// 時、分、秒のスキップを考慮したエンコード
std::uint32_t DeteUtility::encodeTime(int hour, int minute, int second) {
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