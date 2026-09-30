2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/aging.bin'
Loaded 1000000 samples of 160 distinct 8-bit-wide symbols
Number of Binary samples: 8000000

Samples have been translated
Raw Mean = 127.516019
Median = 80
Binary = false
Literal Most Common Value Estimate: Mode count = 51076
Literal Most Common Value Estimate: p-hat = 0.051076000000000003
Literal Most Common Value Estimate: p_u = 0.051643076240260828
Literal Most Common Value Estimate: min entropy = 4.2752812490757082
Bitstring Most Common Value Estimate: Mode count = 4001763
Bitstring Most Common Value Estimate: p-hat = 0.50022037500000005
Bitstring Most Common Value Estimate: p_u = 0.50067572157616091
Bitstring Most Common Value Estimate: min entropy = 0.99805159594841197
H_bitstring = 0.99805159594841197
H_bitstring Per Symbol = 7.9844127675872958
H_original = 4.2752812490757082
Assessed min entropy: 4.2752812490757082
Chi square independence: T = 44745.230660363915
Chi square independence: df = 6234
Chi square independence: P-value = 0
Chi square goodness of fit: T = 340138.30807158793
Chi square goodness of fit: df = 1062
Chi square goodness of fit: P-value = 0
Chi square tests: Failed
Literal Longest Repeated Substring results: P_col = 0.02803965565199999931826
Literal Longest Repeated Substring results: W = 10
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.000150195145124309753417
Length of longest repeated substring test: Failed
Permutation testing: Unpermuted result excursion = 11423.34774199999628763
Permutation testing: Unpermuted result numDirectionalRuns = 665250
Permutation testing: Unpermuted result lenDirectionalRuns = 9
Permutation testing: Unpermuted result numIncreasesDecreases = 518205
Permutation testing: Unpermuted result numRunsMedian = 500251
Permutation testing: Unpermuted result lenRunsMedian = 22
Permutation testing: Unpermuted result avgCollision = 7.80103441819827114756
Permutation testing: Unpermuted result maxCollision = 36
Permutation testing: Unpermuted result periodicity(1) = 35969
Permutation testing: Unpermuted result periodicity(2) = 36041
Permutation testing: Unpermuted result periodicity(8) = 36007
Permutation testing: Unpermuted result periodicity(16) = 36155
Permutation testing: Unpermuted result periodicity(32) = 35740
Permutation testing: Unpermuted result covariance(1) = 16260388989
Permutation testing: Unpermuted result covariance(2) = 16260099945
Permutation testing: Unpermuted result covariance(8) = 16260352259
Permutation testing: Unpermuted result covariance(16) = 16259941787
Permutation testing: Unpermuted result covariance(32) = 16259910513
Permutation testing: Unpermuted result compression = 720482


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion       6       0      19
       numDirectionalRuns      97       0       6
       lenDirectionalRuns       1       5       8
   numIncreasesDecreases*       0       0   10000
            numRunsMedian       6       0      10
            lenRunsMedian       4       2      76
            avgCollision*   10000       0       0
             maxCollision       1       5    3524
          periodicity(1)*       0       0   10000
          periodicity(2)*       0       0   10000
          periodicity(8)*       0       0   10000
         periodicity(16)*       0       0   10000
         periodicity(32)*       0       0   10000
            covariance(1)       6       0       9
            covariance(2)      66       0       6
            covariance(8)       6       0      16
           covariance(16)      17       0       6
           covariance(32)       6       0      21
             compression*   10000       0       0
(* denotes failed test)

IID permutation tests: Failed
Chi-square tests (5.2.1-5.2.4): FAILED (smallest p-value 0.000000)
LRS test (5.2.5): FAILED
Permutation tests (5.1): FAILED
H_original: 4.275281 bits per symbol
H_bitstring: 0.998052 bits per bit
IID assumption rejected: use the non-IID track (test_non_iid)
