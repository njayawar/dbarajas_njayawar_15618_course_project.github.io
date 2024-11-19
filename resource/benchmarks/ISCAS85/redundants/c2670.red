3870->3876 /1
3875 /0
3870 /1
3864->3869 /1
3864 /1
3859 /0
3858 /0
3857 /0
3843->3852 /1
3840->3852 /1
3852 /0
3513 /1
3716 /1
3494 /1
3691 /1
3432 /1
3503 /1
3434 /1
3505 /1
3414 /1
3484 /1
3416 /1
3486 /1
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
<203
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls
0som_96*        2s_c2670.test   4s_c2670.log    6_c2670.fault
c2670.out
1s_c2670.log    3_c2670         4s_c2670.out    6_c2670.func
c2670.red.ip
1s_c2670.out    3_c2670.fault   4s_c2670.test   6s_c2670.fault
c2670.ssl
1s_c2670.test   3_c2670.func    5_c2670         awk.fault.3_10_96
c2670.test
2_c2670         3_c2670.log     5_c2670.fault   awk.func.3_10_96
clean_fault_script*
2_c2670.fault   3_c2670.out     5_c2670.func    awk.func.3_6_96
com_96*
2_c2670.func    3_c2670.test    5_c2670.log     awk.test.clean*
faults
2_c2670.log     4_c2670         5_c2670.out     c2670              out
2_c2670.out     4_c2670.fault   5_c2670.test    c2670.fault
som_96*
2_c2670.test    4_c2670.func    5s_c2670.fault  c2670.fsim.out
ssl_redundant
2s_c2670        4_c2670.log     5s_c2670.log    c2670.fsim.ssl
test_script*
2s_c2670.fault  4_c2670.out     5s_c2670.out    c2670.func
2s_c2670.log    4_c2670.test    5s_c2670.test   c2670.ip
2s_c2670.out    4s_c2670.fault  6_c2670         c2670.log
<204
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v 6s_c2670.fault
3870->3876 /1
3875 /0
3870 /1
3864->3869 /1
3864 /1
3859 /0
3858 /0
3857 /0
3852 /0
3840->3852 /1
3843->3852 /1
3432 /1
3503 /1
3434 /1
3505 /1
3414 /1
3484 /1
3416 /1
3486 /1
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
<205
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls
0som_96*        2s_c2670.test   4s_c2670.log    6_c2670.fault
c2670.out
1s_c2670.log    3_c2670         4s_c2670.out    6_c2670.func
c2670.red.ip
1s_c2670.out    3_c2670.fault   4s_c2670.test   6s_c2670.fault
c2670.ssl
1s_c2670.test   3_c2670.func    5_c2670         awk.fault.3_10_96
c2670.test
2_c2670         3_c2670.log     5_c2670.fault   awk.func.3_10_96
clean_fault_script*
2_c2670.fault   3_c2670.out     5_c2670.func    awk.func.3_6_96
com_96*
2_c2670.func    3_c2670.test    5_c2670.log     awk.test.clean*
faults
2_c2670.log     4_c2670         5_c2670.out     c2670              out
2_c2670.out     4_c2670.fault   5_c2670.test    c2670.fault
som_96*
2_c2670.test    4_c2670.func    5s_c2670.fault  c2670.fsim.out
ssl_redundant
2s_c2670        4_c2670.log     5s_c2670.log    c2670.fsim.ssl
test_script*
2s_c2670.fault  4_c2670.out     5s_c2670.out    c2670.func
2s_c2670.log    4_c2670.test    5s_c2670.test   c2670.ip
2s_c2670.out    4s_c2670.fault  6_c2670         c2670.log
<206
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v 6_c2670.func

