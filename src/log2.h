#ifndef __LOG2_H
#define __LOG2_H

// Static int logarithm
template<unsigned long long N>
static inline unsigned long long log2(){
	if (N <= 1U) return 0;
	if (N <= 2U) return 1;
	if (N <= 4U) return 2;
	if (N <= 8U) return 3;
	if (N <= 16U) return 4;
	if (N <= 32U) return 5;
	if (N <= 64U) return 6;
	if (N <= 128U) return 7;
	if (N <= 256U) return 8;
	if (N <= 512U) return 9;
	if (N <= 1024U) return 10;
	if (N <= 2048U) return 11;
	if (N <= 4096U) return 12;
	if (N <= 8192U) return 13;
	if (N <= 16384U) return 14;
	if (N <= 32768U) return 15;
	if (N <= 65536U) return 16;
	if (N <= 131072U) return 17;
	if (N <= 262144U) return 18;
	if (N <= 524288U) return 19;
	if (N <= 1048576U) return 20;
	if (N <= 2097152U) return 21;
	if (N <= 4194304U) return 22;
	if (N <= 8388608U) return 23;
	if (N <= 16777216U) return 24;
	if (N <= 33554432U) return 25;
	if (N <= 67108864U) return 26;
	if (N <= 134217728U) return 27;
	if (N <= 268435456U) return 28;
	if (N <= 536870912U) return 29;
	if (N <= 1073741824U) return 30;
	if (N <= 2147483648U) return 31;
	if (N <= 4294967296U) return 32;
	if (N <= 8589934592U) return 33;
	if (N <= 17179869184U) return 34;
	if (N <= 34359738368U) return 35;
	if (N <= 68719476736U) return 36;
	if (N <= 137438953472U) return 37;
	if (N <= 274877906944U) return 38;
	if (N <= 549755813888U) return 39;
	if (N <= 1099511627776U) return 40;
	if (N <= 2199023255552U) return 41;
	if (N <= 4398046511104U) return 42;
	if (N <= 8796093022208U) return 43;
	if (N <= 17592186044416U) return 44;
	if (N <= 35184372088832U) return 45;
	if (N <= 70368744177664U) return 46;
	if (N <= 140737488355328U) return 47;
	if (N <= 281474976710656U) return 48;
	if (N <= 562949953421312U) return 49;
	if (N <= 1125899906842624U) return 50;
	if (N <= 2251799813685248U) return 51;
	if (N <= 4503599627370496U) return 52;
	if (N <= 9007199254740992U) return 53;
	if (N <= 18014398509481984U) return 54;
	if (N <= 36028797018963968U) return 55;
	if (N <= 72057594037927936U) return 56;
	if (N <= 144115188075855872U) return 57;
	if (N <= 288230376151711744U) return 58;
	if (N <= 576460752303423488U) return 59;
	if (N <= 1152921504606846976U) return 60;
	if (N <= 2305843009213693952U) return 61;
	if (N <= 4611686018427387904U) return 62;
	if (N <= 9223372036854775808U) return 63;
}

// Dynamic int logarithm
static inline int log2(int n){
	return 8 * sizeof(n) - __builtin_clz(n) - 1;
}

#endif
