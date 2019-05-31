/// PREDEFINED ESSENTIAL..., you know exactly what does that mean
#ifndef PERV_H
#define PERV_H

#define delegate_type_to_base(class_, type_, base_) \
    class_(const type_& other) : base_(other) {} \
    class_(type_&& other) : base_(other)

#define delegate_to_base(class_, base_) delegate_type_to_base(class_, base_, base_)

template <typename T>
class uncopiable : public T {
    uncopiable(const uncopiable&);
public:
    delegate_to_base(uncopiable, T) {}
};

template <typename Rubbish>
struct cleaner : public uncopiable<Rubbish> {
    delegate_type_to_base(cleaner, Rubbish, uncopiable<Rubbish>) {}
    ~cleaner() { Rubbish::clear(); }
};


#endif // PERV_H
