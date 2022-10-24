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
// Created by Adrián on 17/12/2018.
//

#ifndef RUNS_VECTORS_ZOMBIT_VECTOR_HPP
#define RUNS_VECTORS_ZOMBIT_VECTOR_HPP

#include <sdsl/int_vector.hpp>
#include <sdsl/rank_support.hpp>
#include <algorithm>
#include <cstdint>
#include "succ_support_v.hpp"
#include <sdsl/select_support.hpp>

namespace runs_vectors {

    template<uint8_t t_b = 1> class rank_support_zombit;
    template<uint8_t t_b = 1> class succ_support_zombit_naive;
    template<uint8_t t_b = 1> class succ_support_zombit;
    class rec_zombit_vector;
    template<uint8_t t_b = 1> class rank_support_zombit_rec;
    template<uint8_t t_b = 1> class succ_support_zombit_rec;

    class zombit_vector {

    public:
        typedef sdsl::bit_vector bitmap_type;
        typedef sdsl::bit_vector values_type;
        typedef typename sdsl::bit_vector::rank_1_type rank_bitmap_type;
        typedef typename sdsl::bit_vector::size_type size_type;
        typedef typename sdsl::bit_vector::value_type value_type;

        friend class rank_support_zombit<1>;
        friend class rank_support_zombit<0>;
        friend class succ_support_zombit_naive<1>;
        friend class succ_support_zombit_naive<0>;
        friend class succ_support_zombit<1>;
        friend class succ_support_zombit<0>;
        friend class rec_zombit_vector;
        friend class rank_support_zombit_rec<1>;
        friend class rank_support_zombit_rec<0>;
        friend class succ_support_zombit_rec<1>;
        friend class succ_support_zombit_rec<0>;
    private:

        size_type m_size = 0;
        size_type m_sample;
        bitmap_type m_full;
        rank_bitmap_type m_rank_full;
        bitmap_type m_full_type;
        rank_bitmap_type m_rank_full_type;
        values_type m_mixed;

        void copy(const zombit_vector &o){
            m_size = o.m_size;
            m_sample = o.m_sample;
            m_full = o.m_full;
            m_rank_full = o.m_rank_full;
            m_rank_full.set_vector(&m_full);
            m_full_type = o.m_full_type;
            m_rank_full_type = o.m_rank_full_type;
            m_rank_full_type.set_vector(&m_full_type);
            m_mixed = o.m_mixed;
        }

        void compute_runs(const sdsl::bit_vector &c, std::vector<size_type> &runs){
            size_type i = 0;
            runs.push_back(i);
            bool ones = static_cast<bool>(c[0]);
            while(i < c.size()){
                if(c[i] && !ones){ //first element of a run with 1s
                    ones = true;
                    runs.push_back(i);
                }else if (!c[i] && ones){ // first element of a run with 0s
                    ones = false;
                    runs.push_back(i);
                }
                ++i;
            }
            runs.push_back(i);
        }



    public:

        const size_type &sample = m_sample;
        const values_type &mixed = m_mixed;

        zombit_vector() = default;

