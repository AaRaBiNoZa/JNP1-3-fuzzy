#ifndef FUZZY_H
#define FUZZY_H

#include <iostream>
#include <vector>
#include <compare>
#include <tuple>
#include <cmath>
#include <set>

using real_t = double;

class TriFuzzyNum {
private:
    mutable std::tuple<real_t, real_t, real_t> rank;
    mutable bool is_rank_computed;
    real_t l, m, u;

public:
    //Konstruktory
    TriFuzzyNum() = delete;

    constexpr ~TriFuzzyNum() = default;

    constexpr TriFuzzyNum(TriFuzzyNum const &) = default;

    constexpr TriFuzzyNum(real_t a, real_t b, real_t c)
            : rank(0, 0, 0), is_rank_computed(false), l(a), m(b), u(c) {
        adjust_values();
    }

    constexpr TriFuzzyNum(TriFuzzyNum &&other) noexcept
            : rank(std::move(other.rank)),
              is_rank_computed(other.is_rank_computed),
              l(other.l),
              m(other.m),
              u(other.u) {}
    //Koniec konstruktorów


    //Operatory
    TriFuzzyNum &operator=(const TriFuzzyNum &) = default;

    TriFuzzyNum &operator=(TriFuzzyNum &&other) noexcept {
        if (this != &other) {
            rank = std::move(other.rank);
            is_rank_computed = other.is_rank_computed;
            l = other.l;
            m = other.m;
            u = other.u;
        }
        return *this;
    }

    TriFuzzyNum &operator+=(TriFuzzyNum const &rhs) {
        l += rhs.lower_value();
        m += rhs.modal_value();
        u += rhs.upper_value();

        is_rank_computed = false;

        return *this;
    }

    TriFuzzyNum &operator-=(TriFuzzyNum const &rhs) {
        l -= rhs.upper_value();
        m -= rhs.modal_value();
        u -= rhs.lower_value();

        is_rank_computed = false;
        return *this;
    }

    TriFuzzyNum &operator*=(TriFuzzyNum const &rhs) {
        l *= rhs.lower_value();
        m *= rhs.modal_value();
        u *= rhs.upper_value();

        is_rank_computed = false;
        adjust_values();

        return *this;
    }

    TriFuzzyNum operator+(const TriFuzzyNum &other) const {
        TriFuzzyNum result(*this);
        result += other;
        return result;
    }

    TriFuzzyNum operator-(const TriFuzzyNum &other) const {
        TriFuzzyNum result(*this);
        result -= other;
        return result;
    }

    TriFuzzyNum operator*(const TriFuzzyNum &other) const {
        TriFuzzyNum result(*this);
        result *= other;
        return result;
    }

    auto operator<=>(TriFuzzyNum const &other) const {
        if (!is_rank_computed)
            compute_rank();
        if (!other.is_rank_computed)
            other.compute_rank();

        return rank <=> other.rank;
    }

    constexpr bool operator==(TriFuzzyNum const &other) const {
        return l == other.l and m == other.m and u == other.u;
    }

    bool operator!=(TriFuzzyNum const &other) const {
        return !(*this == other);
    }
    //Koniec operatorów


    //Metody
    constexpr real_t lower_value() const {
        return l;
    }

    constexpr real_t modal_value() const {
        return m;
    }

    constexpr real_t upper_value() const {
        return u;
    }

    void compute_rank() const {
        real_t z = (u - l) + sqrt(1 + (u - m) * (u - m)) +
                   sqrt(1 + (m - l) * (m - l));
        real_t y = (u - l) / z;
        real_t x = ((u - l) * m + sqrt(1 + (u - m) * (u - m)) * l +
                    sqrt(1 + (m - l) * (m - l)) * u) / z;

        rank = std::make_tuple(x - y / 2, 1 - y, m);
        is_rank_computed = true;
    }

    constexpr void adjust_values() {
        real_t args[3] = {l, m, u};
        std::sort(std::begin(args), std::end(args));
        l = args[0];
        m = args[1];
        u = args[2];
    }
    //Koniec metod
};

std::ostream &operator<<(std::ostream &os, const TriFuzzyNum &num) {
    os << '('
       << num.lower_value() << ", "
       << num.modal_value() << ", "
       << num.upper_value()
       << ')';

    return os;
}


class TriFuzzyNumSet {
private:
    std::multiset<TriFuzzyNum> set;
    real_t sum_l, sum_m, sum_u;

public:

    TriFuzzyNumSet() : sum_l(0), sum_m(0), sum_u(0) {}

    TriFuzzyNumSet(const TriFuzzyNumSet &other) = default;

    TriFuzzyNumSet(TriFuzzyNumSet &&other) noexcept
            : set(std::move(other.set)),
              sum_l(other.sum_l),
              sum_m(other.sum_m),
              sum_u(other.sum_u) {}

    TriFuzzyNumSet(std::initializer_list<TriFuzzyNum> args)
            : sum_l(0), sum_m(0), sum_u(0) {
        for (const auto &arg: args) {
            insert(arg);
        }
    }
    //Koniec konstruktorów


    //Operatory
    TriFuzzyNumSet &operator=(const TriFuzzyNumSet &) = default;

    TriFuzzyNumSet &operator=(TriFuzzyNumSet && other)  noexcept {
        if (this != &other) {
            set = std::move(other.set);
            sum_l = other.sum_l;
            sum_m = other.sum_m;
            sum_u = other.sum_u;
        }
        return *this;
    };
    //Koniec operatorów


    //Metody
    void insert(const TriFuzzyNum &num) {
        sum_l += num.lower_value();
        sum_m += num.modal_value();
        sum_u += num.upper_value();
        set.insert(num);
    }

    void insert(TriFuzzyNum &&num) {
        sum_l += num.lower_value();
        sum_m += num.modal_value();
        sum_u += num.upper_value();
        set.insert(std::move(num));
    }

    void remove(const TriFuzzyNum &num) {
        size_t erased_el = set.erase(num);
        sum_l -= erased_el * num.lower_value();
        sum_m -= erased_el * num.modal_value();
        sum_u -= erased_el * num.upper_value();
    }

    TriFuzzyNum arithmetic_mean() {
        if (set.empty())
            throw std::length_error(
                    "TriFuzzyNumSet::arithmetic_mean - the set is empty.");

        size_t set_size = set.size();
        return TriFuzzyNum(sum_l / set_size,
                           sum_m / set_size, sum_u / set_size);
    }
    //Koniec metod
};

consteval TriFuzzyNum crisp_number(real_t v) {
    return TriFuzzyNum(v, v, v);
}

constinit TriFuzzyNum crisp_zero = crisp_number(0);

#endif