INPUT(1)
INPUT(2)
INPUT(3)
INPUT(4)
INPUT(5)
INPUT(6)
INPUT(7)
INPUT(8)
INPUT(11)
INPUT(14)
INPUT(15)
INPUT(16)
INPUT(19)
INPUT(20)
INPUT(21)
INPUT(22)
INPUT(23)
INPUT(24)
INPUT(25)
INPUT(26)
INPUT(27)
INPUT(28)
INPUT(29)
INPUT(32)
INPUT(33)
INPUT(34)
INPUT(35)
INPUT(36)
INPUT(37)
INPUT(40)
INPUT(43)
INPUT(44)
INPUT(47)
INPUT(48)
INPUT(49)
INPUT(50)
INPUT(51)
INPUT(52)
INPUT(53)
INPUT(54)
INPUT(55)
INPUT(56)
INPUT(57)
INPUT(60)
INPUT(61)
"6_c2670.func" 1629L,
30716C                                                    2,1
Top
and23_3857 = OR(3843,3852)
and24_3857 = NOT(3843)
and25_3857 = OR(3852, and24_3857)
and26_3857 = NOT(3852)
and27_3857 = OR(3843, and26_3857)
3857 = AND(and23_3857, and25_3857, and27_3857)
and23_3858 = OR(3852,3840)
and24_3858 = NOT(3852)
and25_3858 = OR(3840, and24_3858)
and26_3858 = NOT(3840)
and27_3858 = OR(3852, and26_3858)
3858 = AND(and23_3858, and25_3858, and27_3858)
or23_3859 = AND(3857,3858)
or24_3859 = NOT(3857)
or25_3859 = AND(3858, or24_3859)
or26_3859 = NOT(3858)
or27_3859 = AND(3857, or26_3859)
3859 = OR(or23_3859, or25_3859, or27_3859)
3864 = NOT(3859)
and23_3869 = OR(3859,3864)
and24_3869 = NOT(3859)
and25_3869 = OR(3864, and24_3869)
and26_3869 = NOT(3864)
and27_3869 = OR(3859, and26_3869)
3869 = AND(and23_3869, and25_3869, and27_3869)
or23_3870 = AND(3869,3864)
or24_3870 = NOT(3869)
or25_3870 = AND(3864, or24_3870)
or26_3870 = NOT(3864)
or27_3870 = AND(3869, or26_3870)
3870 = OR(or23_3870, or25_3870, or27_3870)
3875 = NOT(3870)
and32_3876 = NOT(2826)
and33_3876 = NOT(3028)
and34_3876 = NOT(3870)
and36_3876 = OR(and32_3876, and33_3876, 3870)
and37_3876 = OR(and32_3876, 3028, and34_3876)
and38_3876 = OR(and32_3876, 3028, 3870)
and39_3876 = OR(2826, and33_3876, and34_3876)
and3A_3876 = OR(2826, and33_3876, 3870)
and3B_3876 = OR(2826, 3028, and34_3876)
and35_3876 = OR(2826, 3028, 3870)
3876 =
AND(and35_3876,and36_3876,and37_3876,and38_3876,and39_3876,and3A_3876,and3B_3876)

3877 = AND(3826,3876,1591)
3881 = BUFF(3877)
3882 = NOT(3877)
:1