        explicit zombit_vector(const sdsl::bit_vector &c){

            if(c.empty()) return;

            //Computing runs and sample
            m_size = c.size();
            std::vector<size_type > runs;
            compute_runs(c, runs);
            auto s = static_cast<size_type >(std::ceil(std::sqrt(m_size/ (double_t) runs.size())));
            //std::cout << "s: " << s << std::endl;
            m_sample = (s + 7) & (-8);
            //std::cout << "sample: " << m_sample << std::endl;

            //Computing full and mixed blocks
            auto n_blocks = static_cast<size_type >(std::ceil(m_size/ (double_t) sample));
            m_full = sdsl::bit_vector(n_blocks, 0);
            m_full_type = sdsl::bit_vector(n_blocks, 0);

            //Computing full and mixed blocks
            bool is_run_ones = (c[0] == 0);
            size_type start_block = 0, end_block = sample;
            size_type ith_run = 0, ith_full_type = 0;
            std::vector<size_type> mixed_blocks;
            for(size_type ith_block = 0; ith_block <n_blocks; ++ith_block){
                if(runs[ith_run] > start_block && runs[ith_run] < end_block){
                    // [ ___------______ ]
                    //mixed
                    /*while(ith_run < runs.size() && runs[ith_run] < end_block){
                        ++ith_run;
                    }*/
                    m_full[ith_block] = 0;
                    mixed_blocks.push_back(ith_block);
                }else if(runs[ith_run] == start_block){
                    if(runs[ith_run+1] < end_block){
                        // [ --------_____ ]
                        //mixed
                        /*while(ith_run < n_blocks && runs[ith_run] < end_block){
                            ++ith_run;
                        }*/
                        m_full[ith_block] = 0;
                        mixed_blocks.push_back(ith_block);
                    }else{
                        // [ -------------- ]
                        //full
                        m_full[ith_block] = 1;
                        m_full_type[ith_full_type] = (ith_run + c[0]) % 2;
                        ++ith_full_type;
                    }

                }else{
                    // [ -------------- ]
                    //full
                    m_full[ith_block] = 1;
                    m_full_type[ith_full_type] = (ith_run + 1 + c[0]) % 2;
                    ++ith_full_type;
                }
                while(ith_run < runs.size() && runs[ith_run] < end_block){
                    ++ith_run;
                }

                start_block = end_block;
                end_block += sample;
            }
            //tricks for avoiding if statements in succ
            m_full_type.resize(ith_full_type+1);
            m_full_type[ith_full_type] = 1;
            m_full.resize(m_full.size()+2);
            m_full[m_full.size()-2] = 0;
            m_full[m_full.size()-1] = 1;
            //Storing info of mixed blocks
            m_mixed = sdsl::bit_vector(mixed_blocks.size()*sample);
            size_type ith_mixed = 0;
            for(const auto &mixed_block : mixed_blocks){
                start_block = mixed_block * sample;
                end_block = std::min(start_block + sample, c.size());
                for(size_type i = start_block; i < end_block; ++i){
                    m_mixed[ith_mixed] = c[i];
                    ++ith_mixed;
                }
            }
            sdsl::util::init_support(m_rank_full, &m_full);
            sdsl::util::init_support(m_rank_full_type, &m_full_type);
        }

        //! Copy constructor
        zombit_vector(const zombit_vector& o)
        {
            copy(o);
        }

        //! Move constructor
        zombit_vector(zombit_vector&& o)
        {
            *this = std::move(o);
        }


        zombit_vector &operator=(const zombit_vector &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }
        zombit_vector &operator=(zombit_vector &&o) {
            if (this != &o) {
                m_size = o.m_size;
                m_sample = o.m_sample;
                m_full = std::move(o.m_full);
                m_rank_full = std::move(o.m_rank_full);
                m_rank_full.set_vector(&m_full);
                m_full_type = std::move(o.m_full_type);
                m_rank_full_type = std::move(o.m_rank_full_type);
                m_rank_full_type.set_vector(&m_full_type);
                m_mixed = std::move(o.m_mixed);
            }
            return *this;
        }

        void swap(zombit_vector &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_size, o.m_size);
            std::swap(m_sample, o.m_sample);
            m_full.swap(o.m_full);
            sdsl::util::swap_support(m_rank_full, o.m_rank_full, &m_full, &o.m_full);
            m_full_type.swap(o.m_full_type);
            sdsl::util::swap_support(m_rank_full_type, o.m_rank_full_type, &m_full_type, &o.m_full_type);
            m_mixed.swap(o.m_mixed);
        }

        inline value_type access(const size_type i) const{

            //std::cout << "access at " << i << std::endl;
            auto j = i / sample;
            if(m_full[j]){
                //std::cout << "full: " << j << "(" << m_full.size() << ")" << std::endl;
                //std::cout << "rank: " << m_rank_full(j+1) << std::endl;
                //std::cout << "value: " << m_full_type[m_rank_full(j+1)-1] << std::endl;
                return m_full_type[m_rank_full(j+1)-1];
            }else{
                auto i_mixed = j - m_rank_full(j+1);
                //std::cout << "mixed: " << i_mixed << std::endl;
                return m_mixed[i_mixed*sample + i%sample];
            }

        }

