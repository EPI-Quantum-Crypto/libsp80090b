2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/interference.bin'
Loaded 1000000 samples of 116 distinct 8-bit-wide symbols
Number of Binary samples: 8000000

Samples have been translated
Raw Mean = 127.50077899999999
Median = 58
Binary = false
Literal Most Common Value Estimate: Mode count = 18543
Literal Most Common Value Estimate: p-hat = 0.018543
Literal Most Common Value Estimate: p_u = 0.018890490498679125
Literal Most Common Value Estimate: min entropy = 5.7261960274436356
Bitstring Most Common Value Estimate: Mode count = 4000690
Bitstring Most Common Value Estimate: p-hat = 0.50008624999999995
Bitstring Most Common Value Estimate: p_u = 0.50054159661361408
Bitstring Most Common Value Estimate: min entropy = 0.99843812825457134
H_bitstring = 0.99843812825457134
H_bitstring Per Symbol = 7.9875050260365708
H_original = 5.7261960274436356
Assessed min entropy: 5.7261960274436356
Chi square independence: T = 1733987.2817082759
Chi square independence: df = 9607
Chi square independence: P-value = 0
Chi square goodness of fit: T = 959.63152565299004
Chi square goodness of fit: df = 945
Chi square goodness of fit: P-value = 0.3631125059334675
Chi square tests: Failed
Literal Longest Repeated Substring results: P_col = 0.01190965264599999994486
Literal Longest Repeated Substring results: W = 9
Literal Longest Repeated Substring results: Pr(X >= 1) = 2.410204839813025103762e-06
Length of longest repeated substring test: Failed
Permutation testing: Unpermuted result excursion = 2540.877830001804341009
Permutation testing: Unpermuted result numDirectionalRuns = 236864
Permutation testing: Unpermuted result lenDirectionalRuns = 13
Permutation testing: Unpermuted result numIncreasesDecreases = 514685
Permutation testing: Unpermuted result numRunsMedian = 100203
Permutation testing: Unpermuted result lenRunsMedian = 12
Permutation testing: Unpermuted result avgCollision = 13.10365206452373776358
Permutation testing: Unpermuted result maxCollision = 43
Permutation testing: Unpermuted result periodicity(1) = 29536
Permutation testing: Unpermuted result periodicity(2) = 13119
Permutation testing: Unpermuted result periodicity(8) = 7069
Permutation testing: Unpermuted result periodicity(16) = 7548
Permutation testing: Unpermuted result periodicity(32) = 7076
Permutation testing: Unpermuted result covariance(1) = 17017225145
Permutation testing: Unpermuted result covariance(2) = 16903623342
Permutation testing: Unpermuted result covariance(8) = 15609140325
Permutation testing: Unpermuted result covariance(16) = 16503352668
Permutation testing: Unpermuted result covariance(32) = 15608777705
Permutation testing: Unpermuted result compression = 624901


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
               excursion*   10000       0       0
      numDirectionalRuns*   10000       0       0
      lenDirectionalRuns*       0       1    9999
   numIncreasesDecreases*       0       0   10000
           numRunsMedian*   10000       0       0
           lenRunsMedian*   10000       0       0
            avgCollision*       0       0   10000
             maxCollision       3       3       3
          periodicity(1)*       0       0   10000
          periodicity(2)*       0       0   10000
          periodicity(8)*   10000       0       0
         periodicity(16)*   10000       0       0
         periodicity(32)*   10000       0       0
           covariance(1)*       0       0   10000
           covariance(2)*       0       0   10000
           covariance(8)*   10000       0       0
          covariance(16)*       0       0   10000
          covariance(32)*   10000       0       0
             compression*   10000       0       0
(* denotes failed test)

IID permutation tests: Failed
Chi-square tests (5.2.1-5.2.4): FAILED (smallest p-value 0.000000)
LRS test (5.2.5): FAILED
Permutation tests (5.1): FAILED
H_original: 5.726196 bits per symbol
H_bitstring: 0.998438 bits per bit
IID assumption rejected: use the non-IID track (test_non_iid)
