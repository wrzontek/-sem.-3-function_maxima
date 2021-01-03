#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H
#include <set>
#include <memory>
#include <stdexcept>


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
            return p1.arg() < p2.arg(); //TODO try catch w tym i reszcie?
        }
        bool operator() (const A &a, const point_type &p2) const {
            return a < p2.arg();
        }
        bool operator() (const point_type &p2, const A &a) const {
            return p2.arg() < a;
        }
    };
    struct cmpV {
        using is_transparent = void;
        bool operator() (const point_type &p1, const point_type &p2) const {
            return p1.value() > p2.value();
        }
        bool operator() (const V &v, const point_type &p2) const {
            return v > p2.value();
        }
        bool operator() (const point_type &p2, const V &v) const {
            return p2.value() > v;
        }
    };

    std::set<point_type, cmpA> points; // ten po A rosnąco
    std::set<point_type, cmpV> maxima; // ten po V malejąco
};

using InvalidArg = std::invalid_argument;

template<typename A, typename V>
V const& FunctionMaxima<A, V>::value_at(const A &a) const {
    iterator point_it;
    try {
        point_it = points.find(a);
    } catch(...) {
        throw;
    }
    if (point_it == points.end())
        throw InvalidArg("invalid argument value");

    return point_it->value();
}

template<typename A, typename V>
void FunctionMaxima<A, V>::set_value(const A &a, const V &v) {
    iterator point_it;
    try {
        point_it = points.find(a); // porównanie moze wywalić
    } catch (...) {
        throw;
    }
    if (point_it == points.end()) {
        try {
            points.insert(point_type(a,v));
        } catch (...) {
            throw;
        }
        //TODO maxima
    } else {
        //TODO tu zapewnić error correctness
        points.erase(point_it);
        points.insert(point_type(a,v));
        // ^ to nie jest najfajniesze ale nie wiem czy da sie to zrobić lepiej
        //TODO maxima
    }
}

template<typename A, typename V>
void FunctionMaxima<A, V>::erase(const A &a) {
    auto point_it = points.find(a);
    if (point_it != points.end()) {
        try {
            points.erase(point_it);
        } catch (...) {
            throw;
        }
    }
    //TODO error correctness tutaj
    //TODO maxima
}


#endif //FUNCTION_MAXIMA_H
