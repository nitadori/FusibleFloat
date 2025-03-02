# はじめに
`fma()` 関数ではなく演算子 `+ - *` で数式に近い記法でソースを書きつつ FMA (fused multiply-add)
命令を使いたい、でもコンパイラが勝手に精度を変える最適化をする[^1]のは嫌といった要望に答える C++ クラスです。

[^1]: FP contract （短縮、契約ではなくて式を短縮するという意味）といって C 言語の仕様ではそういった最適化も認められているそうですが、そういうのは
off にして使うことを想定しています。

# 使い方

```C++
template <typename Float> class FusibleFloat;
using Ffloat = FusibleFloat<float>;
using Fdouble = FusibleFloat<double>;
```
のようにしてあるので、 `float/double` に代えて `Ffloat/Fdouble` 型で式を書くと勝手に `std::fma()` を呼んでくれます。元の
`float/double` に対しては[暗黙の型変換](FusibleFloat.hpp#L21-L23)を定義してあるので相互に代入や変換が可能です。なお、
`Ffloat/Fdouble` 以外の名前を使いたいときは `FUSIBLE_FLOAT_NO_TYPE_ALIASE` を定義した状態で `include` してください。