INPUT(1)
INPUT(2)
INPUT(3)
INPUT(4)
INPUT(5)
INPUT(6)
INPUT(7)
INPUT(8)
INPUT(11)
INPUT(14)
INPUT(15)
INPUT(16)
INPUT(19)
INPUT(20)
INPUT(21)
INPUT(22)
INPUT(23)
INPUT(24)
INPUT(25)
INPUT(26)
INPUT(27)
INPUT(28)
INPUT(29)
INPUT(32)
INPUT(33)
INPUT(34)
INPUT(35)
INPUT(36)
INPUT(37)
INPUT(40)
INPUT(43)
INPUT(44)
INPUT(47)
INPUT(48)
INPUT(49)
INPUT(50)
INPUT(51)
INPUT(52)
INPUT(53)
INPUT(54)
INPUT(55)
INPUT(56)
INPUT(57)
INPUT(60)
INPUT(61)
<207
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls
0som_96*        2s_c2670.test   4s_c2670.log    6_c2670.fault
c2670.out
1s_c2670.log    3_c2670         4s_c2670.out    6_c2670.func
c2670.red.ip
1s_c2670.out    3_c2670.fault   4s_c2670.test   6s_c2670.fault
c2670.ssl
1s_c2670.test   3_c2670.func    5_c2670         awk.fault.3_10_96
c2670.test
2_c2670         3_c2670.log     5_c2670.fault   awk.func.3_10_96
clean_fault_script*
2_c2670.fault   3_c2670.out     5_c2670.func    awk.func.3_6_96
com_96*
2_c2670.func    3_c2670.test    5_c2670.log     awk.test.clean*
faults
2_c2670.log     4_c2670         5_c2670.out     c2670              out
2_c2670.out     4_c2670.fault   5_c2670.test    c2670.fault
som_96*
2_c2670.test    4_c2670.func    5s_c2670.fault  c2670.fsim.out
ssl_redundant
2s_c2670        4_c2670.log     5s_c2670.log    c2670.fsim.ssl
test_script*
2s_c2670.fault  4_c2670.out     5s_c2670.out    c2670.func
2s_c2670.log    4_c2670.test    5s_c2670.test   c2670.ip
2s_c2670.out    4s_c2670.fault  6_c2670         c2670.log
<208
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v 6_c2670.f
6_c2670.fault  6_c2670.func
<208
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v 6_c2670.fault
and3A_3876 /1
and38_3876 /1
and36_3876 /1
and35_3876 /1
3875 /0
3870 /1
and25_3869 /1
3864 /1
or27_3859 /0
or25_3859 /0
3858 /0
3857 /0
nand27_3852 /1
nand25_3852 /1
nand25_3504 /1
nand25_3564 /1
nand25_3485 /1
nand25_3553 /1
3503 /1
3484 /1
3432 /1
3414 /1
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
<209
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls
0som_96*        2s_c2670.test   4s_c2670.log    6_c2670.fault
c2670.out
1s_c2670.log    3_c2670         4s_c2670.out    6_c2670.func
c2670.red.ip
1s_c2670.out    3_c2670.fault   4s_c2670.test   6s_c2670.fault
c2670.ssl
1s_c2670.test   3_c2670.func    5_c2670         awk.fault.3_10_96
c2670.test
2_c2670         3_c2670.log     5_c2670.fault   awk.func.3_10_96
clean_fault_script*
2_c2670.fault   3_c2670.out     5_c2670.func    awk.func.3_6_96
com_96*
2_c2670.func    3_c2670.test    5_c2670.log     awk.test.clean*
faults
2_c2670.log     4_c2670         5_c2670.out     c2670              out
2_c2670.out     4_c2670.fault   5_c2670.test    c2670.fault
som_96*
2_c2670.test    4_c2670.func    5s_c2670.fault  c2670.fsim.out
ssl_redundant
2s_c2670        4_c2670.log     5s_c2670.log    c2670.fsim.ssl
test_script*
2s_c2670.fault  4_c2670.out     5s_c2670.out    c2670.func
2s_c2670.log    4_c2670.test    5s_c2670.test   c2670.ip
2s_c2670.out    4s_c2670.fault  6_c2670         c2670.log
<210
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v ssl_redundant
* List of identified redundant faults:
3859->3869 /1
3869 /0
3436 /1
3507 /1
3836 /1
3418 /1
3488 /1
3834 /1
2403 /1
3634 /1
2656 /1
3661 /1
2376 /1
3599 /1
2645 /1
3629 /1
2655 /1
3631 /1
2630 /1
3596 /1
3438 /1
3509 /1
3420 /1
3490 /1
3430 /1
3501 /1
3412 /1
3482 /1
246->543 /1
537->1069 /1
2391->2632 /1
2646->2810 /1
2391->2631 /1
2646->2809 /1
2382->2612 /1
2638->2790 /1
2382->2611 /1
2638->2789 /1
2291->2710 /1
2710 /0
2294->2709 /1
2709 /0
2869->3071 /1
2869->3070 /1
2869->3069 /1
"ssl_redundant" 107L,
1222C                                                     1,1
Top
1963->2357 /1
1966->2356 /1
2356 /1
241->1814 /1
1656->2156 /1
730->2155 /1
2155 /0
2395 /1
2377 /1
2119 /1
241->1815 /1
1963 /1
1656->1888 /1
1656 /1
582->1128 /1
594->1128 /1
1102->1495 /1
594->1495 /1
582->1494 /1
1113->1494 /1
1102->1493 /1
1113->1493 /1
550->1099 /1
1086->1472 /1
37->499 /1
37->499 /1

