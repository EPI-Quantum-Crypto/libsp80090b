2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/lowpass.bin'
Loaded 1000000 samples of 188 distinct 8-bit-wide symbols
Number of Binary samples: 8000000

Samples have been translated
Raw Mean = 127.289964
Median = 94
Binary = false
Literal Most Common Value Estimate: Mode count = 19885
Literal Most Common Value Estimate: p-hat = 0.019885
Literal Most Common Value Estimate: p_u = 0.020244599113403074
Literal Most Common Value Estimate: min entropy = 5.6263191149417153
Bitstring Most Common Value Estimate: Mode count = 4002376
Bitstring Most Common Value Estimate: p-hat = 0.50029699999999999
Bitstring Most Common Value Estimate: p_u = 0.5007523465400574
Bitstring Most Common Value Estimate: min entropy = 0.99783081832262521
H_bitstring = 0.99783081832262521
H_bitstring Per Symbol = 7.9826465465810017
H_original = 5.6263191149417153
Assessed min entropy: 5.6263191149417153
Chi square independence: T = 1547845.7287921435
Chi square independence: df = 11121
Chi square independence: P-value = 0
Chi square goodness of fit: T = 2167.3021902417941
Chi square goodness of fit: df = 1341
Chi square goodness of fit: P-value = 5.7410822847685431e-42
Chi square tests: Failed
Literal Longest Repeated Substring results: P_col = 0.01404697039599999997463
Literal Longest Repeated Substring results: W = 8
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.0007576353966650372349402
Length of longest repeated substring test: Failed
Permutation testing: Unpermuted result excursion = 55897.8440679984196322
Permutation testing: Unpermuted result numDirectionalRuns = 506312
Permutation testing: Unpermuted result lenDirectionalRuns = 18
Permutation testing: Unpermuted result numIncreasesDecreases = 531158
Permutation testing: Unpermuted result numRunsMedian = 100810
Permutation testing: Unpermuted result lenRunsMedian = 244
Permutation testing: Unpermuted result avgCollision = 7.015630919474105553491
Permutation testing: Unpermuted result maxCollision = 36
Permutation testing: Unpermuted result periodicity(1) = 62916
Permutation testing: Unpermuted result periodicity(2) = 45495
Permutation testing: Unpermuted result periodicity(8) = 24316
Permutation testing: Unpermuted result periodicity(16) = 18928
Permutation testing: Unpermuted result periodicity(32) = 15836
Permutation testing: Unpermuted result covariance(1) = 16585993721
Permutation testing: Unpermuted result covariance(2) = 16566921740
Permutation testing: Unpermuted result covariance(8) = 16470411324
Permutation testing: Unpermuted result covariance(16) = 16379931850
Permutation testing: Unpermuted result covariance(32) = 16279847316
Permutation testing: Unpermuted result compression = 648408


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
               excursion*       0       0   10000
      numDirectionalRuns*   10000       0       0
      lenDirectionalRuns*       0       0   10000
   numIncreasesDecreases*       0       0   10000
           numRunsMedian*   10000       0       0
           lenRunsMedian*       0       0   10000
            avgCollision*   10000       0       0
             maxCollision     379       6       0
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
H_original: 5.626319 bits per symbol
H_bitstring: 0.997831 bits per bit
IID assumption rejected: use the non-IID track (test_non_iid)
