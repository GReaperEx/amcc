#ifndef I_REF_COUNTED_H
#define I_REF_COUNTED_H

class IRefCounted
{
public:
    IRefCounted(): refCount(1) {}
    virtual ~IRefCounted() {}

    void grab() const {
        ++refCount;
    }

    void drop() const {
        if (--refCount == 0) {
            delete this;
        }
    }

protected:
    IRefCounted(const IRefCounted&) {}
    IRefCounted& operator= (const IRefCounted&) { return *this; }

private:
    mutable unsigned refCount;
};

#endif // I_REF_COUNTED_H
