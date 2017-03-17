
Put your answers to questions 1 and 3 in this file
along with the table produced for question 4.

All answers are to be in plain text ASCII.

Question 1. 
(a)
| 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
----------------------------------------------------------------------------
| CT | CT | CT | CT | CT | CT | CT | CT | CT | CI | CI | CI | CO | CO | CO |

(b) 
 Operation    Address    Cache Set     Tag    Hit?     Value/Unknown
-------------------------------------------------------------------------
  Read        0x02F8     n/a           00B    No       Unknown   
  Read        0x35DD     3             0D7    Yes      28    
  Read        0x7A2F     5             1E8    No       Unknown
  Read        0x2055     n/a           081    No       Unknown  

Question 3. 

(a) An array with 64 rows and 64 columns, a direct-mapped cache, and the function sumA from the program cache-test provided with question 2.

Answer:

sumA access the array row-first or row major order. The first load will load
16bytes of memory into a line cache set. After the first access, the next three 
will be cache hits. The first access was 4 bytes and the next 3 bytes 
sums to a total of 16 bytes. Since 16 bytes > size of one cache line, the next
access (number 5) will be a miss. This pattern will then be repeated. 
In summary, it's 1 miss for 3 hits i.e. 25% miss rate; For 64 * 64 = 4096 
accesses, 1024 will be a miss.

(b) An array with 64 rows and 64 columns, a direct-mapped cache, and the function sumB from the program cache-test provided with question 2.

Answer: 

sumB accesses the columns then the rows i.e. in column major order. The first
array access will likewise load 16 bytes of memory into a cache set. In this case, 
the miss rate is actually 100%. Consider trying to load cache set 0 as the set to be loaded
into. On the next memory access it would load 16*4 = 256 bytes ahead in memory which
would cause 16bytes of memory to be loaded into cache set 16. This would repeat for 8 accesses
and the next access (9th access) would overwrite the value written to by the first access.

(c) An array with 64 rows and 64 columns, a direct-mapped cache, and the function sumC from the program cache-test provided with question 2.

Answer: 

sumC alternates its reads -- starts by reading the value in the current index then
the adjacent row, adjacent column, adjacent row, adjacent column... etc. The first four
accesses, the first access to each row is always a miss and the next access to the 
new element in the row is a hit. For columns however, like in question 3b will always be
a miss. So for rows the miss rate is 50%; in a 64 row-64column matrix, this results in a 25% miss rate since half 

(d) An array with 68 rows and 68 columns, a direct-mapped cache, and the function sumB from the program cache-test provided with question 2.

Answer:

The difference between this problem and the problem in question 3b is that now we are
considering a non-square matrix with 4 extra rows. This is a problem since after the end of the first column is reached, all the values in the first column are stored in different sets which 
is caused by 68%16 != 0. Therefore, when the second, third and fourth columns are iterated over,
the values have already been loaded into the cache. This pattern is repeated and the miss
rate is 25%





