2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/warmup.bin'
Loaded 1000000 samples of 183 distinct 8-bit-wide symbols
Number of Binary samples: 8000000

Samples have been translated
Raw Mean = 127.497005
Median = 93
Binary = false
Literal Most Common Value Estimate: Mode count = 116298
Literal Most Common Value Estimate: p-hat = 0.116298
Literal Most Common Value Estimate: p_u = 0.11712376474780223
Literal Most Common Value Estimate: min entropy = 3.0938942623849059
Bitstring Most Common Value Estimate: Mode count = 4001311
Bitstring Most Common Value Estimate: p-hat = 0.50016387500000004
Bitstring Most Common Value Estimate: p_u = 0.50061922159593208
Bitstring Most Common Value Estimate: min entropy = 0.9982144095967358
H_bitstring = 0.9982144095967358
H_bitstring Per Symbol = 7.9857152767738864
H_original = 3.0938942623849059
Assessed min entropy: 3.0938942623849059
Chi square independence: T = 353186.67146633397
Chi square independence: df = 10029
Chi square independence: P-value = 0
Chi square goodness of fit: T = 827802.93462839501
Chi square goodness of fit: df = 1287
Chi square goodness of fit: P-value = 0
Chi square tests: Failed
Literal Longest Repeated Substring results: P_col = 0.0353870795100000006881
Literal Longest Repeated Substring results: W = 38
Literal Longest Repeated Substring results: Pr(X >= 1) = 0
Length of longest repeated substring test: Failed
Permutation testing: Unpermuted result excursion = 14550.59354499894470791
Permutation testing: Unpermuted result numDirectionalRuns = 632811
Permutation testing: Unpermuted result lenDirectionalRuns = 23
Permutation testing: Unpermuted result numIncreasesDecreases = 555572
Permutation testing: Unpermuted result numRunsMedian = 499896
Permutation testing: Unpermuted result lenRunsMedian = 18
Permutation testing: Unpermuted result avgCollision = 6.63063039168230439202
Permutation testing: Unpermuted result maxCollision = 38
Permutation testing: Unpermuted result periodicity(1) = 111307
Permutation testing: Unpermuted result periodicity(2) = 111547
Permutation testing: Unpermuted result periodicity(8) = 111049
Permutation testing: Unpermuted result periodicity(16) = 110976
Permutation testing: Unpermuted result periodicity(32) = 111015
Permutation testing: Unpermuted result covariance(1) = 16255503915
Permutation testing: Unpermuted result covariance(2) = 16255111381
Permutation testing: Unpermuted result covariance(8) = 16255187225
Permutation testing: Unpermuted result covariance(16) = 16254817830
Permutation testing: Unpermuted result covariance(32) = 16255238692
Permutation testing: Unpermuted result compression = 712786


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion       6       0       9
      numDirectionalRuns*   10000       0       0
      lenDirectionalRuns*       0       0   10000
   numIncreasesDecreases*       0       0   10000
            numRunsMedian       6       0       7
            lenRunsMedian      53       6       0
            avgCollision*   10000       0       0
             maxCollision       3       3     329
          periodicity(1)*       0       0   10000
          periodicity(2)*       0       0   10000
          periodicity(8)*       0       0   10000
         periodicity(16)*       0       0   10000
         periodicity(32)*       0       0   10000
            covariance(1)       6       0       6
            covariance(2)      30       0       6
            covariance(8)      12       0       6
           covariance(16)     130       0       6
           covariance(32)       6       0      17
             compression*   10000       0       0
(* denotes failed test)

IID permutation tests: Failed
Chi-square tests (5.2.1-5.2.4): FAILED (smallest p-value 0.000000)
LRS test (5.2.5): FAILED
Permutation tests (5.1): FAILED
H_original: 3.093894 bits per symbol
H_bitstring: 0.998214 bits per bit
IID assumption rejected: use the non-IID track (test_non_iid)
