2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/saturated.bin'
Loaded 1000000 samples of 256 distinct 8-bit-wide symbols
Number of Binary samples: 8000000
Raw Mean = 127.48746300000001
Median = 128
Binary = false
Literal Most Common Value Estimate: Mode count = 262885
Literal Most Common Value Estimate: p-hat = 0.26288499999999998
Literal Most Common Value Estimate: p_u = 0.26401888190425654
Literal Most Common Value Estimate: min entropy = 1.9212869840327411
Bitstring Most Common Value Estimate: Mode count = 4000919
Bitstring Most Common Value Estimate: p-hat = 0.50011487499999996
Bitstring Most Common Value Estimate: p_u = 0.50057022160837095
Bitstring Most Common Value Estimate: min entropy = 0.99835562570642278
H_bitstring = 0.99835562570642278
H_bitstring Per Symbol = 7.9868450056513822
H_original = 1.9212869840327411
Assessed min entropy: 1.9212869840327411
Chi square independence: T = 20622.920035524607
Chi square independence: df = 20566
Chi square independence: P-value = 0.3883260935991602
Chi square goodness of fit: T = 2190.982117971048
Chi square goodness of fit: df = 2295
Chi square goodness of fit: P-value = 0.93934285471718215
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.1390412798900000010947
Literal Longest Repeated Substring results: W = 13
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.9734663824709857431108
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 74114.10444600284972694
Permutation testing: Unpermuted result numDirectionalRuns = 642778
Permutation testing: Unpermuted result lenDirectionalRuns = 13
Permutation testing: Unpermuted result numIncreasesDecreases = 569363
Permutation testing: Unpermuted result numRunsMedian = 500113
Permutation testing: Unpermuted result lenRunsMedian = 19
Permutation testing: Unpermuted result avgCollision = 4.713254181847318591281
Permutation testing: Unpermuted result maxCollision = 23
Permutation testing: Unpermuted result periodicity(1) = 139122
Permutation testing: Unpermuted result periodicity(2) = 139101
Permutation testing: Unpermuted result periodicity(8) = 138899
Permutation testing: Unpermuted result periodicity(16) = 139328
Permutation testing: Unpermuted result periodicity(32) = 139232
Permutation testing: Unpermuted result covariance(1) = 16254706393
Permutation testing: Unpermuted result covariance(2) = 16275326740
Permutation testing: Unpermuted result covariance(8) = 16244706464
Permutation testing: Unpermuted result covariance(16) = 16258187748
Permutation testing: Unpermuted result covariance(32) = 16253925011
Permutation testing: Unpermuted result compression = 782811


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion      18       0       6
       numDirectionalRuns       6       0      13
       lenDirectionalRuns       2       4       4
    numIncreasesDecreases      17       0       6
            numRunsMedian       7       0       6
            lenRunsMedian      15       5       1
             avgCollision      18       0       6
             maxCollision       3       3       4
           periodicity(1)       6       0      20
           periodicity(2)       6       0       7
           periodicity(8)      10       0       6
          periodicity(16)       6       0      17
          periodicity(32)       6       0      12
            covariance(1)       6       0      16
            covariance(2)       6       0     137
            covariance(8)      21       0       6
           covariance(16)       6       0      11
           covariance(32)       6       0       6
              compression       6       0      42
(* denotes failed test)

IID permutation tests: Passed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.388326)
LRS test (5.2.5): passed
Permutation tests (5.1): passed
H_original: 1.921287 bits per symbol
H_bitstring: 0.998356 bits per bit
IID assumption holds: H_I = 1.921287 bits per symbol
