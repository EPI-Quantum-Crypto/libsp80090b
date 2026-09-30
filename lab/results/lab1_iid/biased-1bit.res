2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/biased-1bit.bin'
Loaded 1000000 samples of 2 distinct 1-bit-wide symbols
Raw Mean = 0.25031999999999999
Median = 0.5
Binary = true
Literal Most Common Value Estimate: Mode count = 749680
Literal Most Common Value Estimate: p-hat = 0.74968000000000001
Literal Most Common Value Estimate: p_u = 0.75079584284818224
Literal Most Common Value Estimate: min entropy = 0.41350743288974684
H_original = 0.41350743288974684
Assessed min entropy: 0.41350743288974684
Chi square independence: T = 133.74632172525898
Chi square independence: df = 126
Chi square independence: P-value = 0.30149080837030962
Chi square goodness of fit: T = 7.4676583432175976
Chi square goodness of fit: df = 9
Chi square goodness of fit: P-value = 0.58854935449150658
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.6246802048000000130538
Literal Longest Repeated Substring results: W = 54
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.9901261958871397929035
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 469.0201599999903692151
Permutation testing: Unpermuted result numDirectionalRuns = 78311
Permutation testing: Unpermuted result lenDirectionalRuns = 11
Permutation testing: Unpermuted result numIncreasesDecreases = 76742
Permutation testing: Unpermuted result numRunsMedian = 375247
Permutation testing: Unpermuted result lenRunsMedian = 41
Permutation testing: Unpermuted result avgCollision = 9.736641221374044974368
Permutation testing: Unpermuted result maxCollision = 34
Permutation testing: Unpermuted result periodicity(1) = 28607
Permutation testing: Unpermuted result periodicity(2) = 28732
Permutation testing: Unpermuted result periodicity(8) = 28782
Permutation testing: Unpermuted result periodicity(16) = 28697
Permutation testing: Unpermuted result periodicity(32) = 28676
Permutation testing: Unpermuted result covariance(1) = 500849
Permutation testing: Unpermuted result covariance(2) = 501292
Permutation testing: Unpermuted result covariance(8) = 499860
Permutation testing: Unpermuted result covariance(16) = 502091
Permutation testing: Unpermuted result covariance(32) = 500495
Permutation testing: Unpermuted result compression = 136090


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion       6       0      28
       numDirectionalRuns      13       0       6
       lenDirectionalRuns       4       5       1
    numIncreasesDecreases      39       0       6
            numRunsMedian      11       0       6
            lenRunsMedian      15       3       3
             avgCollision       6       0      88
             maxCollision       5       1       6
           periodicity(1)      11       0       6
           periodicity(2)       8       0       6
           periodicity(8)       6       0      18
          periodicity(16)       6       0       6
          periodicity(32)       5       1       6
            covariance(1)      22       0       6
            covariance(2)       6       0       7
            covariance(8)    2291       0       6
           covariance(16)       6       0      85
           covariance(32)      44       0       6
              compression      37       0       6
(* denotes failed test)

IID permutation tests: Passed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.301491)
LRS test (5.2.5): passed
Permutation tests (5.1): passed
H_original: 0.413507 bits per symbol
IID assumption holds: H_I = 0.413507 bits per symbol
