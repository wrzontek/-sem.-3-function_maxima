#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H
#include <iterator>

template<typename A, typename V>
class FunctionMaxima {
public:
    FunctionMaxima();
    FunctionMaxima(const FunctionMaxima<A, V> &other);
    FunctionMaxima& operator=(const FunctionMaxima<A, V> &other);

    V const &value_at(A const &a) const throws InvalidArg;
    void set_value(A const &a, V const &v);
    void erase(A const &a);

    using point_type = struct  {
    public:
        A const &arg() const;
        V const &value() const;
        point_type() = delete;
        point_type(const point_type &other);
        point_type& operator=(const point_type &other);
    private:
        // smart wskaźniki na A i V
    }

    class iterator : bidirectional_operator_tag {
    public:
        point_type& operator *();
        bool operator ==(const interator& other);
    };

    iterator begin() const;
    iterator end() const;
    iterator find(A const &a) const;

    class mx_iterator : std::bidirectional_operator_tag {
    public:
        point_type& operator *();
        bool operator ==(const mx_interator& other);
    };

    mx_iterator mx_begin() const;
    mx_iterator mx_end() const;

    typedef size_t size_type;
    size_type size() const;

private:
    std::set<point_type> points;
    std::set<point_type> maxima;

};

class InvalidArg : public std::exception {

};

#endif //FUNCTION_MAXIMA_H
