2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/ringosc-slow.bin'
Loaded 1000000 samples of 2 distinct 1-bit-wide symbols
Raw Mean = 0.49992900000000001
Median = 0.5
Binary = true
Literal Most Common Value Estimate: Mode count = 500071
Literal Most Common Value Estimate: p-hat = 0.50007100000000004
Literal Most Common Value Estimate: p_u = 0.50135891528274756
Literal Most Common Value Estimate: min entropy = 0.99608431799427444
H_original = 0.99608431799427444
Assessed min entropy: 0.99608431799427444
Chi square independence: T = 2055.0608067620824
Chi square independence: df = 2046
Chi square independence: P-value = 0.43965108768332739
Chi square goodness of fit: T = 12.086116243704449
Chi square goodness of fit: df = 9
Chi square goodness of fit: P-value = 0.20849791705007797
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.5000000100820000555036
Literal Longest Repeated Substring results: W = 39
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.5972444139286070828631
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 357.7644540000110850997
Permutation testing: Unpermuted result numDirectionalRuns = 79459
Permutation testing: Unpermuted result lenDirectionalRuns = 11
Permutation testing: Unpermuted result numIncreasesDecreases = 74781
Permutation testing: Unpermuted result numRunsMedian = 497902
Permutation testing: Unpermuted result lenRunsMedian = 20
Permutation testing: Unpermuted result avgCollision = 20.68995199470286294741
Permutation testing: Unpermuted result maxCollision = 66
Permutation testing: Unpermuted result periodicity(1) = 24620
Permutation testing: Unpermuted result periodicity(2) = 24419
Permutation testing: Unpermuted result periodicity(8) = 24521
Permutation testing: Unpermuted result periodicity(16) = 24492
Permutation testing: Unpermuted result periodicity(32) = 24707
Permutation testing: Unpermuted result covariance(1) = 1999576
Permutation testing: Unpermuted result covariance(2) = 1999042
Permutation testing: Unpermuted result covariance(8) = 1999410
Permutation testing: Unpermuted result covariance(16) = 1998099
Permutation testing: Unpermuted result covariance(32) = 1999379
Permutation testing: Unpermuted result compression = 155888


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion      24       0       6
       numDirectionalRuns      87       0       6
       lenDirectionalRuns       4       2       4
    numIncreasesDecreases       6       0      16
           numRunsMedian*    9998       0       2
            lenRunsMedian       4       2       4
             avgCollision       7       0       6
             maxCollision       5       3       3
           periodicity(1)       6       0      19
           periodicity(2)      30       1       5
           periodicity(8)       9       0       6
          periodicity(16)       6       0       6
          periodicity(32)       6       0      73
            covariance(1)       6       0       6
            covariance(2)      16       0       6
            covariance(8)      12       0       6
           covariance(16)     120       0       6
           covariance(32)       6       0      48
              compression       6       0      23
(* denotes failed test)

IID permutation tests: Failed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.208498)
LRS test (5.2.5): passed
Permutation tests (5.1): FAILED
H_original: 0.996084 bits per symbol
IID assumption rejected: use the non-IID track (test_non_iid)
