#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H
#include <set>
#include <memory>
#include <stdexcept>
#include <vector>
#include <iostream>

template<typename A, typename V>
class FunctionMaxima {
public:
    FunctionMaxima() = default;
    FunctionMaxima(const FunctionMaxima<A, V> &other);
    FunctionMaxima& operator=(const FunctionMaxima<A, V> &other);

    V const &value_at(A const &a) const; // throw InvalidArg?
    void set_value(A const &a, V const &v);
    void erase(A const &a);

    class point_type {
    public:
        A const& arg() const { return *a_ptr.get(); }
        V const& value() const { return *v_ptr.get(); }


        point_type(const point_type &other): a_ptr(other.a_ptr), v_ptr(other.v_ptr) {} // TODO ?
        point_type& operator=(const point_type &other) {
            this->a_ptr = other.a_ptr;
            this->v_ptr = other.v_ptr; // TODO ?
        }

        point_type(const A &a, const V &v) { // TODO to nie powinno być publiczne ale nie wiem jak to zrobić inaczej
            a_ptr = std::make_shared<A> (a);
            v_ptr = std::make_shared<V> (v);
        }
    private:
        std::shared_ptr<A> a_ptr;
        std::shared_ptr<V> v_ptr;
    };

    using iterator =  typename std::set<point_type>::iterator;
    using mx_iterator =  typename std::set<point_type>::iterator;

    iterator begin() const { return points.begin(); }
    iterator end() const { return points.end(); }
    iterator find(A const &a) const { return points.find(a); }

    mx_iterator mx_begin() const { return maxima.begin(); }
    mx_iterator mx_end() const { return maxima.end(); }

    using size_type = size_t;
    [[nodiscard]] size_type size() const {
        return points.size(); // ?
    }

private:
    struct cmpA {
        using is_transparent = void;
        bool operator() (const point_type &p1, const point_type &p2) const {
            if (p1.arg() == p2.arg())
                return p1.value() > p2.value();
            else
                return p1.arg() < p2.arg();
        }
        bool operator() (const A &a, const point_type &p2) const {
            return a < p2.arg(); //todo tu chyba nie trzeba try catch bo set to i tak throwuje nie?
        }
        bool operator() (const point_type &p2, const A &a) const {
            return p2.arg() < a;
        }
    };

    struct cmpV {
        using is_transparent = void;
        bool operator() (const point_type &p1, const point_type &p2) const {
            if (p1.value() == p2.value())
                return p1.arg() < p2.arg();
            else
                return p1.value() > p2.value();
        }
    };

    bool is_maximum(const iterator &point_it, const iterator &erased);

    std::set<point_type, cmpA> points; // ten po A rosnąco
    std::set<point_type, cmpV> maxima; // ten po V malejąco (jak V równe to po A rosnąco)
};

using InvalidArg = std::invalid_argument;

template<typename A, typename V>
V const& FunctionMaxima<A, V>::value_at(const A &a) const {
    iterator point_it = points.find(a);

    if (point_it == points.end())
        throw InvalidArg("invalid argument value");

    return point_it->value();
}

template<typename A, typename V>
bool FunctionMaxima<A, V>::is_maximum(const FunctionMaxima::iterator &point_it, const FunctionMaxima::iterator &erased) {
    bool cond1 = false, cond2 = false;
    if (point_it == points.begin()) {
        cond1 = true;
    } else {
        auto previous = point_it;
        --previous;
        if (previous == erased) {
            if (previous == points.begin())
                cond1 = true;
            else
                --previous;
        }
        if (!cond1 and point_it->value() >= previous->value())
            cond1 = true;
    }

    if (cond1) {
        if (point_it == --points.end()) {
            cond2 = true;
        } else {
            auto next = point_it;
            ++next;
            if (next == erased) {
                if (next == --points.end())
                    cond2 = true;
                else
                    ++next;
            }
            if (!cond2 and point_it->value() >= next->value())
                cond2 = true;
        }
    }

    return (cond1 and cond2);

}

