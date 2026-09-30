2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/drbg-zero-seed.bin'
Loaded 1000000 samples of 256 distinct 8-bit-wide symbols
Number of Binary samples: 8000000
Raw Mean = 127.54993899999999
Median = 128
Binary = false
Literal Most Common Value Estimate: Mode count = 4121
Literal Most Common Value Estimate: p-hat = 0.0041209999999999997
Literal Most Common Value Estimate: p_u = 0.004286014417720147
Literal Most Common Value Estimate: min entropy = 7.8661475815588791
Bitstring Most Common Value Estimate: Mode count = 4000192
Bitstring Most Common Value Estimate: p-hat = 0.50002400000000002
Bitstring Most Common Value Estimate: p_u = 0.50047934661986426
Bitstring Most Common Value Estimate: min entropy = 0.99861756057914164
H_bitstring = 0.99861756057914164
H_bitstring Per Symbol = 7.9889404846331331
H_original = 7.8661475815588791
Assessed min entropy: 7.8661475815588791
Chi square independence: T = 65099.071996302584
Chi square independence: df = 65280
Chi square independence: P-value = 0.69123166453406093
Chi square goodness of fit: T = 2369.6289461491897
Chi square goodness of fit: df = 2295
Chi square goodness of fit: P-value = 0.13574510783090926
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.003907296095999999987214
Literal Longest Repeated Substring results: W = 4
Literal Longest Repeated Substring results: Pr(X >= 1) = 1
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 63984.10906899536348647
Permutation testing: Unpermuted result numDirectionalRuns = 666563
Permutation testing: Unpermuted result lenDirectionalRuns = 8
Permutation testing: Unpermuted result numIncreasesDecreases = 502443
Permutation testing: Unpermuted result numRunsMedian = 499008
Permutation testing: Unpermuted result lenRunsMedian = 21
Permutation testing: Unpermuted result avgCollision = 20.70164579236103818971
Permutation testing: Unpermuted result maxCollision = 81
Permutation testing: Unpermuted result periodicity(1) = 3812
Permutation testing: Unpermuted result periodicity(2) = 3957
Permutation testing: Unpermuted result periodicity(8) = 3832
Permutation testing: Unpermuted result periodicity(16) = 3981
Permutation testing: Unpermuted result periodicity(32) = 4057
Permutation testing: Unpermuted result covariance(1) = 16275912659
Permutation testing: Unpermuted result covariance(2) = 16265327423
Permutation testing: Unpermuted result covariance(8) = 16270779892
Permutation testing: Unpermuted result covariance(16) = 16265325562
Permutation testing: Unpermuted result covariance(32) = 16265503332
Permutation testing: Unpermuted result compression = 1067539


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion       6       0       6
       numDirectionalRuns       9       0       6
       lenDirectionalRuns       7       6       0
    numIncreasesDecreases       6       0     217
            numRunsMedian      84       0       6
            lenRunsMedian       2       4       6
             avgCollision      21       0       6
             maxCollision       4       2     142
           periodicity(1)      73       1       5
           periodicity(2)       6       0      27
           periodicity(8)      30       0       6
          periodicity(16)       6       0      16
          periodicity(32)       6       0    1009
            covariance(1)       6       0      40
            covariance(2)      44       0       6
            covariance(8)       6       0      17
           covariance(16)      20       0       6
           covariance(32)      14       0       6
              compression      21       0       6
(* denotes failed test)

IID permutation tests: Passed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.135745)
LRS test (5.2.5): passed
Permutation tests (5.1): passed
H_original: 7.866148 bits per symbol
H_bitstring: 0.998618 bits per bit
IID assumption holds: H_I = 7.866148 bits per symbol
