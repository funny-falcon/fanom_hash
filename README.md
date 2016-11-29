# fanom_hash
FAst NOn-Multiplicative hash :-)

fanom_hash.h contains fast non-multiplicative function for 64bit processor.
It passes whole SMHasher.

It is not "fastest function in a world":
- SpookyHash is also non-multiplicative, and it is faster for long strings.
  But it uses 12 64bit variable and has huge code.
- there is bunch of multiplicative hashes that are faster,
  (to be fair, SpookyHash is as fast on long strings).
- functions on top of hardware AES and CRC are fastest.
  (AES ones: Golang x86_64 assembler function, and falkhash).

But performance of fanom64 is comparable, and it is better than Murmur3_x64
So if you need crossplatform and relatively simple function,
which works fast despite of absence of fast multiplication or
"hardware permutation" instructions, then this function is suitable for you :-).

fanom_hash32.h contains 32bit  variant that produce 32bit output.
Its performance 2 times better than murmur3_32, it is better than Murmur3_x32_128,
and just slightly slower than Murmur3_x64.
So, fanom32 is a good function, if you need fast crossplatform hash function
which not relies and 64bit arithmetic.

## argue
I argued against @ticki ( https://github.com/ticki ) it has no seed independent collision.
If it has I will give @ticki 50$.
If it hasn't, @ticki will write a blog post about this function.

For the argue, test_fanom.c is written.
It reads a seed from /dev/urantom, then it reads stdin line by line and
inserts it into simple chained hash table.

If there is a file with strings that leads to notable performace degradation compared to
random strings, then I loose.

Copyright 2016 Sokolov Yura aka funny-falcon.
