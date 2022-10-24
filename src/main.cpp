/***
BSD 2-Clause License

Copyright (c) 2018, <author_name>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#include <succ_support_v.hpp>
#include <zombit_vector.hpp>
#include <time.hpp>

uint64_t check_succ(uint64_t i, sdsl::bit_vector &bm){
    for(uint64_t j = i; j < bm.size(); ++j){
        if(bm[j]) return j;
    }
    return bm.size();
}


//ratio:
sdsl::bit_vector generate(uint64_t size, uint64_t ratio){

    sdsl::bit_vector bv = sdsl::bit_vector(size, 0);
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> distribution(0, bv.size()-1);
    auto dice = bind(distribution, rng);
    // populate vectors with some other bits
    for (uint64_t i=0; i < bv.size()/ratio; ++i) {
        uint64_t x = dice();
        bv[x] = 1;
    }
    return bv;
}


sdsl::bit_vector generate_runs(uint64_t size, double mean_1, double stdev_1, double mean_0, double stdev_0) {

    sdsl::bit_vector bm(size);
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    //Error con seed=1380572158; distribution_zeroes (1000, 100); distribution_ones (100, 10);
    //unsigned seed = 1380572158;
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution_zeroes(mean_0, stdev_0);
    std::normal_distribution<double> distribution_ones(mean_1, stdev_1);

    auto index = 0;
    bool value = false;
    while (index < bm.size()) {
        int64_t run_size = 0;
        if (value) {
            run_size = std::abs(static_cast<int64_t >(std::ceil(distribution_ones(generator))));
        } else {
            run_size = std::abs(static_cast<int64_t >(std::ceil(distribution_zeroes(generator))));
        }
        //std::cout << "run_size: " << run_size << std::endl;
        for (uint64_t i = 0; i < run_size; ++i) {
            bm[index] = value;
            ++index;
            if (index >= bm.size()) break;
        }
        value = !value;
    }
    return bm;
}

void plain_test(sdsl::bit_vector &bm) {

    runs_vectors::succ_support_v<1> bm_succ;
    sdsl::util::init_support(bm_succ, &bm);
    for(size_t i = 0; i < bm.size(); ++i){
        if(bm_succ(i) != check_succ(i, bm)){
            sdsl::store_to_file(bm, "error.bin");
            std::cout << "Error in Plain-succ at i=" << i << std::endl;
            std::cout << "Expected=" << bm_succ(i) << std::endl;
            std::cout << "Obtained=" << check_succ(i, bm) << std::endl;
            exit(0);
        }
    }
}

void zombit_test(sdsl::bit_vector &bm) {

    runs_vectors::zombit_vector zombit(bm);
    runs_vectors::succ_support_zombit<1> zombit_succ;
    sdsl::util::init_support(zombit_succ, &zombit);
    for(size_t i = 0; i < bm.size(); ++i){
        if(zombit_succ(i) != check_succ(i, bm)){
            sdsl::store_to_file(bm, "erro.bin");
            std::cout << "Error in Zombit-succ at i=" << i << std::endl;
            std::cout << "Expected=" << zombit_succ(i) << std::endl;
            std::cout << "Obtained=" << check_succ(i, bm) << std::endl;
            exit(0);
        }
    }

}

void plain_exp(sdsl::bit_vector &bm) {


    runs_vectors::succ_support_v<1> bm_succ;
    sdsl::rank_support_v<1> bm_rank;
    sdsl::select_support_mcl<1> bm_select;
    sdsl::util::init_support(bm_succ, &bm);
    sdsl::util::init_support(bm_rank, &bm);
    sdsl::util::init_support(bm_select, &bm);

    auto t0 = util::time::user::now();
    for(size_t i = 0; i < bm.size(); ++i){
        bm_succ(i);
    }
    auto t1 = util::time::user::now();
    auto time = util::time::duration_cast<util::time::microseconds>(t1-t0);
    std::cout << "Plain-succ: size= " << sdsl::size_in_bytes(bm) + sdsl::size_in_bytes(bm_succ)
              << "(bytes) succ= " << time << " (µs)" << std::endl;

    t0 = util::time::user::now();
    for(size_t i = 0; i < bm.size(); ++i){
        bm_select(bm_rank(i)+1);
    }
    t1 = util::time::user::now();
    time = util::time::duration_cast<util::time::microseconds>(t1-t0);
    std::cout << "Plain-R+S: size= " << sdsl::size_in_bytes(bm) + sdsl::size_in_bytes(bm_rank)+ sdsl::size_in_bytes(bm_select)
              << "(bytes) succ= " << time << " (µs)" << std::endl;
}

void zombit_exp(sdsl::bit_vector &bm) {

    runs_vectors::zombit_vector zombit(bm);
    runs_vectors::succ_support_zombit<1> zombit_succ;
    sdsl::util::init_support(zombit_succ, &zombit);
    auto t0 = util::time::user::now();
    for(size_t i = 0; i < bm.size(); ++i){
        zombit_succ(i);
    }
    auto t1 = util::time::user::now();
    auto time = util::time::duration_cast<util::time::microseconds>(t1-t0);
    std::cout << "Zombit: size= " << sdsl::size_in_bytes(zombit) + sdsl::size_in_bytes(zombit_succ)
              << "(bytes) succ= " << time << " (µs)" << std::endl;


}



int main(int argc, char** argv) {

    if(argc != 3){
        std::cout << argv[0] << " <size> <test>" << std::endl;
        exit(0);
    }

    auto size = std::atoll(argv[1]);
    bool test = std::atoi(argv[2]);
    if(test) {
        auto mean = 10;
        auto stdev = 2;
        while (mean < size) {
            std::cout << "Test (mean_1=" << mean << ", stdev_1=" << stdev << ", mean_0="
                      << mean << ", stdev_0=" << stdev << ", size=" << size << ")" << std::endl;
            for (auto step = 0; step < 100; ++step) {
                auto bv = generate_runs(size, mean, stdev, mean, stdev);
                plain_test(bv);
                zombit_test(bv);
            }
            mean = mean * 10;
            stdev = stdev * 5;
        }

        for (auto ratio = 2; ratio < 100; ratio *= 2) {
            mean = 10;
            stdev = 2;
            while (mean < size) {
                auto mean_1 = std::max(1, mean / ratio);
                auto stdev_1 = std::max(1, stdev / ratio);
                std::cout << "Test (mean_1=" << mean_1 << ", stdev_1=" << stdev_1 << ", mean_0="
                          << mean << ", stdev_0=" << stdev << ", size=" << size << ")" << std::endl;
                for (auto step = 0; step < 100; ++step) {
                    auto bv = generate_runs(size, mean_1, stdev_1, mean, stdev);
                    plain_test(bv);
                    zombit_test(bv);
                }
                mean = mean * 10;
                stdev = stdev * 5;
            }
        }
        std::cout << "Everything is ok" << std::endl;
    }
    std::cout << "--- Benchmark ---" << std::endl;
    auto mean = 10;
    auto stdev = 2;
    while (mean < size) {
        std::cout << "Exp (mean_1=" << mean << ", stdev_1=" << stdev << ", mean_0="
                  << mean << ", stdev_0=" << stdev << ", size=" << size << ")" << std::endl;
        auto bv = generate_runs(size, mean, stdev, mean, stdev);
        std::cout << "------------------" << std::endl;
        plain_exp(bv);
        zombit_exp(bv);
        std::cout << "------------------" << std::endl;
        mean = mean * 10;
        stdev = stdev * 5;
    }

    for (auto ratio = 2; ratio < 100; ratio *= 2) {
        mean = 10;
        stdev = 2;
        while (mean < size) {
            auto mean_1 = std::max(1, mean / ratio);
            auto stdev_1 = std::max(1, stdev / ratio);
            std::cout << "Exp (mean_1=" << mean_1 << ", stdev_1=" << stdev_1 << ", mean_0="
                      << mean << ", stdev_0=" << stdev << ", size=" << size << ")" << std::endl;
            auto bv = generate_runs(size, mean_1, stdev_1, mean, stdev);
            std::cout << "------------------" << std::endl;
            plain_exp(bv);
            zombit_exp(bv);
            std::cout << "------------------" << std::endl;
            mean = mean * 10;
            stdev = stdev * 5;
        }
    }
}