        inline value_type operator[](const size_type i) const{
            return access(i);
        }

        inline size_type size() const {
            return m_size;
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_full.serialize(out, child, "full");
            written_bytes += m_rank_full.serialize(out, child, "rank_full");
            written_bytes += m_full_type.serialize(out, child, "full_type");
            written_bytes += m_rank_full_type.serialize(out, child, "rank_full_type");
            written_bytes += m_mixed.serialize(out, child, "mixed");
            written_bytes += sdsl::serialize(m_size, out, child, "size");
            written_bytes += sdsl::serialize(m_sample, out, child, "sample");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream& in)
        {
            m_full.load(in);
            m_rank_full.load(in, &m_full);
            m_full_type.load(in);
            m_rank_full_type.load(in, &m_full_type);
            m_mixed.load(in);
            sdsl::load(m_size, in);
            sdsl::load(m_sample, in);
        }

        void print(){
            std::cout << "sample=" << sample << std::endl;
            std::cout << "full={";
            for(auto i = 0; i < m_full.size(); ++i){
                std::cout << m_full[i] << ", ";
            }
            std::cout << "}" << std::endl;

            std::cout << "full_type={";
            for(auto i = 0; i < m_full_type.size(); ++i){
                std::cout << m_full_type[i] << ", ";
            }
            std::cout << "}" << std::endl;

            std::cout << "mixed={";
            for(auto i = 0; i < m_mixed.size(); ++i){
                std::cout << m_mixed[i] << ", ";
            }
            std::cout << "}" << std::endl;
        }

    };


    template<uint8_t t_b>
    struct rank_support_zombit_trait {
        typedef zombit_vector::size_type size_type;
        static size_type adjust_rank(size_type r,size_type)
        {
            return r;
        }
    };

    template<>
    struct rank_support_zombit_trait<0> {
        typedef zombit_vector::size_type size_type;
        static size_type adjust_rank(size_type r, size_type n)
        {
            return n - r;
        }
    };

    template<uint8_t t_b>
    class rank_support_zombit {

    public:
        friend class rank_support_zombit_rec<1>;
        friend class rank_support_zombit_rec<0>;
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
        typedef typename sdsl::bit_vector::rank_1_type rank_mixed_type;
    private:
        const zombit_vector* m_v = nullptr;
        rank_mixed_type m_rank_mixed;

        void copy(const rank_support_zombit& ss){
            m_v = ss.m_v;
            m_rank_mixed = ss.m_rank_mixed;
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->mixed));
            }
        }
    public:


        rank_support_zombit() = default;

        explicit rank_support_zombit(const zombit_vector* v)
        {
            m_v = v;
            sdsl::util::init_support(m_rank_mixed, &(m_v->m_mixed));

        }


        //! Returns the position of the i-th occurrence in the bit vector.
        size_type rank(size_type i)const
        {
            assert(i >= 0); assert(i <= m_v->size());
            if(i <= 0) return 0;
            auto j = (i-1) / m_v->sample; //block
            auto full = m_v->m_rank_full(j); // Z and O blocks before j
            auto mixed = j - full; // M blocks before j
            size_type r = m_v->m_rank_full_type(full) * m_v->sample;
            if(m_v->m_full[j]){
                if(m_v->m_full_type[full]){
                    // r contains all the ones in O blocks
                    // mixed*sample= position where the mixed-th mixed block ends+1
                    // (i-1)%sample position of the (i-1) bit inside the full block
                    // +1 because we are counting
                    r = r + m_rank_mixed(mixed*m_v->sample) + ((i-1) % m_v->sample)+1;
                }else{
                    //r contains all the ones in O blocks
                    // mixed*sample= position where the (mixed+1)-th M block starts
                    r = r + m_rank_mixed(mixed*m_v->sample);
                }
            }else{
                //r contains all the ones in O blocks
                // (i-1)%sample position of the (i-1) bit inside the block
                // +1 because we need an inclusive rank
                r = r + m_rank_mixed(mixed*m_v->sample + (i-1) % m_v->sample + 1);
            }
            return rank_support_zombit_trait<t_b>::adjust_rank(r, i);
        }


        size_type operator()(size_type i)const
        {
            return rank(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const zombit_vector* v=nullptr)
        {
            m_v = v;
            if(m_v != nullptr){
                sdsl::util::init_support(m_rank_mixed, &(m_v->mixed));
            }

        }

        rank_support_zombit& operator=(const rank_support_zombit& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        rank_support_zombit& operator=(rank_support_zombit& ss)
        {

            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_rank_mixed = std::move(ss.m_rank_mixed);
                if(m_v != nullptr){
                    m_rank_mixed.set_vector(&(m_v->mixed));
                }
            }
            return *this;
        }

        void swap(rank_support_zombit& o) {
            std::swap(m_v, o.m_v);
            m_rank_mixed.swap(o.m_rank_mixed);
            if(m_v != nullptr){
                m_rank_mixed.set_vector(&(m_v->mixed));
            }else{
                m_rank_mixed.set_vector(nullptr);
            }
            if(o.m_v != nullptr){
                o.m_rank_mixed.set_vector(&(o.m_v->mixed));
            }else{
                o.m_rank_mixed.set_vector(nullptr);
            }


        }

        void load(std::istream& in, const zombit_vector* v=nullptr)
        {
            m_v = v;
            if(v != nullptr){
                m_rank_mixed.load(in, &(m_v->mixed));
            }
        }

        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_rank_mixed.serialize(out, child, "rank_mixed");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };


    template <uint8_t t_b>
    class succ_support_zombit {

    public:
        friend class succ_support_zombit_rec<1>;
        friend class succ_support_zombit_rec<0>;
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
    private:
        const zombit_vector* m_v = nullptr;
        sdsl::bit_vector::select_1_type m_select_full;
        succ_support_v<0>   m_succ_no_full;
        succ_support_v<t_b> m_succ_full_type;
        succ_support_v<t_b> m_succ_mixed;


        void copy(const succ_support_zombit& ss){
            m_v = ss.m_v;
            m_select_full = ss.m_select_full;
            m_succ_no_full = ss.m_succ_no_full;
            m_succ_full_type = ss.m_succ_full_type;
            m_succ_mixed = ss.m_succ_mixed;
            if(m_v != nullptr){
                m_select_full.set_vector(&(m_v->m_full));
                m_succ_no_full.set_vector(&(m_v->m_full));
                m_succ_mixed.set_vector(&(m_v->m_mixed));
                m_succ_full_type.set_vector(&(m_v->m_full_type));
            }
        }

    public:

        succ_support_zombit() = default;


        explicit succ_support_zombit(const zombit_vector* v)
        {
            m_v = v;
            if(m_v != nullptr){
                sdsl::util::init_support(m_select_full, &(m_v->m_full));
                sdsl::util::init_support(m_succ_no_full, &(m_v->m_full));
                sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
                sdsl::util::init_support(m_succ_full_type, &(m_v->m_full_type));
            }
        }

        inline size_type succ(size_type i) const{
            auto j = i /m_v->sample;
            auto p = m_v->m_rank_full(j+1);
            auto q = j+1-p;
            if(m_v->m_full[j]){
                if(m_v->m_full_type[p-1] == t_b) {
                    return i;
                }
            }else{
                auto next_in_mixed = m_succ_mixed((q-1)*m_v->sample + i %m_v->sample);
                if(next_in_mixed < q*m_v->sample) {
                    return next_in_mixed - (q - 1) * m_v->sample + j * m_v->sample;
                }
            }
            size_type next_mixed = m_succ_no_full(j+1);
            size_type n_f = m_succ_full_type(p);
            size_type next_full_type = m_select_full(n_f+1);
            size_type r;
            if(next_mixed > next_full_type){
                r =  next_full_type * m_v->sample;
            }else {
                r = next_mixed * m_v->sample + m_succ_mixed(q*m_v->sample) - q*m_v->sample;
            }
            return std::min(r, m_v->size());
        };


        size_type operator()(size_type i)const
        {
            return succ(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const zombit_vector* v=nullptr)
        {
            m_v = v;
            if(v != nullptr){
                sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
                sdsl::util::init_support(m_select_full, &(m_v->m_full));
                sdsl::util::init_support(m_succ_no_full, &(m_v->m_full));
                sdsl::util::init_support(m_succ_full_type, &(m_v->m_full_type));
            }
        }

        succ_support_zombit& operator=(const succ_support_zombit& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        succ_support_zombit& operator=(succ_support_zombit&& ss)
        {
            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_select_full = std::move(ss.m_select_full);
                m_succ_no_full = std::move(ss.m_succ_no_full);
                m_succ_full_type = std::move(ss.m_succ_full_type);
                m_succ_mixed = std::move(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_select_full.set_vector(&(m_v->m_full));
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                    m_succ_no_full.set_vector(&(m_v->m_full));
                    m_succ_full_type.set_vector(&(m_v->m_full_type));
                }
            }
            return *this;
        }

        void swap(succ_support_zombit& ss) {
            if (this != &ss) {
                std::swap(m_v, ss.m_v);
                m_select_full.swap(ss.m_select_full);
                m_succ_no_full.swap(ss.m_succ_no_full);
                m_succ_full_type.swap(ss.m_succ_full_type);
                m_succ_mixed.swap(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_select_full.set_vector(&(m_v->m_full));
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                    m_succ_no_full.set_vector(&(m_v->m_full));
                    m_succ_full_type.set_vector(&(m_v->m_full_type));
                }else{
                    m_select_full.set_vector(nullptr);
                    m_succ_mixed.set_vector(nullptr);
                    m_succ_no_full.set_vector(nullptr);
                    m_succ_full_type.set_vector(nullptr);
                }
                if(ss.m_v != nullptr){
                    ss.m_select_full.set_vector(&(ss.m_v->m_full));
                    ss.m_succ_mixed.set_vector(&(ss.m_v->m_mixed));
                    ss.m_succ_no_full.set_vector(&(ss.m_v->m_full));
                    ss.m_succ_full_type.set_vector(&(ss.m_v->m_full_type));
                }else{
                    ss.m_select_full.set_vector(nullptr);
                    ss.m_succ_mixed.set_vector(nullptr);
                    ss.m_succ_no_full.set_vector(nullptr);
                    ss.m_succ_full_type.set_vector(nullptr);
                }
            }
        }

        void load(std::istream& in, const succ_support_zombit* v=nullptr)
        {
            m_v = v;
            if(m_v != nullptr){
                m_select_full.load(in, &(m_v->m_full));
                m_succ_mixed.load(in, &(m_v->m_mixed));
                m_succ_no_full.load(in, &(m_v->m_full));
                m_succ_full_type.load(in, &(m_v->m_full_type));
            }

        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_select_full.serialize(out, child, "select_full");
            written_bytes += m_succ_mixed.serialize(out, child, "succ_mixed");
            written_bytes += m_succ_no_full.serialize(out, child, "succ_no_full");
            written_bytes += m_succ_full_type.serialize(out, child, "succ_full_type");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };


    template <uint8_t t_b>
    class succ_support_zombit_naive {

        friend class succ_support_zombit_rec<1>;
        friend class succ_support_zombit_rec<0>;

    public:
        typedef sdsl::bit_vector::size_type size_type;
        typedef sdsl::bit_vector::value_type value_type;
    private:
        const zombit_vector* m_v;
        sdsl::bit_vector m_check_block;
        succ_support_v<1> m_next_check;
        succ_support_v<t_b> m_succ_mixed;


        void copy(const succ_support_zombit_naive& ss){
            m_v = ss.m_v;
            m_check_block = ss.m_check_block;
            m_next_check = ss.m_next_check;
            m_next_check.set_vector(&m_check_block);
            m_succ_mixed = ss.m_succ_mixed;
            if(m_v != nullptr){
                m_succ_mixed.set_vector(&(m_v->m_mixed));
            }else{
                m_succ_mixed.set_vector(nullptr);
            }
        }

    public:

        //! Copy constructor
        succ_support_zombit_naive(const succ_support_zombit_naive& hybrid)
        {
            copy(hybrid);
        }

        succ_support_zombit_naive(const zombit_vector* v = nullptr)
        {
            m_v = v;
            if(m_v == nullptr) return;
            m_check_block = sdsl::bit_vector(m_v->m_full.size()-2, 0);
            for(size_type i = 0; i < m_check_block.size(); ++i){
                if(m_v->m_full[i]){
                    if(m_v->m_full_type[m_v->m_rank_full(i+1)-1]){
                        m_check_block[i] = 1;
                    }
                }else{
                    m_check_block[i] = 1;
                }
            }
            sdsl::util::init_support(m_next_check, &m_check_block);
            sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
        }

        inline size_type succ(size_type i) const{
            auto j = i /m_v->sample;
            auto p = m_v->m_rank_full(j+1);
            auto q = j+1-p;
            if(m_v->m_full[j]){
                if(m_v->m_full_type[p-1] == t_b) {
                    return i;
                }
            }else{
                auto next_in_mixed = m_succ_mixed((q-1)*m_v->sample + i %m_v->sample);
                if(next_in_mixed < q*m_v->sample) {
                    return next_in_mixed - (q - 1) * m_v->sample + j * m_v->sample;
                }
            }
            j = m_next_check(j+1);
            //std::cout << "next block1: " << j << " (" << m_check_block.size() << ") " << std::endl;
            if(j >= m_check_block.size()) return m_v->size();
            if(m_v->m_full[j]){
                return j * m_v->sample;
            }else{
                size_type succ_mixed = m_succ_mixed(q*m_v->sample);
                if(succ_mixed == m_v->mixed.size()) return m_v->size();
                return j * m_v->sample + succ_mixed - q*m_v->sample;
            }
        };

        size_type operator()(size_type i)const
        {
            return succ(i);
        }

        size_type size()const
        {
            return m_v->size();
        }

        void set_vector(const zombit_vector* v=nullptr)
        {
            m_v = v;
            if(v != nullptr){
                sdsl::util::init_support(m_succ_mixed, &(m_v->m_mixed));
            }
        }

        succ_support_zombit_naive& operator=(const succ_support_zombit_naive& ss)
        {
            if (this != &ss) {
                copy(ss);
            }
            return *this;
        }

        succ_support_zombit_naive& operator=(succ_support_zombit_naive&& ss)
        {
            if (this != &ss) {
                m_v = std::move(ss.m_v);
                m_check_block = std::move(ss.m_check_block);
                m_next_check = std::move(ss.m_next_check);
                m_next_check.set_vector(&m_check_block);
                m_succ_mixed == std::move(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                }else{
                    m_succ_mixed.set_vector(nullptr);
                }
            }
            return *this;
        }

        void swap(succ_support_zombit_naive& ss) {
            if (this != &ss) {
                std::swap(m_v, ss.m_v);
                m_check_block.swap(ss.m_check_block);
                sdsl::util::swap_support(m_next_check, ss.m_next_check, &m_check_block, &ss.m_check_block);
                m_succ_mixed.swap(ss.m_succ_mixed);
                if(m_v != nullptr){
                    m_succ_mixed.set_vector(&(m_v->m_mixed));
                }else{
                    m_succ_mixed.set_vector(nullptr);
                }
                if(ss.m_v != nullptr){
                    ss.m_succ_mixed.set_vector(&(ss.m_v->m_mixed));
                }else{
                    ss.m_succ_mixed.set_vector(nullptr);
                }
            }
        }

        void load(std::istream& in, const succ_support_zombit_naive* v=nullptr)
        {
            m_v = v;
            m_check_block.load(in);
            m_next_check.load(in, &m_check_block);
            if(m_v != nullptr){
                m_succ_mixed.load(in, &(m_v->m_mixed));
            }
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_check_block.serialize(out, child, "check_block");
            written_bytes += m_next_check.serialize(out, child, "m_next_check");
            written_bytes += m_succ_mixed.serialize(out, child, "succ_mixed");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }


    };

}

#endif //RCT_ZOMBIT_VECTOR_HPP
