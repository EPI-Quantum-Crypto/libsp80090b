2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/drift.bin'
Loaded 1000000 samples of 256 distinct 8-bit-wide symbols
Number of Binary samples: 8000000
Raw Mean = 67.532336000000001
Median = 15
Binary = false
Literal Most Common Value Estimate: Mode count = 33407
Literal Most Common Value Estimate: p-hat = 0.033406999999999999
Literal Most Common Value Estimate: p_u = 0.033869868746992923
Literal Most Common Value Estimate: min entropy = 4.8838537936845894
Bitstring Most Common Value Estimate: Mode count = 4999992
Bitstring Most Common Value Estimate: p-hat = 0.62499899999999997
Bitstring Most Common Value Estimate: p_u = 0.6254398877045183
Bitstring Most Common Value Estimate: min entropy = 0.67705686417820199
H_bitstring = 0.67705686417820199
H_bitstring Per Symbol = 5.4164549134256159
H_original = 4.8838537936845894
Assessed min entropy: 4.8838537936845894
Chi square independence: T = 429838.90048326936
Chi square independence: df = 26879
Chi square independence: P-value = 0
Chi square goodness of fit: T = 884667.95899606554
Chi square goodness of fit: df = 2295
Chi square goodness of fit: P-value = 0
Chi square tests: Failed
Literal Longest Repeated Substring results: P_col = 0.01855104114800000018985
Literal Longest Repeated Substring results: W = 9
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.0001300915143892948156769
Length of longest repeated substring test: Failed
Permutation testing: Unpermuted result excursion = 30019675.5323360003531
Permutation testing: Unpermuted result numDirectionalRuns = 664871
Permutation testing: Unpermuted result lenDirectionalRuns = 9
Permutation testing: Unpermuted result numIncreasesDecreases = 516493
Permutation testing: Unpermuted result numRunsMedian = 113248
Permutation testing: Unpermuted result lenRunsMedian = 174
Permutation testing: Unpermuted result avgCollision = 8.965428512511543601704
Permutation testing: Unpermuted result maxCollision = 74
Permutation testing: Unpermuted result periodicity(1) = 32984
Permutation testing: Unpermuted result periodicity(2) = 32735
Permutation testing: Unpermuted result periodicity(8) = 33495
Permutation testing: Unpermuted result periodicity(16) = 33150
Permutation testing: Unpermuted result periodicity(32) = 33560
Permutation testing: Unpermuted result covariance(1) = 8172496634
Permutation testing: Unpermuted result covariance(2) = 8163351145
Permutation testing: Unpermuted result covariance(8) = 8165866392
Permutation testing: Unpermuted result covariance(16) = 8165902062
Permutation testing: Unpermuted result covariance(32) = 8160383871
Permutation testing: Unpermuted result compression = 810924


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
               excursion*       0       0   10000
       numDirectionalRuns    9970       0       6
       lenDirectionalRuns       0       6       9
   numIncreasesDecreases*       0       0   10000
           numRunsMedian*   10000       0       0
           lenRunsMedian*       0       0   10000
            avgCollision*   10000       0       0
            maxCollision*       0       0   10000
          periodicity(1)*       0       0   10000
          periodicity(2)*       0       0   10000
          periodicity(8)*       0       0   10000
         periodicity(16)*       0       0   10000
         periodicity(32)*       0       0   10000
           covariance(1)*       0       0   10000
           covariance(2)*       0       0   10000
           covariance(8)*       0       0   10000
          covariance(16)*       0       0   10000
          covariance(32)*       0       0   10000
             compression*   10000       0       0
(* denotes failed test)

IID permutation tests: Failed
Chi-square tests (5.2.1-5.2.4): FAILED (smallest p-value 0.000000)
LRS test (5.2.5): FAILED
Permutation tests (5.1): FAILED
H_original: 4.883854 bits per symbol
H_bitstring: 0.677057 bits per bit
IID assumption rejected: use the non-IID track (test_non_iid)