template<typename A, typename V>
void FunctionMaxima<A, V>::set_value(const A &a, const V &v) { // zapewnia silną odporność na błędy
    iterator max_it;
    iterator point_to_erase = points.end();
    iterator maxima_to_erase[3] = {maxima.end(), maxima.end(), maxima.end()};
    iterator added_max[3] = {maxima.end(), maxima.end(), maxima.end()};
    // nie można od razu erase bo nie da sie tego nothrow cofnac jak sie cos dalej wywróci
    // trzeba zbierac iteratory na to co jest do wywalenia i zrobic to na koncu (erase(iterator) jest nothrow)

    iterator point_it = points.find(a); // jak wywali to nie szkodzi, nie ma jeszcze zmian

    if (point_it != points.end()) {
        if (point_it->value() == v)
            return; // nic sie nie zmieniło

        point_to_erase = point_it;
        max_it = maxima.find(*point_it);

        if (max_it != maxima.end())
            maxima_to_erase[0] = max_it;
    }

    point_type new_point = point_type(a,v);
    std::pair<iterator, bool> inserted = points.insert(new_point);
    point_it = inserted.first;

    if (is_maximum(point_it, point_to_erase)) {
        try {
            inserted = maxima.insert(new_point);
            added_max[0] = inserted.first;
        } catch (...) {
            points.erase(point_it);
            throw;
        }
    }

    if (point_it != points.begin()) {
        auto prev = point_it;
        prev--;
        if (is_maximum(prev, point_to_erase)) {
            try {
                inserted = maxima.insert(*prev);
                added_max[1] = inserted.first;
            } catch (...) {
                points.erase(point_it);
                if (added_max[0] != maxima.end())
                    maxima.erase(added_max[0]);
                throw;
            }
        } else {
            try {
                max_it = maxima.find(*prev);
            } catch (...) {
                points.erase(point_it);
                if (added_max[0] != maxima.end())
                    maxima.erase(added_max[0]);
                throw;
            }
            if (max_it != maxima.end())
                maxima_to_erase[1] = max_it;
        }
    }

    if (point_it != --points.end()) {
        auto next = point_it;
        next--;
        if (is_maximum(next, point_to_erase)) {
            try {
                inserted = maxima.insert(*next);
                added_max[2] = inserted.first;
            } catch (...) {
                points.erase(point_it);
                if (added_max[0] != maxima.end())
                    maxima.erase(added_max[0]);
                if (added_max[1] != maxima.end())
                    maxima.erase(added_max[1]);
                throw;
            }
        } else {
            try {
                max_it = maxima.find(*next);
            } catch (...) {
                points.erase(point_it);
                if (added_max[0] != maxima.end())
                    maxima.erase(added_max[0]);
                if (added_max[1] != maxima.end())
                    maxima.erase(added_max[1]);
                throw;
            }
            if (max_it != maxima.end())
                maxima_to_erase[2] = max_it;
        }
    }

    if (point_to_erase != points.end()) {
        points.erase(point_to_erase); //nothrow
    }
    for (iterator it: maxima_to_erase)
        if (it != maxima.end())
            maxima.erase(it); //nothrow
}

template<typename A, typename V>
void FunctionMaxima<A, V>::erase(const A &a) { // zapewnia silną odporność na błędy
    auto point_it = points.find(a);
    if (point_it == points.end())
        return;

    iterator max_it;
    iterator maxima_to_erase[3] = {maxima.end(), maxima.end(), maxima.end()};
    iterator added_max[2] = {maxima.end(), maxima.end()};
    std::pair<iterator, bool> inserted;

    max_it = maxima.find(*point_it);
    if (max_it != maxima.end())
        maxima_to_erase[0] = max_it;

    if (point_it != points.begin()) {
        auto prev = point_it;
        prev--;
        if (is_maximum(prev, point_it)) {
            inserted = maxima.insert(*prev);
            added_max[0] = inserted.first;

        } else {
            max_it = maxima.find(*prev);
            if (max_it != maxima.end())
                maxima_to_erase[1] = max_it;
        }
    }

    if (point_it != --points.end()) {
        auto next = point_it;
        next--;
        if (is_maximum(next, point_it)) {
            try {
                inserted = maxima.insert(*next);
                added_max[1] = inserted.first;
            } catch (...) {
                if (added_max[0] != maxima.end())
                    maxima.erase(added_max[0]);
                throw;
            }
        } else {
            try {
                max_it = maxima.find(*next);
            } catch (...) {
                if (added_max[0] != maxima.end())
                    maxima.erase(added_max[0]);
                throw;
            }
            if (max_it != maxima.end())
                maxima_to_erase[2] = max_it;
        }
    }

    points.erase(point_it); //nothrow
    for (iterator it: maxima_to_erase)
        if (it != maxima.end())
            maxima.erase(it); //nothrow
}




#endif //FUNCTION_MAXIMA_H
