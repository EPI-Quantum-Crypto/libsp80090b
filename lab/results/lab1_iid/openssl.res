2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)
Loaded 1000000 samples from 'data/lab1_iid/openssl.bin'
Loaded 1000000 samples of 256 distinct 8-bit-wide symbols
Number of Binary samples: 8000000
Raw Mean = 127.506384
Median = 128
Binary = false
Literal Most Common Value Estimate: Mode count = 4034
Literal Most Common Value Estimate: p-hat = 0.0040340000000000003
Literal Most Common Value Estimate: p_u = 0.004197270416298594
Literal Most Common Value Estimate: min entropy = 7.8963328703576474
Bitstring Most Common Value Estimate: Mode count = 4000924
Bitstring Most Common Value Estimate: p-hat = 0.50011550000000005
Bitstring Most Common Value Estimate: p_u = 0.50057084660823992
Bitstring Most Common Value Estimate: min entropy = 0.9983538243934208
H_bitstring = 0.9983538243934208
H_bitstring Per Symbol = 7.9868305951473664
H_original = 7.8963328703576474
Assessed min entropy: 7.8963328703576474
Chi square independence: T = 65601.641356097723
Chi square independence: df = 65280
Chi square independence: P-value = 0.18658695020985838
Chi square goodness of fit: T = 2332.642036826107
Chi square goodness of fit: df = 2295
Chi square goodness of fit: P-value = 0.28691362225454259
Chi square tests: Passed
Literal Longest Repeated Substring results: P_col = 0.003907094801999999987715
Literal Longest Repeated Substring results: W = 4
Literal Longest Repeated Substring results: Pr(X >= 1) = 1
Length of longest repeated substring test: Passed
Permutation testing: Unpermuted result excursion = 33659.03249599795526592
Permutation testing: Unpermuted result numDirectionalRuns = 665592
Permutation testing: Unpermuted result lenDirectionalRuns = 9
Permutation testing: Unpermuted result numIncreasesDecreases = 501845
Permutation testing: Unpermuted result numRunsMedian = 499369
Permutation testing: Unpermuted result lenRunsMedian = 18
Permutation testing: Unpermuted result avgCollision = 20.76380813953488413404
Permutation testing: Unpermuted result maxCollision = 70
Permutation testing: Unpermuted result periodicity(1) = 3762
Permutation testing: Unpermuted result periodicity(2) = 3975
Permutation testing: Unpermuted result periodicity(8) = 4111
Permutation testing: Unpermuted result periodicity(16) = 3746
Permutation testing: Unpermuted result periodicity(32) = 3989
Permutation testing: Unpermuted result covariance(1) = 16268853597
Permutation testing: Unpermuted result covariance(2) = 16254544898
Permutation testing: Unpermuted result covariance(8) = 16262413947
Permutation testing: Unpermuted result covariance(16) = 16254084596
Permutation testing: Unpermuted result covariance(32) = 16256678512
Permutation testing: Unpermuted result compression = 1067915


                statistic  C[i][0]  C[i][1]  C[i][2]
----------------------------------------------------
                excursion     369       0       6
       numDirectionalRuns     518       0       6
       lenDirectionalRuns       0       6      10
    numIncreasesDecreases      10       0       6
            numRunsMedian      44       0       6
            lenRunsMedian      39       5       1
             avgCollision       6       0      24
             maxCollision      17       2       4
           periodicity(1)     620       0       6
           periodicity(2)       6       0      35
          periodicity(8)*       3       0    9997
          periodicity(16)    1499       0       6
          periodicity(32)       6       0     131
            covariance(1)       6       0      84
            covariance(2)      13       0       6
            covariance(8)       6       0      30
           covariance(16)      24       0       6
           covariance(32)      10       0       6
              compression       5       1      70
(* denotes failed test)

IID permutation tests: Failed
Chi-square tests (5.2.1-5.2.4): passed (smallest p-value 0.186587)
LRS test (5.2.5): passed
Permutation tests (5.1): FAILED
H_original: 7.896333 bits per symbol
H_bitstring: 0.998354 bits per bit
IID assumption rejected: use the non-IID track (test_non_iid)
