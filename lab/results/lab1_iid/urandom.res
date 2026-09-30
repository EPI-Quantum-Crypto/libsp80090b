2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/urandom.bin'
Loaded 1000000 samples of 256 distinct 8-bit-wide symbols
Number of Binary samples: 8000000
Raw Mean = 127.47054
Median = 127
Binary = false
Literal Most Common Value Estimate: Mode count = 4074
Literal Most Common Value Estimate: p-hat = 0.0040740000000000004
Literal Most Common Value Estimate: p_u = 0.0042380745962540692
Literal Most Common Value Estimate: min entropy = 7.8823753032052659
Bitstring Most Common Value Estimate: Mode count = 4001011
Bitstring Most Common Value Estimate: p-hat = 0.50012637500000001
Bitstring Most Common Value Estimate: p_u = 0.50058172160584447
Bitstring Most Common Value Estimate: min entropy = 0.99832248190754491
H_bitstring = 0.99832248190754491
H_bitstring Per Symbol = 7.9865798552603593
H_original = 7.8823753032052659
Assessed min entropy: 7.8823753032052659
Chi square independence: T = 65665.65813892943
Chi square independence: df = 65280
Chi square independence: P-value = 0.14296868150790537
Chi square goodness of fit: T = 2335.9495235382246
Chi square goodness of fit: df = 2295
Chi square goodness of fit: P-value = 0.27070141950745968
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.003907207722000000000772
Literal Longest Repeated Substring results: W = 4
Literal Longest Repeated Substring results: Pr(X >= 1) = 1
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 79354.33998000012070406
Permutation testing: Unpermuted result numDirectionalRuns = 666444
Permutation testing: Unpermuted result lenDirectionalRuns = 9
Permutation testing: Unpermuted result numIncreasesDecreases = 501820
Permutation testing: Unpermuted result numRunsMedian = 499768
Permutation testing: Unpermuted result lenRunsMedian = 19
Permutation testing: Unpermuted result avgCollision = 20.75584798356130278307
Permutation testing: Unpermuted result maxCollision = 70
Permutation testing: Unpermuted result periodicity(1) = 3940
Permutation testing: Unpermuted result periodicity(2) = 3815
Permutation testing: Unpermuted result periodicity(8) = 3872
Permutation testing: Unpermuted result periodicity(16) = 3973
Permutation testing: Unpermuted result periodicity(32) = 4009
Permutation testing: Unpermuted result covariance(1) = 16251664680
Permutation testing: Unpermuted result covariance(2) = 16251474367
Permutation testing: Unpermuted result covariance(8) = 16253997513
Permutation testing: Unpermuted result covariance(16) = 16255774723
Permutation testing: Unpermuted result covariance(32) = 16241455913
Permutation testing: Unpermuted result compression = 1067842


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion       6       0      27
       numDirectionalRuns       6       0       7
       lenDirectionalRuns       0       6      10
    numIncreasesDecreases      17       0       6
            numRunsMedian       6       0      12
            lenRunsMedian       9       5       1
             avgCollision       6       0      13
             maxCollision      30       3       3
           periodicity(1)       6       0       6
           periodicity(2)      31       0       6
           periodicity(8)      19       1       5
          periodicity(16)       6       0      26
          periodicity(32)       6       0     123
            covariance(1)      11       0       6
            covariance(2)       6       0      13
            covariance(8)       6       0      39
           covariance(16)       6       0     120
           covariance(32)      13       0       6
              compression       6       0      17
(* denotes failed test)

IID permutation tests: Passed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.142969)
LRS test (5.2.5): passed
Permutation tests (5.1): passed
H_original: 7.882375 bits per symbol
H_bitstring: 0.998322 bits per bit
IID assumption holds: H_I = 7.882375 bits per symbol
