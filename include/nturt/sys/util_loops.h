#ifndef NTURT_SYS_UTIL_LOOP_H_
#define NTURT_SYS_UTIL_LOOP_H_

#define N_FOR_LOOP_GET_ARG(                                                    \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, \
    _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, \
    _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, \
    _62, _63, _64, N, ...)                                                     \
  N

#define N_FOR_LOOP_0(z_call, sep, fixed_arg0, fixed_arg1, ...)

#define N_FOR_LOOP_1(z_call, sep, fixed_arg0, fixed_arg1, x) \
  z_call(0, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_2(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_1(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(1, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_3(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_2(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(2, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_4(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_3(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(3, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_5(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_4(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(4, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_6(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_5(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(5, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_7(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_6(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(6, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_8(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_7(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(7, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_9(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_8(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(8, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_10(z_call, sep, fixed_arg0, fixed_arg1, x, ...) \
  N_FOR_LOOP_9(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(9, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_11(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_10(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(10, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_12(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_11(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(11, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_13(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_12(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(12, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_14(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_13(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(13, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_15(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_14(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(14, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_16(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_15(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(15, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_17(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_16(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(16, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_18(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_17(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(17, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_19(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_18(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(18, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_20(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_19(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(19, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_21(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_20(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(20, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_22(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_21(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(21, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_23(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_22(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(22, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_24(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_23(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(23, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_25(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_24(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(24, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_26(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_25(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(25, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_27(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_26(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(26, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_28(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_27(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(27, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_29(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_28(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(28, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_30(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_29(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(29, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_31(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_30(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(30, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_32(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_31(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(31, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_33(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_32(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(32, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_34(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_33(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(33, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_35(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_34(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(34, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_36(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_35(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(35, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_37(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_36(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(36, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_38(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_37(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(37, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_39(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_38(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(38, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_40(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_39(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(39, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_41(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_40(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(40, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_42(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_41(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(41, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_43(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_42(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(42, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_44(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_43(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(43, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_45(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_44(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(44, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_46(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_45(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(45, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_47(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_46(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(46, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_48(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_47(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(47, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_49(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_48(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(48, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_50(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_49(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(49, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_51(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_50(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(50, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_52(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_51(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(51, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_53(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_52(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(52, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_54(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_53(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(53, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_55(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_54(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(54, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_56(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_55(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(55, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_57(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_56(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(56, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_58(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_57(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(57, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_59(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_58(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(58, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_60(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_59(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(59, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_61(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_60(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(60, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_62(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_61(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(61, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_63(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_62(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(62, x, fixed_arg0, fixed_arg1)

#define N_FOR_LOOP_64(z_call, sep, fixed_arg0, fixed_arg1, x, ...)  \
  N_FOR_LOOP_63(z_call, sep, fixed_arg0, fixed_arg1, ##__VA_ARGS__) \
  __DEBRACKET sep z_call(63, x, fixed_arg0, fixed_arg1)

#define N_FOR_EACH_ENGINE(x, sep, fixed_arg0, fixed_arg1, ...)                 \
  N_FOR_LOOP_GET_ARG(                                                          \
      __VA_ARGS__, N_FOR_LOOP_64, N_FOR_LOOP_63, N_FOR_LOOP_62, N_FOR_LOOP_61, \
      N_FOR_LOOP_60, N_FOR_LOOP_59, N_FOR_LOOP_58, N_FOR_LOOP_57,              \
      N_FOR_LOOP_56, N_FOR_LOOP_55, N_FOR_LOOP_54, N_FOR_LOOP_53,              \
      N_FOR_LOOP_52, N_FOR_LOOP_51, N_FOR_LOOP_50, N_FOR_LOOP_49,              \
      N_FOR_LOOP_48, N_FOR_LOOP_47, N_FOR_LOOP_46, N_FOR_LOOP_45,              \
      N_FOR_LOOP_44, N_FOR_LOOP_43, N_FOR_LOOP_42, N_FOR_LOOP_41,              \
      N_FOR_LOOP_40, N_FOR_LOOP_39, N_FOR_LOOP_38, N_FOR_LOOP_37,              \
      N_FOR_LOOP_36, N_FOR_LOOP_35, N_FOR_LOOP_34, N_FOR_LOOP_33,              \
      N_FOR_LOOP_32, N_FOR_LOOP_31, N_FOR_LOOP_30, N_FOR_LOOP_29,              \
      N_FOR_LOOP_28, N_FOR_LOOP_27, N_FOR_LOOP_26, N_FOR_LOOP_25,              \
      N_FOR_LOOP_24, N_FOR_LOOP_23, N_FOR_LOOP_22, N_FOR_LOOP_21,              \
      N_FOR_LOOP_20, N_FOR_LOOP_19, N_FOR_LOOP_18, N_FOR_LOOP_17,              \
      N_FOR_LOOP_16, N_FOR_LOOP_15, N_FOR_LOOP_14, N_FOR_LOOP_13,              \
      N_FOR_LOOP_12, N_FOR_LOOP_11, N_FOR_LOOP_10, N_FOR_LOOP_9, N_FOR_LOOP_8, \
      N_FOR_LOOP_7, N_FOR_LOOP_6, N_FOR_LOOP_5, N_FOR_LOOP_4, N_FOR_LOOP_3,    \
      N_FOR_LOOP_2, N_FOR_LOOP_1,                                              \
      N_FOR_LOOP_0)(x, sep, fixed_arg0, fixed_arg1, __VA_ARGS__)

#endif  // NTURT_SYS_UTIL_LOOP_H_
