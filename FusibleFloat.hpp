#include <cmath> // for std::fma()

// #define FUSIBLE_FLOAT_FUSE_LVALUE
#ifndef FUSIBLE_FLOAT_NO_FUSE_RVALUE
#  define FUSIBLE_FLOAT_FUSE_RVALUE
#endif
// #define FUSIBLE_FLOAT_ROUND_LEFT_PRODUCT

#ifndef FUSIBLE_FLOAT_NO_ALIASE
#  ifndef FUSIBLE_FLOAT_FLOAT_ALIAS
#    define FUSIBLE_FLOAT_FLOAT_ALIAS Ffloat
#  endif
#  ifndef FUSIBLE_FLOAT_DOUBLE_ALIAS
#    define FUSIBLE_FLOAT_DOUBLE_ALIAS Fdouble
#  endif
#endif

template <typename Float>
class FusibleFloat{
public:
	FusibleFloat(const Float val) : val(val) {}

	operator Float() const { return val; }

private:
	struct FusibleProduct;

public:
	FusibleProduct operator*(const FusibleFloat &rhs) const {
		return {val, rhs.val};
	}
#ifdef FUSIBLE_FLOAT_FUSE_LVALUE
	FusibleProduct operator*(const Float &rhs) const {
		return {val, rhs};
	}

	friend FusibleProduct operator*(const Float &lhs, const FusibleFloat &rhs){
		return {lhs, rhs.val};
	}
#else
	Float operator*(const Float &rhs) const {
		return (val * rhs);
	}

	friend Float operator*(const Float &lhs, const FusibleFloat &rhs){
		return (lhs * rhs.val);
	}
#endif

#ifdef FUSIBLE_FLOAT_FUSE_RVALUE
	FusibleProduct
	operator*(Float &&rhs) const {
		return {val, rhs};
	}

	friend FusibleProduct
	operator*(Float &&lhs, const FusibleFloat &rhs){
		return {lhs, rhs.val};
	}
#endif

private:
	Float val;
	struct FusibleProduct{
		Float a, b;
		operator Float() const {
			return a * b;
		}

		Float operator+(const Float &rhs) const {
			return std::fma(a, b, rhs);
		}

		Float operator-(const Float &rhs) const {
			return std::fma(a, b, -rhs);
		}

		friend Float operator+(const Float &lhs, const FusibleProduct &rhs){
			return std::fma(rhs.a, rhs.b, lhs);
		}

		friend Float operator-(const Float &lhs, const FusibleProduct &rhs){
			return std::fma(-rhs.a, rhs.b, lhs);
		}

		friend Float &operator+=(Float &lhs, const FusibleProduct &rhs){
			lhs = std::fma(rhs.a, rhs.b, lhs);
			return lhs;
		}

		friend Float &operator-=(Float &lhs, const FusibleProduct &rhs){
			lhs = std::fma(-rhs.a, rhs.b, lhs);
			return lhs;
		}
#ifdef FUSIBLE_FLOAT_ROUND_LEFT_PRODUCT
		Float operator+(const FusibleProduct &rhs){
			return Float(*this) + rhs;
		}

		Float operator-(const FusibleProduct &rhs){
			return Float(*this) - rhs;
		}
#endif
	};
};

#ifndef FUSIBLE_FLOAT_NO_ALIASE
using FUSIBLE_FLOAT_FLOAT_ALIAS = FusibleFloat<float>;
using FUSIBLE_FLOAT_DOUBLE_ALIAS = FusibleFloat<double>;
#endif
