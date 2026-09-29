/* fixed.h - 嵌入式定点运算库, Q16.16
 *
 * 值域: [-32768, 32768), 精度: 1/65536 ≈ 0.000015
 *
 * 用法:
 *   fix_t x = FIX(1.5);              // 字面量转定点
 *   fix_t y = fix_mul(x, FIX(2));    // 定点乘法
 *   FIX_PRINT(y);                    // 调试打印 -> y = 3.00000
 *
 * 最后"调试打印"一节依赖 <stdio.h>, 上目标板时整节删掉即可, 其余部分无库依赖。
 */
#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

/* ---- 基本类型与常量 ---- */

#define FIX_BITS   16
#define FIX_ONE    ((fix_t)1 << FIX_BITS)      /* 65536 */
#define FIX_HALF   (FIX_ONE >> 1)              /* 32768, 四舍五入用 */

typedef int32_t fix_t;

/* 浮点字面量转定点, 四舍五入 (编译期完成; v 只写常量, 会被求值两次) */
#define FIX(v)     ((fix_t)((v) * (double)FIX_ONE + ((v) >= 0 ? 0.5 : -0.5)))
/* 整数转定点, n 需在 [-32768, 32767] 内 */
#define FIX_INT(n) ((fix_t)((int64_t)(n) << FIX_BITS))

/* ---- 四则运算 (普通版回绕, 安全关键用 *_sat 版) ---- */

static inline fix_t fix_add(fix_t a, fix_t b) { return a + b; }
static inline fix_t fix_sub(fix_t a, fix_t b) { return a - b; }

/* 乘法: 64 位中间量, 结果四舍五入 */
static inline fix_t fix_mul(fix_t a, fix_t b) {
    int64_t t = (int64_t)a * b;
    /* 右移是向负无穷取整, 所以无论正负都统一 +0.5 再右移, 即四舍五入 */
    t += FIX_HALF;
    return (fix_t)(t >> FIX_BITS);
}

/* 除法: 左移后除, 除零饱和, 结果四舍五入, 溢出饱和 */
static inline fix_t fix_div(fix_t a, fix_t b) {
    if (b == 0) return (a >= 0) ? INT32_MAX : INT32_MIN;   /* 除零饱和 */
    int64_t t = (int64_t)a << FIX_BITS;
    /* C 的 / 是向零截断; 按结果符号加/减半量再截断, 即四舍五入 */
    if ((t >= 0) == (b >= 0)) t += b / 2;
    else                      t -= b / 2;
    int64_t q = t / b;
    if (q > INT32_MAX) return INT32_MAX;   /* 结果超出 Q16.16 值域时饱和 */
    if (q < INT32_MIN) return INT32_MIN;
    return (fix_t)q;
}

/* ---- 饱和运算 ---- */

static inline fix_t fix_add_sat(fix_t a, fix_t b) {
    int64_t t = (int64_t)a + b;
    if (t > INT32_MAX) return INT32_MAX;
    if (t < INT32_MIN) return INT32_MIN;
    return (fix_t)t;
}

static inline fix_t fix_sub_sat(fix_t a, fix_t b) {
    int64_t t = (int64_t)a - b;
    if (t > INT32_MAX) return INT32_MAX;
    if (t < INT32_MIN) return INT32_MIN;
    return (fix_t)t;
}

static inline fix_t fix_mul_sat(fix_t a, fix_t b) {
    int64_t t = ((int64_t)a * b + FIX_HALF) >> FIX_BITS;
    if (t > INT32_MAX) return INT32_MAX;
    if (t < INT32_MIN) return INT32_MIN;
    return (fix_t)t;
}

/* ---- 取整 / 小数 ---- */

/* 向下取整 (向负无穷), 负数也成立: fix_to_int(FIX(-0.75)) == -1 */
static inline int32_t fix_to_int(fix_t a) { return a >> FIX_BITS; }
/* 小数部分, 恒在 [0, FIX_ONE) 内:
 * a == fix_to_int(a) * FIX_ONE + fix_frac(a) 对任意 a 成立 */
static inline fix_t fix_frac(fix_t a) { return a & (FIX_ONE - 1); }

/* 四舍五入到整数 (先扩到 64 位, 防 a + FIX_HALF 溢出) */
static inline int32_t fix_round(fix_t a) {
    return (int32_t)(((int64_t)a + FIX_HALF) >> FIX_BITS);
}

/* 绝对值 (INT32_MIN 无对应正数, 会回绕成自身) */
static inline fix_t fix_abs(fix_t a) {
    int64_t t = a;
    return (fix_t)(t < 0 ? -t : t);
}

/* 转 double, 仅宿主机调试用 */
static inline double fix_to_double(fix_t a) {
    return (double)a / (double)FIX_ONE;
}

#endif
