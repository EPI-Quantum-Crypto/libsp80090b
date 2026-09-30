2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/urandom-1bit.bin'
Loaded 1000000 samples of 2 distinct 1-bit-wide symbols
Raw Mean = 0.49934699999999999
Median = 0.5
Binary = true
Literal Most Common Value Estimate: Mode count = 500653
Literal Most Common Value Estimate: p-hat = 0.50065300000000001
Literal Most Common Value Estimate: p_u = 0.50194091419737441
Literal Most Common Value Estimate: min entropy = 0.99441054706868792
H_original = 0.99441054706868792
Assessed min entropy: 0.99441054706868792
Chi square independence: T = 2026.24425462559
Chi square independence: df = 2046
Chi square independence: P-value = 0.61768393992970316
Chi square goodness of fit: T = 20.102198287033076
Chi square goodness of fit: df = 9
Chi square goodness of fit: P-value = 0.017292070588803405
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.5000008528180000000633
Literal Longest Repeated Substring results: W = 40
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.3653886092347440802406
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 368.7776320000044165681
Permutation testing: Unpermuted result numDirectionalRuns = 79722
Permutation testing: Unpermuted result lenDirectionalRuns = 11
Permutation testing: Unpermuted result numIncreasesDecreases = 74678
Permutation testing: Unpermuted result numRunsMedian = 500240
Permutation testing: Unpermuted result lenRunsMedian = 20
Permutation testing: Unpermuted result avgCollision = 20.61005770816158388925
Permutation testing: Unpermuted result maxCollision = 64
Permutation testing: Unpermuted result periodicity(1) = 24459
Permutation testing: Unpermuted result periodicity(2) = 24462
Permutation testing: Unpermuted result periodicity(8) = 24397
Permutation testing: Unpermuted result periodicity(16) = 24598
Permutation testing: Unpermuted result periodicity(32) = 24366
Permutation testing: Unpermuted result covariance(1) = 1995199
Permutation testing: Unpermuted result covariance(2) = 1994268
Permutation testing: Unpermuted result covariance(8) = 1994849
Permutation testing: Unpermuted result covariance(16) = 1995399
Permutation testing: Unpermuted result covariance(32) = 1994757
Permutation testing: Unpermuted result compression = 155846


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion      15       0       6
       numDirectionalRuns       6       0      16
       lenDirectionalRuns       2       4       5
    numIncreasesDecreases      11       0       6
            numRunsMedian       6       0      17
            lenRunsMedian       2       4       3
             avgCollision      51       0       6
             maxCollision       4       2       4
           periodicity(1)      12       0       6
           periodicity(2)      16       0       6
           periodicity(8)      31       0       6
          periodicity(16)       6       0      12
          periodicity(32)      38       0       6
            covariance(1)       6       0      17
            covariance(2)      16       0       6
            covariance(8)       6       0       8
           covariance(16)       6       0      41
           covariance(32)       6       0      16
              compression      11       1       5
(* denotes failed test)

IID permutation tests: Passed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.017292)
LRS test (5.2.5): passed
Permutation tests (5.1): passed
H_original: 0.994411 bits per symbol
IID assumption holds: H_I = 0.994411 bits per symbol