2 * List of identified redundant faults:
3440 /1
3511 /1
3422 /1
3492 /1

4 * List of identified redundant faults:
3515 /1
3718 /1
3496 /1
3693 /1

5 * List of identified redundant faults:
3513 /1
3716 /1
3494 /1
3691 /1

6 * Not sure about these faults below being redundant...
3870->3876 /1
3875 /0
3870 /1
3864->3869 /1
3864 /1
3859 /0
3858 /0
3857 /0
3852 /0
3840->3852 /1
3843->3852 /1
3432 /1
3503 /1
3434 /1
3505 /1
3414 /1
3484 /1
3416 /1
3486 /1
"ssl_redundant" 126L, 1454C written
<211
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls
0som_96*        2s_c2670.test   4s_c2670.log    6_c2670.fault
c2670.out
1s_c2670.log    3_c2670         4s_c2670.out    6_c2670.func
c2670.red.ip
1s_c2670.out    3_c2670.fault   4s_c2670.test   6s_c2670.fault
c2670.ssl
1s_c2670.test   3_c2670.func    5_c2670         awk.fault.3_10_96
c2670.test
2_c2670         3_c2670.log     5_c2670.fault   awk.func.3_10_96
clean_fault_script*
2_c2670.fault   3_c2670.out     5_c2670.func    awk.func.3_6_96
com_96*
2_c2670.func    3_c2670.test    5_c2670.log     awk.test.clean*
faults
2_c2670.log     4_c2670         5_c2670.out     c2670              out
2_c2670.out     4_c2670.fault   5_c2670.test    c2670.fault
som_96*
2_c2670.test    4_c2670.func    5s_c2670.fault  c2670.fsim.out
ssl_redundant
2s_c2670        4_c2670.log     5s_c2670.log    c2670.fsim.ssl
test_script*
2s_c2670.fault  4_c2670.out     5s_c2670.out    c2670.func
2s_c2670.log    4_c2670.test    5s_c2670.test   c2670.ip
2s_c2670.out    4s_c2670.fault  6_c2670         c2670.log
<212
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
cd ..
Directory:
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory
<213 /afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory>
ls
NR_benchmarks_89/  awk_det_fault   c17.85    c3540/  c7552/
i89_i85       scripts/
Nemesis/           benchmarks_85/  c17.89*   c432/   c880/
iccad95_out/  soprano/
README.com_96      benchmarks_89/  c17.test  c499/   ckts_faults/  junk
README_scripts     c1355/          c1908/    c5315/  cla/
junk.test
atalanta*          c17/            c2670/    c6288/  fsim*
plots/
<214 /afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory>
cd c2670/
Directory:
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670
<215
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls
0som_96*        2s_c2670.test   4s_c2670.log    6_c2670.fault
c2670.out
1s_c2670.log    3_c2670         4s_c2670.out    6_c2670.func
c2670.red.ip
1s_c2670.out    3_c2670.fault   4s_c2670.test   6s_c2670.fault
c2670.ssl
1s_c2670.test   3_c2670.func    5_c2670         awk.fault.3_10_96
c2670.test
2_c2670         3_c2670.log     5_c2670.fault   awk.func.3_10_96
clean_fault_script*
2_c2670.fault   3_c2670.out     5_c2670.func    awk.func.3_6_96
com_96*
2_c2670.func    3_c2670.test    5_c2670.log     awk.test.clean*
faults
2_c2670.log     4_c2670         5_c2670.out     c2670              out
2_c2670.out     4_c2670.fault   5_c2670.test    c2670.fault
som_96*
2_c2670.test    4_c2670.func    5s_c2670.fault  c2670.fsim.out
ssl_redundant
2s_c2670        4_c2670.log     5s_c2670.log    c2670.fsim.ssl
test_script*
2s_c2670.fault  4_c2670.out     5s_c2670.out    c2670.func
2s_c2670.log    4_c2670.test    5s_c2670.test   c2670.ip
2s_c2670.out    4s_c2670.fault  6_c2670         c2670.log
<216
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls ?s*
0som_96*       2s_c2670        2s_c2670.test   4s_c2670.test
5s_c2670.test
1s_c2670.log   2s_c2670.fault  4s_c2670.fault  5s_c2670.fault
6s_c2670.fault
1s_c2670.out   2s_c2670.log    4s_c2670.log    5s_c2670.log
ssl_redundant
1s_c2670.test  2s_c2670.out    4s_c2670.out    5s_c2670.out
<217
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v 6s_c2670.fault
3870->3876 /1
3875 /0
3870 /1
3864->3869 /1
3864 /1
3859 /0
3858 /0
3857 /0
3852 /0
3840->3852 /1
3843->3852 /1
3432 /1
3503 /1
3434 /1
3505 /1
3414 /1
3484 /1
3416 /1
3486 /1
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
<218
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls
0som_96*        2s_c2670.test   4s_c2670.log    6_c2670.fault
c2670.out
1s_c2670.log    3_c2670         4s_c2670.out    6_c2670.func
c2670.red.ip
1s_c2670.out    3_c2670.fault   4s_c2670.test   6s_c2670.fault
c2670.ssl
1s_c2670.test   3_c2670.func    5_c2670         awk.fault.3_10_96
c2670.test
2_c2670         3_c2670.log     5_c2670.fault   awk.func.3_10_96
clean_fault_script*
2_c2670.fault   3_c2670.out     5_c2670.func    awk.func.3_6_96
com_96*
2_c2670.func    3_c2670.test    5_c2670.log     awk.test.clean*
faults
2_c2670.log     4_c2670         5_c2670.out     c2670              out
2_c2670.out     4_c2670.fault   5_c2670.test    c2670.fault
som_96*
2_c2670.test    4_c2670.func    5s_c2670.fault  c2670.fsim.out
ssl_redundant
2s_c2670        4_c2670.log     5s_c2670.log    c2670.fsim.ssl
test_script*
2s_c2670.fault  4_c2670.out     5s_c2670.out    c2670.func
2s_c2670.log    4_c2670.test    5s_c2670.test   c2670.ip
2s_c2670.out    4s_c2670.fault  6_c2670         c2670.log
<219
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
5s*.fault
5s_c2670.fault: Permission denied.
<220
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v 5s*.fault
3870->3876 /1
*:qLog file for the circuit c2670.
* Number of faults detected by each test pattern:


