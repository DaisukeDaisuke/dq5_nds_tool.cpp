//
// Created by Owner on 2024/11/12.
//

#ifndef DQ5_DS_RANDS_DETEUTILITY_H
#define DQ5_DS_RANDS_DETEUTILITY_H


class DeteUtility {
public:
    static std::uint32_t encodeDate(int year1, int month, int day);
    static std::uint32_t encodeTime(int hour, int minute, int second);

private:
    static inline int div_day_adjust(int day);
};


#endif //DQ5_DS_RANDS_DETEUTILITY_H
