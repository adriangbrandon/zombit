/***
BSD 2-Clause License

Copyright (c) 2018, Adrián
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


//
// Created by Adrián on 27/6/22.
//

#ifndef SUCC_VECTORS_UTIL_HPP
#define SUCC_VECTORS_UTIL_HPP

#include <sdsl/bits.hpp>


namespace sdsl {

    namespace bits_more {

        inline uint64_t next_limit(const uint64_t* word, uint64_t idx, uint64_t limit)
        {
            word += (idx>>6);
            if (*word & ~sdsl::bits::lo_set[idx&0x3F]) {
                return (idx & ~((size_t)0x3F)) + sdsl::bits::lo(*word & ~sdsl::bits::lo_set[idx&0x3F]);
            }
            idx = (idx & ~((size_t)0x3F)) + 64;
            ++word;
            while (*word==0 && idx < limit) {
                idx += 64;
                ++word;
            }
            if(idx < limit) {
                return idx + sdsl::bits::lo(*word);
            }
            return limit;
        }
    }
}

#endif //SUCC_VECTORS_UTIL_HPP