End of test pattern generation.

*********************************************************
*                                                       *
*          Welcome to atalanta (version 1.1)            *
*                                                       *
*                 Copyright (C) 1991,                   *
*   Virginia Polytechnic Institute & State University   *
*                                                       *
*********************************************************

******   SUMMARY OF TEST PATTERN GENERATION RESULTS   ******
1. Circuit structure
   Name of the circuit                       : c2670
   Number of gates                           : 1426
   Number of primary inputs                  : 233
   Number of primary outputs                 : 140
   Depth of the circuit                      : 33

2. ATPG parameters
   Test pattern generation mode              : DTPG + TC
   Backtrack limit                           : 10000
   Initial random number generator seed      : 826840462
   Test pattern compaction mode              : REVERSE + SHUFFLE

3. Test pattern generation results
   Number of test patterns before compaction : 0
   Number of test patterns after compaction  : 0
   Fault coverage                            : 0.000 %
   Number of collapsed faults                : 27
   Number of identified redundant faults     : 4
   Number of aborted faults                  : 23
   Total number of backtrackings             : 251607

4. Memory used                               : 271560 Kbytes

5. CPU time
   Initialization                            : 0.317 secs
   Fault simulation                          : 0.000 secs
   FAN                                       : 1810.017 secs
   Total                                     : 1810.333 secs
