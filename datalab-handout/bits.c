/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  /* 用 非 与 实现异或 */
  return ~(~(~x & y) & ~(~y & x));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  /* 0X1 shift left 31 bits */
  return 1<<31;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  /* 把最大值变为0再逻辑取反， 同时排除-1的干扰*/
  return !(~((x+1)^x)) & !!(x+1);
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  /* 构造 0xAAAAAAAA, 与x相与后，若满足条件，则预与后结果仍为0xAAAAAAAA；再利用^与！进行判断 */
  int a = 0xAA;
  a = (a<<8) + 0xAA;
  a = (a<<16) + a;
  return !(a ^ (a & x));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  /* 补码取负 = 反码+1 */
  return ~x+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  /* 通过异或，计算出不同的位，用掩码去除可变位的不同，求0，最后用逻辑取反判断 */
  int a = 0x30; // 0011 0XXX, 0x30--0x37
  int b = 0x38; // 0011 100X, 0x38--0x39
  int maska = ~0x07; //取反后为a的掩码
  int maskb = ~0x01; //取反后为b的掩码
  int ret = !((x^a) & (maska)) | !((x^b) & (maskb));

  return ret;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /* 1 取反后+1为 全1；0 取反后+1为 全0；因为不能使用乘法，用前面的运算可以将1变为全1，将0变为全0，再相与，
  就实现了逻辑上乘以1或0的效果，然后取二者之和即可（因为无效的结果已经为0了）*/
  int tru = !!(x);
  int fal = !tru;
  tru = (~tru) + 1;
  fal = (~fal) + 1;
  tru = tru & y;
  fal = fal & z;

  return tru + fal;
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  /* 已知 (x, y) 的正负共有四种组合 (++, --, +-, -+),前两种情况需要计算差进一步判断，
  而后两种情况直接可以得出大小关系并且不再适合求差来判断大小,因为求差可能会导致int溢出。
  为此，我采用了3个标志量，a,b,c，大小关系的结果为 (b&c) + a; 
  当xy为++/--时，b = 1，a = 0; 此时，大小关系由 y - x 的值决定，c = (y - x >= 0);
  当xy为-+   时，b = 0, a = 1; 此时，c值结果因为b=0而屏蔽，大小关系由 a 决定；
  当xy为+-   时，b = 0, a = 0; 此时，c值结果因为b=0而屏蔽，大小关系由 a 决定；
  */
  int a,b,c;
  int xf = x>>31;
  int yf = y>>31;
  b = !(xf ^ yf);
  a = xf & !yf;
  c = y + (~x+1); // y - x;
  c = !(c>>31);   // 判断 c 是否非负
  return (b&c)+a;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  /* 注：右移操作符 a>>b， 若a为负数，则左侧补1，若a为非负数则左侧补0；
    x != 0: (x>>31) & ((-x)>>31) == 0 | -1 or -1 | -1 ==> -1;
    x == 0: (x>>31) & ((-x)>>31) == 0 | 0  ==> 0;
  */
  int y = ~x+1; // y = -x;
  int b;
  b = (x>>31) | (y>>31) ; // if x == 0, b = 0, else b = -1; 
  b = b + 1;              
  return b;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {

  int sign = x>>31;
  x = (~sign&x) | (sign&~x);
  /* 二分搜索 最高位的1 */
  int ans = 0;
  ans = (!!(x>>16))<<4;
  ans += (!!(x>>(8+ans)))<<3;
  ans += (!!(x>>(4+ans)))<<2;
  ans += (!!(x>>(2+ans)))<<1;
  ans += (!!(x>>(1+ans)))<<0;
  ans += (!!(x>>(0+ans)));
  return ans+1; // +1 是符号位
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  int exp = (uf&0x7f800000)>>23;
  int sign = uf&(1<<31);
  if(exp == 0) return uf<<1|sign;
  if(exp == 255) return uf;
  exp++;
  if(exp == 255) return 0x7f800000|sign;
  return (exp<<23)|(uf&0x807fffff);
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  //https://cdmana.com/2021/06/20210605051544113V.html
int sign = (uf >> 31) & 1;
  int exp = (uf >> 23) & 0xff;
  int frac = uf & 0x7fffff;

  int E = exp - 127;

  if (E < 0) // decimal 
  {
    return 0;
  }
  else if (E >= 31) //  beyond int Range 
  {
    return 0x80000000u;
  }
  else
  {
    frac = frac | (1 << 23);  // Add the implied 1

    if (E < 23)     // Leave out some of the decimal 
    {
      frac >>= (23 - E);
    }
    else        // There's no need to omit the decimal 
    {
      frac <<= (E - 23);
    }

    if (sign)
      return -frac;
    else
      return frac;
  }
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  //https://cdmana.com/2021/06/20210605051544113V.html
  if (x > 127) //too large, return +INF
  {
    return (0xFF << 23);
  }
  else if (x < -148) //too small, return 0
  {
    return 0;
  }
  else if (x >= -126) //norm, Calculation exp
  {
    int exp = x + 127;
    return (exp << 23);
  }
  else //denorm, Make frac One of them is 1
  {
    int t = 148 + x;
    return (1 << t);
  }

}
