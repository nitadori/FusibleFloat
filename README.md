# FusibleFloat 型

## はじめに

`fma()` 関数ではなく演算子 `+ - *` で数式に近い記法でソースを書きつつ FMA (fused multiply-add)
命令を使いたい、でもコンパイラが勝手に精度を変える最適化をする[^1]のは嫌といった要望に答える C++ クラスです。

[^1]: FP contract （短縮、契約ではなくて式を短縮するという意味）といって C 言語の仕様ではそういった最適化も認められているそうですが、そういうのは
off にして使うことを想定しています。

## 使い方

```C++
template <typename Float> class FusibleFloat;
using Ffloat = FusibleFloat<float>;
using Fdouble = FusibleFloat<double>;
```
のようにしてあるので、 `float/double` に代えて `Ffloat/Fdouble` 型で式を書くと勝手に `std::fma()` を呼んでくれます。元の
`float/double` に対しては[暗黙の型変換](FusibleFloat.hpp#L12-L14)を定義してあるので相互に代入や変換が可能です。なお、
`Ffloat/Fdouble` 以外の名前を使いたいときは `FUSIBLE_FLOAT_NO_TYPE_ALIASE` を定義した状態で include してください。

中でやっているのは、 `FusibleFloat` 型に対して `operator*()` が来たら `FusibleProduct`
として両辺の値を保存し、そこに `+` か `-` が来たら `std::fma()` を呼びそれ以外では単に `operator Float()` として掛けて丸めた値を返すというものです。

## 細かな挙動

元の `float/double` は勝手に FMA 化されたくないときに使うことになるので、 `Ffloat/Fdouble` との乗算になったときに
fuse の対象とするかが問題となります。悩んだのですが、
 1. 左辺値（名前のある普通の変数）の `float/double` とは fuse しない
 2. 右辺値（即値、関数の戻り値、式の値など）の `flaot/double` とは fuse する

をデフォルトの挙動としてあります（ `1.5f` のような数値を `Ffloat(1.5f)`
のように書かなければいけないルールだと式が煩雑になって本来の趣旨から離れるため）。これらは
`FUSIBLE_FLOAT_FUSE_LVALUE` や `FUSIBLE_FLOAT_NO_FUSE_RVALUE` を定義した状態で include
することで変えることもできます。なお、加減算のオペランド（ `std::fma()` の第3引数に相当）に関してはこのような型による区別は設けておらず、何が来ても
FMA になるようにしてあります。
FusibleProduct を相手に FMA 化を拒む型というのも考えることはできますがユースケースを思いつかなかったため。

## 曖昧さと対策

`Ffloat a, b, c, d` に対して
`a*b + c*d` という式はデフォルトでは曖昧性のエラーとなります。左の積と右の積のどちらかを丸めてからでないと
FMA にはできず、どちらを優先するとも区別がつかないからです。対処としては、単項演算子 `+` を用いて
`+(a*b) + c*d` とすると左の積については `operator float()` が呼ばれ乗算結果が丸められます。また、
`0.0f + a*b + c*d` のように書くと、厳密には FMA 2回になりますが同様の挙動となります。このように
 FusibleProduct 同士の足し引きで左側を優先して丸める挙動を、 FUSIBLE_FLOAT_ROUND_LEFT_PRODUCT
 を定義しておくことで有効化することもできます。
 
なお `a*b*c + d` のような式では `a*b` が先に評価されこれが `float &&` となって `c` と fuse し、
`fmaf(a*b, c, d)` のような挙動となります。

## [test.cpp](test.cpp) について

網羅的なテストという程にはなっていません。
`std::fma(a, b, -a*b)` のような計算をすると、 `a*b` の計算で発生した丸め誤差を取り出すことができます。これを利用して、
FMA 化されなかった場合は 0 に、された場合には小さな値が表示されるようになっています。実行結果は
```bash
test1
0.000000e+00
-3.576279e-09
0.000000e+00
-3.576279e-09
0.000000e+00
-3.576279e-09
test2
3.576279e-09
-3.576279e-09
3.576279e-09
3.576279e-09
```
となっていますがコンパイラや FPU によっては異なる可能性もあります。

## Tips

単項演算子 `+` に便利な使いどころがあります。
 1. FusibleProduct に対して用いると `operator Float()` の呼び出しになる。明示的に乗算して丸めたい場合、 `printf()` に渡したい場合などに使えます。
 2. 左辺値の `float/double` に対して用いると値はそのまま右辺値にできます。[2次の多項式](test.cpp#L17)はこの書き方で FMA 命令が2回になります。

## 議論

ちゃんとしたライブラリを作りたいのなら `a*b + c` の3つとも `FusibleFloat` のときだけ FMA 化するようにして、戻り値も
`FusibleFloat` に、暗黙の型変換は最小限にするということになろうかとは思いますが、それだと特に `1.5f`
のような即値の扱いが不便になります。なのでせめて rvalue は暗黙のうちに fuse するようにはしてみたのですが、それなら
lvalue も fuse させてしまった方が難しいルールが減ってよかったかもしれません。なお本当の意味で fuse
するのは乗算と加減算であって、そのオペランドが fuse するのではありません。他に適当な名前を思いつかなかったということでご容赦くだせん。

## TODO

ライセンスの追加と~~この説明書の英語版~~

---

# FusibleFloat Datatype

## Introduction

This is a C++ class designed to address the need for using FMA (fused multiply-add) instructions while writing source code with operators `+ - *`, making expressions look closer to mathematical notation. At the same time, it avoids unwanted compiler optimizations that alter precision[^2].

[^2]: In C language specifications, an optimization called FP contract (which refers to expression contraction, not a contractual agreement) allows such optimizations. This implementation assumes that such optimizations are turned off.

## Usage

```C++
template <typename Float> class FusibleFloat;
using Ffloat = FusibleFloat<float>;
using Fdouble = FusibleFloat<double>;
```

By replacing `float/double` with `Ffloat/Fdouble`, expressions will automatically use `std::fma()`. Since implicit type conversions to and from `float/double` are [defined](FusibleFloat.hpp#L12-L14), assignments and conversions between them are possible. If you want to use different names instead of `Ffloat/Fdouble`, define `FUSIBLE_FLOAT_NO_TYPE_ALIASE` before including the header.

Internally, when `operator*()` is applied to `FusibleFloat`, it stores the values in `FusibleProduct`. If `+` or `-` is subsequently used, `std::fma()` is called. Otherwise, `operator Float()` is used to compute the product and round the result.

## Detailed Behavior

Since `float/double` is used when FMA should not be applied, a key issue is whether to fuse multiplication when combined with `Ffloat/Fdouble`. The default behavior is as follows:

1. Multiplication with an lvalue (`float/double` ordinary variable) is **not fused**.
2. Multiplication with an rvalue (`float/double` immediate value, function return value, or temporary result) **is fused**.

This avoids the need to explicitly write `Ffloat(1.5f)` instead of `1.5f`, keeping expressions concise. These behaviors can be modified using `FUSIBLE_FLOAT_FUSE_LVALUE` or `FUSIBLE_FLOAT_NO_FUSE_RVALUE` at the time of inclusion.

For addition and subtraction operands (the third argument of `std::fma()`), no such type distinction is made—everything will be fused.

It is theoretically possible to define a type that refuses to fuse with `FusibleProduct`, but no practical use case was found.

## Ambiguity and Workarounds

Given `Ffloat a, b, c, d`, the expression `a*b + c*d` results in an ambiguity error. This is because one of the products must be rounded before performing FMA, and it is unclear which one should be prioritized. To resolve this:

- Use the unary `+` operator: `+(a*b) + c*d`. This forces `operator float()` to be invoked on the left product, rounding the multiplication result.
- Write `0.0f + a*b + c*d`. This results in two `std::fma()` operations but achieves the same effect.

Alternatively, defining `FUSIBLE_FLOAT_ROUND_LEFT_PRODUCT` enables this left-rounding behavior by default.

For expressions like `a*b*c + d`, `a*b` is evaluated first, becomes `float &&`, then fuses with `c`, effectively behaving like `fmaf(a*b, c, d)`.

## About [test.cpp](test.cpp)

This is not a comprehensive test suite, but it demonstrates behavior verification. Performing `std::fma(a, b, -a*b)` extracts rounding errors from multiplication, showing:

```bash
test1
0.000000e+00
-3.576279e-09
0.000000e+00
-3.576279e-09
0.000000e+00
-3.576279e-09
test2
3.576279e-09
-3.576279e-09
3.576279e-09
3.576279e-09
```

Different results may appear depending on the compiler and FPU.

## Tips

The unary `+` operator has useful applications:

1. When applied to `FusibleProduct`, it invokes `operator Float()`. This is useful when explicitly rounding multiplication results or passing them to `printf()`.
2. When applied to lvalue `float/double`, it converts them to rvalues. This ensures FMA when writing expressions like [quadratic polynomials](test.cpp#L17).

## Discussion

A more rigorous library design would only apply FMA when all three operands (`a*b + c`) are `FusibleFloat` and would keep the return type as `FusibleFloat`, minimizing implicit conversions. However, this would make handling immediate values (`1.5f`) inconvenient. Allowing rvalue fusion was a compromise to maintain usability. It might have been better to fuse lvalues as well to simplify the rules.

Strictly speaking, true fusion applies to multiplication and addition/subtraction, not the operands themselves. The name "FusibleFloat" was chosen for lack of a better alternative.

## TODO

- Add a license
- ~~Provide an English version of this document~~ (generated by ChatGPT 4o)