<221
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls
0som_96*        2s_c2670.test   4s_c2670.log    6_c2670.fault
c2670.out
1s_c2670.log    3_c2670         4s_c2670.out    6_c2670.func
c2670.red.ip
1s_c2670.out    3_c2670.fault   4s_c2670.test   6s_c2670.fault
c2670.ssl
1s_c2670.test   3_c2670.func    5_c2670         awk.fault.3_10_96
c2670.test
2_c2670         3_c2670.log     5_c2670.fault   awk.func.3_10_96
clean_fault_script*
2_c2670.fault   3_c2670.out     5_c2670.func    awk.func.3_6_96
com_96*
2_c2670.func    3_c2670.test    5_c2670.log     awk.test.clean*
faults
2_c2670.log     4_c2670         5_c2670.out     c2670              out
2_c2670.out     4_c2670.fault   5_c2670.test    c2670.fault
som_96*
2_c2670.test    4_c2670.func    5s_c2670.fault  c2670.fsim.out
ssl_redundant
2s_c2670        4_c2670.log     5s_c2670.log    c2670.fsim.ssl
test_script*
2s_c2670.fault  4_c2670.out     5s_c2670.out    c2670.func
2s_c2670.log    4_c2670.test    5s_c2670.test   c2670.ip
2s_c2670.out    4s_c2670.fault  6_c2670         c2670.log
<222
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v 2s*.fault
3870->3876 /1
3875 /0
3870 /1
3864->3869 /1
3864 /1
3859 /0
3858 /0
3857 /0
3843->3852 /1
3840->3852 /1
3852 /0
3513 /1
3716 /1
3515 /1
3718 /1
3494 /1
3691 /1
3496 /1
3693 /1
3440 /1
3511 /1
3422 /1
3492 /1
3432 /1
3503 /1
3434 /1
3505 /1
3414 /1
3484 /1
3416 /1
3486 /1
~
~
~
~
~
~
~
~
~
~
~
~
~
~
~
<223
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v 1s*log
* Log file for the circuit c2670.
* Number of faults detected by each test pattern:

test    1:
011110111000011100000001001010010000000111110000010001000010101000001111010000101100011

01101111011111001101111000001101110001100011111000001101101011000011101011100111111001111000001000

110111001000011100110011010100110101100101001100
0110111000110001111100000110110101100001110101110
0111111001111000001000110111111111101111100111111101101010011111100001111111100001000001010
472 f
aults detected
test    2:
010111100010111111111101111100100001100000100101000001101011101101011010010001001001111

01110110000101101000010010000100010101101101011010011010011001100110001101100001000001010110000010

110010101101001000000000000000000101111111111011
0010001010110110101101001101001100110011000110110
0001000001010110000010110010001100011011100100000001001000111110110100011111101001110001001
274 f
aults detected
test    3:
001111111101000001100001010001000000110001001001011111100110010011001100100101000011100

00100001000011010101110100000101110100101111010100000001001001110100010111010100100100001111000100

100011111010000001111101010100101110010010001011
0010111010010111101010000000100100111010001011101
0100100100001111000100100011110011111100111111011101000110011101100110000111110001011110001
295 f
aults detected
test    4:
011111111100111111101101111110101001011010101011110111000001111000101100110011100011100

