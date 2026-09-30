2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/urandom-4bit.bin'
Loaded 1000000 samples of 16 distinct 4-bit-wide symbols
Number of Binary samples: 4000000
Raw Mean = 7.4967790000000001
Median = 7
Binary = false
Literal Most Common Value Estimate: Mode count = 63078
Literal Most Common Value Estimate: p-hat = 0.063077999999999995
Literal Most Common Value Estimate: p_u = 0.063704192659791298
Literal Most Common Value Estimate: min entropy = 3.9724678639110831
Bitstring Most Common Value Estimate: Mode count = 2000692
Bitstring Most Common Value Estimate: p-hat = 0.50017299999999998
Bitstring Most Common Value Estimate: p_u = 0.50081695736783594
Bitstring Most Common Value Estimate: min entropy = 0.99764468298363385
H_bitstring = 0.99764468298363385
H_bitstring Per Symbol = 3.9905787319345354
H_original = 3.9724678639110831
Assessed min entropy: 3.9724678639110831
Chi square independence: T = 236.21875861624716
Chi square independence: df = 240
Chi square independence: P-value = 0.55686630491158851
Chi square goodness of fit: T = 122.77075334025974
Chi square goodness of fit: df = 135
Chi square goodness of fit: P-value = 0.76641399191541582
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.06250113180999999912816
Literal Longest Repeated Substring results: W = 9
Literal Longest Repeated Substring results: Pr(X >= 1) = 0.9993087574822720564149
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 3834.492018000029474933
Permutation testing: Unpermuted result numDirectionalRuns = 664389
Permutation testing: Unpermuted result lenDirectionalRuns = 9
Permutation testing: Unpermuted result numIncreasesDecreases = 531006
Permutation testing: Unpermuted result numRunsMedian = 492379
Permutation testing: Unpermuted result lenRunsMedian = 24
Permutation testing: Unpermuted result avgCollision = 5.706019297815158708431
Permutation testing: Unpermuted result maxCollision = 16
Permutation testing: Unpermuted result periodicity(1) = 62510
Permutation testing: Unpermuted result periodicity(2) = 62423
Permutation testing: Unpermuted result periodicity(8) = 62462
Permutation testing: Unpermuted result periodicity(16) = 62881
Permutation testing: Unpermuted result periodicity(32) = 62894
Permutation testing: Unpermuted result covariance(1) = 56198366
Permutation testing: Unpermuted result covariance(2) = 56235978
Permutation testing: Unpermuted result covariance(8) = 56195375
Permutation testing: Unpermuted result covariance(16) = 56182692
Permutation testing: Unpermuted result covariance(32) = 56179112
Permutation testing: Unpermuted result compression = 543797


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion       8       0       6
       numDirectionalRuns       6       0      17
       lenDirectionalRuns       0       6       1
    numIncreasesDecreases      20       0       6
            numRunsMedian       6       0      17
            lenRunsMedian       4       2      23
             avgCollision       7       0       6
             maxCollision       1       6       0
           periodicity(1)       6       0       6
           periodicity(2)      15       0       6
           periodicity(8)       9       0       6
          periodicity(16)       6       0      81
          periodicity(32)       6       0      43
            covariance(1)      17       0       6
            covariance(2)       6       0      80
            covariance(8)       6       0       9
           covariance(16)      35       0       6
           covariance(32)      13       0       6
              compression      21       0       6
(* denotes failed test)

IID permutation tests: Passed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.556866)
LRS test (5.2.5): passed
Permutation tests (5.1): passed
H_original: 3.972468 bits per symbol
H_bitstring: 0.997645 bits per bit
IID assumption holds: H_I = 3.972468 bits per symbol
