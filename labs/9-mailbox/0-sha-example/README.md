## Possible overclock: run sha round many times.

We'd like to know when overclocking goes wrong.  One simple hack to
detect bit-flips: just compute a sha-256 over and over:

  1. It is so compute intensive that `gcc` uses the maximal number
     of registers --- so there is a decent chance any corruption will
     hit one.
  2. It avalanches all the results into a single output so well that
     if you find two inputs that have the same sha you can enjoy
     a viral moment.  Thus, if you corrupt any register, it is highly
     likely to produce a different sha result.

As a result, it's very hard to make even a single bit error in any
register and not have it show up.

You can adapt the included code (or use your own) to do this.