00011111111100101001111111110110110101000110110010011100011000101110010011000010011101101101111100

100101001100110001000000000010000111110000111000
1011011010100011011001001110001100010111001001100
0010011101101101111100100101110000011110001111111101011110011101101001111011100110100001001
221 f
aults detected
test    5:
101111001110111111111101111000101010100100111000010110111011111011001001011111111000000

10101001011000010100101011111110111111010000001000010011100110111000100100100110111111011001010100

010011001010110100000000000000000111101000000000
1111011111101000000100001001110011011100010010010
0110111111011001010100010011110000011100101111111101110011101100001110000111111101011111001
141 f
aults detected
test    6:
100111011100111111111101111110001011100001001101001000001010100000010010110010010111101

10011000000100101000100111001010111001001001011011010001101010001001110110001000101101010101101101

000010010110011000000001000100000001111010001100
0101011100100100101101101000110101000100111011000
1000101101010101101101000010001111111111111110011101010101011101010001100111011001101101001
64 f
aults detected
test    7:
000011001101001111111101111101001000101111010101101111011111100110101011101001101000110

10111000111101100110110100101100110110111001011001110110100111110100111011011101011000010100001011

010111111110111001000001000100000000000111011000
0110011011011100101100111011010011111010011101101
1101011000010100001011010111111111110010010111011101000100011111111011111111101010011110001
118 f
aults detected
test    8:
110111101110111111111101111000011100000100101011010111010110100110110100110101000011110

00000001100000001100110101010100001001110110111101101011100110010110001000111000100001111001000110

100100110110111000000000000100010100110101101001
1010000100111011011110110101110011001011000100011
1000100001111001000110100100001111101000000110011101011110011101101011111111010101001111001
75 f
aults detected
@
@
@
"1s_c2670.log" 285L,
51609C                                                     1,1
Top
1656 /1
582->1128 /1
594->1128 /1
1102->1495 /1
594->1495 /1
582->1494 /1
1113->1494 /1
1102->1493 /1
1113->1493 /1
550->1099 /1
1086->1472 /1
37->499 /1
37->499 /1

* List of aborted faults:
3870->3876 /1
3875 /0
3870 /1
3864->3869 /1
3864 /1
3859 /0
3858 /0
3857 /0
3843->3852 /1
3840->3852 /1
3852 /0
3513 /1
3716 /1
3515 /1
3718 /1
3494 /1
3691 /1
3496 /1
3693 /1
3440 /1
3511 /1
3422 /1
3492 /1
3432 /1
3503 /1
3434 /1
3505 /1
3414 /1
3484 /1
3416 /1
3486 /1
search hit BOTTOM, continuing at TOP
*********************************************************

******   SUMMARY OF TEST PATTERN GENERATION RESULTS   ******
1. Circuit structure
   Name of the circuit                       : c2670
   Number of gates                           : 1426
   Number of primary inputs                  : 233
   Number of primary outputs                 : 140
   Depth of the circuit                      : 33

2. ATPG parameters
   Test pattern generation mode              : DTPG + TC
   Backtrack limit                           : 10
   Initial random number generator seed      : 826839438
   Test pattern compaction mode              : REVERSE + SHUFFLE

3. Test pattern generation results
   Number of test patterns before compaction : 162
   Number of test patterns after compaction  : 119
   Fault coverage                            : 95.741 %
   Number of collapsed faults                : 2747
   Number of identified redundant faults     : 86
   Number of aborted faults                  : 31
   Total number of backtrackings             : 352

4. Memory used                               : 271654 Kbytes

5. CPU time
   Initialization                            : 0.367 secs
   Fault simulation                          : 6.317 secs
   FAN                                       : 9.883 secs
   Total                                     : 16.567 secs

