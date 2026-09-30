2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/ringosc-fast.bin'
Loaded 1000000 samples of 2 distinct 1-bit-wide symbols
Raw Mean = 0.49996200000000002
Median = 0.5
Binary = true
Literal Most Common Value Estimate: Mode count = 500038
Literal Most Common Value Estimate: p-hat = 0.50003799999999998
Literal Most Common Value Estimate: p_u = 0.50132591529201276
Literal Most Common Value Estimate: min entropy = 0.99617928088101204
H_original = 0.99617928088101204
Assessed min entropy: 0.99617928088101204
Chi square independence: T = 7453713.0804467043
Chi square independence: df = 2046
Chi square independence: P-value = 0
Chi square goodness of fit: T = 0.97494400563127659
Chi square goodness of fit: df = 9
Chi square goodness of fit: P-value = 0.99949310680460446
Chi square tests: Failed
Literal Longest Repeated Substring results: P_col = 0.5000000028880000000069
Literal Longest Repeated Substring results: W = 177
Literal Longest Repeated Substring results: Pr(X >= 1) = 0
Length of longest repeated substring test: Failed
Permutation testing: Unpermuted result excursion = 225.8671260000081133512
Permutation testing: Unpermuted result numDirectionalRuns = 90003
Permutation testing: Unpermuted result lenDirectionalRuns = 4
Permutation testing: Unpermuted result numIncreasesDecreases = 68227
Permutation testing: Unpermuted result numRunsMedian = 159966
Permutation testing: Unpermuted result lenRunsMedian = 10
Permutation testing: Unpermuted result avgCollision = 6.950567170818505502439
Permutation testing: Unpermuted result maxCollision = 18
Permutation testing: Unpermuted result periodicity(1) = 11485
Permutation testing: Unpermuted result periodicity(2) = 11314
Permutation testing: Unpermuted result periodicity(8) = 28098
Permutation testing: Unpermuted result periodicity(16) = 20409
Permutation testing: Unpermuted result periodicity(32) = 18092
Permutation testing: Unpermuted result covariance(1) = 1783350
Permutation testing: Unpermuted result covariance(2) = 1961648
Permutation testing: Unpermuted result covariance(8) = 2157211
Permutation testing: Unpermuted result covariance(16) = 2017377
Permutation testing: Unpermuted result covariance(32) = 1957727
Permutation testing: Unpermuted result compression = 44235


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion     269       0       6
      numDirectionalRuns*       0       0   10000
      lenDirectionalRuns*   10000       0       0
   numIncreasesDecreases*   10000       0       0
           numRunsMedian*   10000       0       0
           lenRunsMedian*   10000       0       0
            avgCollision*   10000       0       0
            maxCollision*   10000       0       0
          periodicity(1)*   10000       0       0
          periodicity(2)*   10000       0       0
          periodicity(8)*       0       0   10000
         periodicity(16)*   10000       0       0
         periodicity(32)*   10000       0       0
           covariance(1)*   10000       0       0
           covariance(2)*   10000       0       0
           covariance(8)*       0       0   10000
          covariance(16)*       0       0   10000
          covariance(32)*   10000       0       0
             compression*   10000       0       0
(* denotes failed test)

IID permutation tests: Failed
Chi-square tests (5.2.1-5.2.4): FAILED (smallest p-value 0.000000)
LRS test (5.2.5): FAILED
Permutation tests (5.1): FAILED
H_original: 0.996179 bits per symbol
IID assumption rejected: use the non-IID track (test_non_iid)
