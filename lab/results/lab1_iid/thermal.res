2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/thermal.bin'
Loaded 1000000 samples of 184 distinct 8-bit-wide symbols
Number of Binary samples: 8000000

Samples have been translated
Raw Mean = 127.48656699999999
Median = 88
Binary = false
Literal Most Common Value Estimate: Mode count = 20002
Literal Most Common Value Estimate: p-hat = 0.020001999999999999
Literal Most Common Value Estimate: p_u = 0.020362633945174038
Literal Most Common Value Estimate: min entropy = 5.6179320009611811
Bitstring Most Common Value Estimate: Mode count = 4001174
Bitstring Most Common Value Estimate: p-hat = 0.50014674999999997
Bitstring Most Common Value Estimate: p_u = 0.50060209660077648
Bitstring Most Common Value Estimate: min entropy = 0.99826376161339703
H_bitstring = 0.99826376161339703
H_bitstring Per Symbol = 7.9861100929071762
H_original = 5.6179320009611811
Assessed min entropy: 5.6179320009611811
Chi square independence: T = 11182.930052131909
Chi square independence: df = 11049
Chi square independence: P-value = 0.18357698361910776
Chi square goodness of fit: T = 1409.5220645784768
Chi square goodness of fit: df = 1296
Chi square goodness of fit: P-value = 0.014598468740144237
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.01411065465799999983029
Literal Longest Repeated Substring results: W = 6
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.9806841262785169135584
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 15424.3984539946050063
Permutation testing: Unpermuted result numDirectionalRuns = 666678
Permutation testing: Unpermuted result lenDirectionalRuns = 8
Permutation testing: Unpermuted result numIncreasesDecreases = 507135
Permutation testing: Unpermuted result numRunsMedian = 499389
Permutation testing: Unpermuted result lenRunsMedian = 24
Permutation testing: Unpermuted result avgCollision = 11.31054607971768533048
Permutation testing: Unpermuted result maxCollision = 36
Permutation testing: Unpermuted result periodicity(1) = 14149
Permutation testing: Unpermuted result periodicity(2) = 14047
Permutation testing: Unpermuted result periodicity(8) = 14077
Permutation testing: Unpermuted result periodicity(16) = 14287
Permutation testing: Unpermuted result periodicity(32) = 14200
Permutation testing: Unpermuted result covariance(1) = 16252661492
Permutation testing: Unpermuted result covariance(2) = 16252213488
Permutation testing: Unpermuted result covariance(8) = 16253126681
Permutation testing: Unpermuted result covariance(16) = 16252489232
Permutation testing: Unpermuted result covariance(32) = 16252649813
Permutation testing: Unpermuted result compression = 851375


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion       8       0       6
       numDirectionalRuns       7       0       6
       lenDirectionalRuns       4       6       0
    numIncreasesDecreases       6       0      11
            numRunsMedian       6       0       7
            lenRunsMedian       2       4      23
             avgCollision      34       0       6
             maxCollision     211       6       0
           periodicity(1)       6       0      13
           periodicity(2)      11       0       6
           periodicity(8)       6       0       6
          periodicity(16)       6       0      62
          periodicity(32)       6       0      15
            covariance(1)       8       0       6
            covariance(2)      69       0       6
            covariance(8)       6       0      42
           covariance(16)       8       0       6
           covariance(32)       6       0      12
              compression      76       0       6
(* denotes failed test)

IID permutation tests: Passed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.014598)
LRS test (5.2.5): passed
Permutation tests (5.1): passed
H_original: 5.617932 bits per symbol
H_bitstring: 0.998264 bits per bit
IID assumption holds: H_I = 5.617932 bits per symbol