* List of identified redundant faults:
3859->3869 /1
3869 /0
3436 /1
3507 /1
3836 /1
3418 /1
3488 /1
3834 /1
2403 /1
3634 /1
2656 /1
3661 /1
<224
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
c
<225
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
ls
0som_96*        2s_c2670.test   4s_c2670.log    6_c2670.fault
c2670.out
1s_c2670.log    3_c2670         4s_c2670.out    6_c2670.func
c2670.red.ip
1s_c2670.out    3_c2670.fault   4s_c2670.test   6s_c2670.fault
c2670.ssl
1s_c2670.test   3_c2670.func    5_c2670         awk.fault.3_10_96
c2670.test
2_c2670         3_c2670.log     5_c2670.fault   awk.func.3_10_96
clean_fault_script*
2_c2670.fault   3_c2670.out     5_c2670.func    awk.func.3_6_96
com_96*
2_c2670.func    3_c2670.test    5_c2670.log     awk.test.clean*
faults
2_c2670.log     4_c2670         5_c2670.out     c2670              out
2_c2670.out     4_c2670.fault   5_c2670.test    c2670.fault
som_96*
2_c2670.test    4_c2670.func    5s_c2670.fault  c2670.fsim.out
ssl_redundant
2s_c2670        4_c2670.log     5s_c2670.log    c2670.fsim.ssl
test_script*
2s_c2670.fault  4_c2670.out     5s_c2670.out    c2670.func
2s_c2670.log    4_c2670.test    5s_c2670.test   c2670.ip
2s_c2670.out    4s_c2670.fault  6_c2670         c2670.log
<226
/afs/ece/usr/blanton/PRIVATE/IP_experiments/experiment_directory/c2670>
v ssl_redundant
* List of identified redundant faults:
3859->3869 /1
3869 /0
3436 /1
3507 /1
3836 /1
3418 /1
3488 /1
3834 /1
2403 /1
3634 /1
2656 /1
3661 /1
2376 /1
3599 /1
2645 /1
3629 /1
2655 /1
3631 /1
2630 /1
3596 /1
3438 /1
3509 /1
3420 /1
3490 /1
3430 /1
3501 /1
3412 /1
3482 /1
246->543 /1
537->1069 /1
2391->2632 /1
2646->2810 /1
2391->2631 /1
2646->2809 /1
2382->2612 /1
2638->2790 /1
2382->2611 /1
2638->2789 /1
2291->2710 /1
2710 /0
2294->2709 /1
2709 /0
2869->3071 /1
2869->3070 /1
2869->3069 /1
2869->3068 /1
2854->3060 /1
2854->3059 /1
2854->3058 /1
2854->3057 /1
2395->2636 /1
2395->2635 /1
2395->2634 /1
2395->2633 /1
2377->2610 /1
2377->2609 /1
2377->2608 /1
2377->2607 /1
2869 /1
2854 /1
1963->2357 /1
1966->2356 /1
2356 /1
241->1814 /1
1656->2156 /1
730->2155 /1
2155 /0
2395 /1
2377 /1
2119 /1
241->1815 /1
1963 /1
1656->1888 /1
1656 /1
582->1128 /1
594->1128 /1
1102->1495 /1
594->1495 /1
582->1494 /1
1113->1494 /1
1102->1493 /1
1113->1493 /1
550->1099 /1
1086->1472 /1
37->499 /1
37->499 /1

2 * List of identified redundant faults:
3440 /1
3511 /1
3422 /1
3492 /1

4 * List of identified redundant faults:
3515 /1
3718 /1
3496 /1
3693 /1

5 * List of identified redundant faults:
3513 /1
3716 /1
3494 /1
3691 /1

6 * Not sure about these faults; but I am pretty positive they are
too...(you may want to validate these)
3870->3876 /1
3875 /0
3870 /1
3864->3869 /1
3864 /1
3859 /0
3858 /0
3857 /0
3852 /0
3840->3852 /1
3843->3852 /1
3432 /1
3503 /1
3434 /1
3505 /1
3414 /1
3484 /1
3416 /1
3486 /1


