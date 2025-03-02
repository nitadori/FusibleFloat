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
`float/double` に対しては[暗黙の型変換](FusibleFloat.hpp#L12-L14)を定義してあるので相互に代入や変換が可能です。なお、
`Ffloat/Fdouble` 以外の名前を使いたいときは `FUSIBLE_FLOAT_NO_TYPE_ALIASE` を定義した状態で `include` してください。

中でやっているのは、 `FusibleFloat` 型に対して `operator*()` が来たら `FusibleProduct`
として両辺の値を保存し、そこに `+` か `-` が来たら `std::fma()` を呼びそれ以外では単に `operator Float()` として掛けて丸めた値を返すというものです。

# 細かな挙動

元の `float/double` は勝手に FMA 化されたくないときに使うことになるので、 `Ffloat/Fdouble` との乗算になったときに
fuse の対象とするかが問題となります。悩んだのですが、
 1. 左辺値（名前のある普通の変数）の `float/double` とは fuse しない
 2. 右辺値（即値、関数の戻り値、式の値など）の `flaot/double` とは fuse する

をデフォルトの挙動としてあります（ `1.5f` のような数値を `Ffloat(1.5f)`
のように書かなければいけないルールだと式が煩雑になって本来の趣旨から離れるため）。これらは
`FUSIBLE_FLOAT_FUSE_LVALUE` や `FUSIBLE_FLOAT_NO_FUSE_RVALUE` を定義した状態で `include`
することで変えることもできます。なお、加減算のオペランド（ `std::fma()` の第3引数に相当）に関してはこのような型による区別は設けておらず、何が来ても
FMA になるようにしてあります。
FusibleProduct を相手に FMA 化を拒む型というのも考えることはできますがユースケースを思いつかなかったため。

# 曖昧さと対策
