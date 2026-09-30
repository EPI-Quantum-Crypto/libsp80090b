2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/thermal-lsb.bin'
Loaded 1000000 samples of 2 distinct 1-bit-wide symbols
Raw Mean = 0.50134000000000001
Median = 0.5
Binary = true
Literal Most Common Value Estimate: Mode count = 501340
Literal Most Common Value Estimate: p-hat = 0.50134000000000001
Literal Most Common Value Estimate: p_u = 0.50262791067056256
Literal Most Common Value Estimate: min entropy = 0.99243730932737662
H_original = 0.99243730932737662
Assessed min entropy: 0.99243730932737662
Chi square independence: T = 1889.2351167733746
Chi square independence: df = 2046
Chi square independence: P-value = 0.99388959626108642
Chi square goodness of fit: T = 4.6802736155972173
Chi square goodness of fit: df = 9
Chi square goodness of fit: P-value = 0.86124186003915026
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.5000035912000000000682
Literal Longest Repeated Substring results: W = 38
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.8378688120976144697001
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 282.6842199999978220148
Permutation testing: Unpermuted result numDirectionalRuns = 79613
Permutation testing: Unpermuted result lenDirectionalRuns = 11
Permutation testing: Unpermuted result numIncreasesDecreases = 74841
Permutation testing: Unpermuted result numRunsMedian = 500315
Permutation testing: Unpermuted result lenRunsMedian = 19
Permutation testing: Unpermuted result avgCollision = 21.02961467272421458574
Permutation testing: Unpermuted result maxCollision = 71
Permutation testing: Unpermuted result periodicity(1) = 24691
Permutation testing: Unpermuted result periodicity(2) = 24547
Permutation testing: Unpermuted result periodicity(8) = 24477
Permutation testing: Unpermuted result periodicity(16) = 24406
Permutation testing: Unpermuted result periodicity(32) = 24862
Permutation testing: Unpermuted result covariance(1) = 2010421
Permutation testing: Unpermuted result covariance(2) = 2010955
Permutation testing: Unpermuted result covariance(8) = 2008673
Permutation testing: Unpermuted result covariance(16) = 2009974
Permutation testing: Unpermuted result covariance(32) = 2010777
Permutation testing: Unpermuted result compression = 155882


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion      23       0       6
       numDirectionalRuns       8       0       6
       lenDirectionalRuns       3       3       4
    numIncreasesDecreases       6       0      23
            numRunsMedian       6       0       6
            lenRunsMedian       9       4       2
             avgCollision       6       0     881
             maxCollision       3       3      31
           periodicity(1)       6       0      27
           periodicity(2)       6       0       6
           periodicity(8)      18       0       6
          periodicity(16)      19       0       6
          periodicity(32)       6       0     253
            covariance(1)      14       0       6
            covariance(2)       6       0      11
            covariance(8)    3744       0       6
           covariance(16)      24       0       6
           covariance(32)       6       0       9
              compression       6       0      10
(* denotes failed test)

IID permutation tests: Passed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.861242)
LRS test (5.2.5): passed
Permutation tests (5.1): passed
H_original: 0.992437 bits per symbol
IID assumption holds: H_I = 0.992437 bits per symbol
