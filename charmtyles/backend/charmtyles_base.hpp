#pragma once

#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
#include <KokkosBlas2_gemv.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>
//make all the hapi dependencies cuda only
#ifdef KOKKOS_ENABLE_CUDA
#include "hapi.h"
#endif

class CProxy_vector_impl;
class CProxy_matrix_impl;
class CProxy_scalar_impl;
class CProxy_get_partial_vec_future;
class CProxy_KokkosGroup;
class CProxy_reductionGroup;

#include <charmtyles/util/sizes.hpp>
#include <charmtyles/backend/libcharmtyles.decl.h>

using ExecSpace = Kokkos::DefaultExecutionSpace;
using RangePolicy = Kokkos::RangePolicy<ExecSpace>;
using MDRangePolicy = Kokkos::MDRangePolicy<Kokkos::Rank<2>, ExecSpace>;
#ifdef GPU_BACKEND
using HostPinnedSpace = Kokkos::CudaHostPinnedSpace;
#else
using HostPinnedSpace = Kokkos::HostSpace;
#endif


class KokkosGroup : public CBase_KokkosGroup
{
private:
    std::map<uint64_t, void*> kernelHandles;
    std::string to_string(uint64_t const hash)
    {
        std::ostringstream oss;
        oss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return oss.str();
    }

public:
    KokkosGroup()
    {
        Kokkos::initialize();
        #ifdef GPU_BACKEND
        hapiCheck(cudaSetDevice(CkMyPe()));//later make RR on gpus
        auto start = CkTimer();
        hapiCreateStreams();
        ckout << "Time to create streams " <<CkTimer() - start << endl;
        #endif
    }

    void finalize()
    {
        // Kokkos::finalize();
    }

    void dkload(uint64_t hash)
    {
        if (kernelHandles.find(hash) != kernelHandles.end())
            return;
        std::string lib_name("libkernel-" + to_string(hash) + ".so");
        void* handle = dlopen(std::string("./" + lib_name).c_str(), RTLD_NOW);
        void* functor = dlsym(handle, "run_kernel");
        kernelHandles[hash] = functor;
    }

    void* getHandle(uint64_t hash)
    {
        return kernelHandles[hash];
    }
};

CProxy_KokkosGroup kokkosMgmt;

struct reductionMgmtPayload {
    int process;
    int sdag_indx;
    CProxyElement_matrix_impl proxy;
};

class reductionGroup : public CBase_reductionGroup {
private:
    int num_active_chares;
    std::vector<int> sdag_indexes;
    std::vector<CProxyElement_matrix_impl> chunkProxies;

    int resultSize;
    int contributeCnt;
    int resultIndex;
    // std::vector<double> localArr;
    Kokkos::View<double*> localArr;
    CProxy_vector_impl result_proxy;

    ExecSpace exec_space;

    int resultCnt;
    // std::vector<double> resultArr;
    Kokkos::View<double*> resultArr;
    std::vector<Kokkos::View<double*>> rootProcBuffers;
    Kokkos::View<double*> dummyArr;
    Kokkos::View<double*, Kokkos::HostSpace> resultArr_h;// don't clear between iteration
public:
    reductionGroup() {
        num_active_chares = 0;
        chunkProxies.reserve(5);
        sdag_indexes.reserve(5);

        contributeCnt = 0;
        resultSize = 0;
        resultIndex = -1;

        resultCnt = 0;

        auto stream = hapiGetStream();
        exec_space = ExecSpace(stream);
        dummyArr = Kokkos::View<double*> (Kokkos::view_alloc(exec_space, "dummpPtr"),1);
    }

    void accumulate(CProxy_vector_impl _result_proxy, int result_size, int result_index, int indx, int len, Kokkos::View<double*> data, ExecSpace exec_space) {
        contributeCnt++;
        // std::ostringstream os;
        // os << "accumulate (" << std::to_string(CkMyPe()) << ")";
        // NVTXTracer(os.str(), NVTXColor::WetAsphalt);

        ckout<<"resultSize "<<resultSize<<endl;

        resultSize = result_size;
        result_proxy = _result_proxy;
        resultIndex = result_index;

        if (localArr.size() != result_size) {
            localArr = Kokkos::View<double*> (Kokkos::view_alloc(exec_space, "local_arr"), result_size);
            Kokkos::deep_copy(exec_space, localArr, 0.0);
        }


        auto localArr = this->localArr;

        Kokkos::parallel_for("accumulate_local_contibutions", RangePolicy(exec_space, 0, len), 
        KOKKOS_LAMBDA(int i){
            localArr[indx + i] += data[i];
        });
        ckout<<"contributeCnt "<<contributeCnt<<"num_active_chares "<<num_active_chares<<endl;
        if (contributeCnt == num_active_chares) 
            {
                ckout<<"localArr size "<<localArr.size()<<endl;
                thisProxy[0].reduce(false, resultSize, CkDeviceBuffer(localArr.data(), exec_space.cuda_stream()));}
        
    }

    void reduce(bool dummy, int len, double*& data, CkDeviceBufferPost* postInfo){
        if(!dummy){
            Kokkos::View<double*> buffer(Kokkos::view_alloc(exec_space, "rootProcBuffers"), len);
            rootProcBuffers.push_back(buffer);
            data = buffer.data();
        } else {
            data = dummyArr.data();
        }
        postInfo[0].hapi_stream = exec_space.cuda_stream();
    }

    void reduce(bool dummy, int len, double* data) {
        resultCnt++;
        ckout<<"in reduce "<<endl;
        ckout<<"resultCnt "<<resultCnt<<endl;
        if (!dummy) {
            if (resultArr.size() != len) {
                Kokkos::resize(exec_space, resultArr, len);
                Kokkos::deep_copy(exec_space, resultArr, 0.0);
            }
        }

        if(resultCnt == CkNumNodes()) {
            Kokkos::View<Kokkos::View<double*>*> rootProcBuffers_d(Kokkos::view_alloc(exec_space, "mew mew"), rootProcBuffers.size());
            auto rootProcBuffers_h = Kokkos::create_mirror_view(rootProcBuffers_d);

            //debug
            std::size_t vec_len = CT_ACCESS_SINGLETON(ct::util::array_block_len);
            for(int i=0;i<rootProcBuffers.size();i++){
                auto rocProfBuffer_h = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(),rootProcBuffers[i]);
                for(int i=0;i<5;i++)
                    ckout<<rocProfBuffer_h(i)<<" ";
                ckout<<endl;
                // for(int i=vec_len;i<vec_len+5;i++)
                //     ckout<<rocProfBuffer_h(i)<<" ";
                // ckout<<endl;
            }
            // debug
            for(int i=0;i<rootProcBuffers.size();i++){
                rootProcBuffers_h(i) = rootProcBuffers[i];
            }
            Kokkos::deep_copy(exec_space, rootProcBuffers_d, rootProcBuffers_h);
            auto resultArr = this->resultArr;
            Kokkos::parallel_for("rootReduce",
            RangePolicy(exec_space, 0, len),
        KOKKOS_LAMBDA(int i){
            for(int j=0;j<rootProcBuffers_d.size();j++){
                resultArr[i]+=rootProcBuffers_d[j][i];
            }
             });
             exec_space.fence();
             auto resultArr_h = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), resultArr);
            
            for(int i = 0; ;i++) {
                if(i * vec_len >= resultSize) break;
                int length = (((i + 1) * vec_len) >= resultSize) ? (resultSize - (i * vec_len)) : vec_len;
                CkCallback cb(CkIndex_reductionGroup::reset(), thisProxy);
                result_proxy[i].update_vector(resultIndex, length, CkDeviceBuffer(resultArr.data() + i * vec_len, cb, exec_space.cuda_stream()));
            }
            // reset();
        }
    }

    void numActiveChares(CkReductionMsg *msg) {
        CkReduction::setElement* current = (CkReduction::setElement*) msg->getData();
        while (current != NULL)
        {
            reductionMgmtPayload result = *(reductionMgmtPayload*)(&current->data);
            int process = result.process;
            if (process == thisIndex) {
                num_active_chares++;
                sdag_indexes.emplace_back(result.sdag_indx);
                chunkProxies.emplace_back(result.proxy);
            }
            current = current->next();
        }
        
        for(int i = 0; i < num_active_chares; i++)
        {
            chunkProxies[i].active_chares_set(sdag_indexes[i]);
        }

        if (num_active_chares == 0)
            thisProxy[0].reduce(true, 1, CkDeviceBuffer(dummyArr.data(), exec_space.cuda_stream()));
    }

    void reset() {
        num_active_chares = 0;
        chunkProxies.clear();
        sdag_indexes.clear();

        contributeCnt = 0;
        resultSize = 0;
        resultIndex = -1;
        Kokkos::deep_copy(exec_space, localArr, 0);

        resultCnt = 0;
        Kokkos::deep_copy(exec_space, resultArr,0.0);

        rootProcBuffers.clear();
    }
};

CProxy_reductionGroup reductionMgmt;

#include "codegen.hpp"

/* readonly */ CProxy_scalar_impl scalar_impl_proxy;

class set_future : public CBase_set_future
{
public:
    set_future(ck::future<bool> is_done_, int count)
      : is_done(is_done_)
      , total(count)
      , counter(0)
    {
    }

    void pup(PUP::er& p)
    {
        p | is_done;
        p | total;
        p | counter;
    }

    void mark_complete()
    {
        if (++counter == total)
        {
            is_done.set(true);
            counter = 0;
        }
    }

    void evaluate_bool(bool result)
    {
        is_done.set(result);
    }

private:
    ck::future<bool> is_done;
    int total;
    int counter;
};

class get_vec_future : public CBase_get_vec_future
{
public:
    get_vec_future(ck::future<std::vector<double>> output_, size_t len_)
      : output(output_)
      , len(len_)
    {
    }

    void pup(PUP::er& p)
    {
        p | output;
        p | len;
    }

    void construct_vector(CkReductionMsg* msg)
    {
        std::vector<double> out(len, 0.);
        CkReduction::setElement* current =
            (CkReduction::setElement*) msg->getData();
        while (current != NULL)
        {
            double* result = (double*) &current->data;
            size_t index = (size_t) result[0];
            size_t len = current->dataSize / sizeof(double);
            for (size_t i = 1; i < len; i++)
            {
                out[index + i - 1] = result[i];
            }
            current = current->next();
        }
        output.set(out);
    }

private:
    ck::future<std::vector<double>> output;
    size_t len;
};

class get_mat_future : public CBase_get_mat_future
{
public:
    get_mat_future(ck::future<std::vector<std::vector<double>>> output_,
        size_t rows_, size_t cols_)
      : output(output_)
      , rows(rows_)
      , cols(cols_)
    {
    }

    void pup(PUP::er& p)
    {
        p | output;
        p | rows;
        p | cols;
    }

    void construct_matrix(CkReductionMsg* msg)
    {
        std::vector<std::vector<double>> out(
            rows, std::vector<double>(cols, 0.0));
        CkReduction::setElement* current =
            (CkReduction::setElement*) msg->getData();
        while (current != NULL)
        {
            double* result = (double*) &current->data;
            size_t row_index = (size_t) result[0];
            size_t col_index = (size_t) result[1];
            size_t row_size = (size_t) result[2];
            size_t col_size = (current->dataSize - (3 * sizeof(double))) /
                (sizeof(double) * row_size);
            for (size_t i = 0; i < row_size; i++)
            {
                for (size_t j = 0; j < col_size; j++)
                    out[row_index + i][col_index + j] =
                        result[3 + i * col_size + j];
            }
            current = current->next();
        }
        output.set(out);
    }

private:
    ck::future<std::vector<std::vector<double>>> output;
    size_t rows;
    size_t cols;
};

class get_partial_vec_future : public CBase_get_partial_vec_future
{
public:
    get_partial_vec_future(ck::future<std::vector<double>> output_, size_t k_)
      : output(output_)
      , k(k_)
    {
    }

    void pup(PUP::er& p)
    {
        p | output;
        p | k;
    }

    void construct_partial_vector(CkReductionMsg* msg)
    {
        std::vector<double> out(k, 0.0);

        CkReduction::setElement* current =
            (CkReduction::setElement*) msg->getData();
        while (current != NULL)
        {
            double* result = (double*) &current->data;
            size_t len = current->dataSize / sizeof(double);

            if (len > 0)
            {
                int count = (int) result[0];
                // [count, sample_index1, value1, sample_index2, value2, ...]
                for (int i = 0; i < count && (1 + 2 * i + 1) < len; i++)
                {
                    size_t sample_index = (size_t) result[1 + 2 * i];
                    double value = result[1 + 2 * i + 1];

                    if (sample_index < k)
                    {
                        out[sample_index] = value;
                    }
                }
            }
            current = current->next();
        }

        output.set(out);
    }

private:
    ck::future<std::vector<double>> output;
    size_t k;
};

class scalar_impl : public CBase_scalar_impl
{
public:
    scalar_impl_SDAG_CODE;

    scalar_impl()
      : SDAG_INDEX(0)
    {
        thisProxy.main_kernel();
    }

    std::vector<double> scal_map;

    int SDAG_INDEX;
};

#define CHECK_IF_EXIST_ELSE_ADD_VECTOR(node)                                   \
    if (node.name_ == vec_map.size())                                          \
    {                                                                          \
        std::size_t vec_dim = get_vec_dim(node.vec_len_);                      \
                                                                               \
        Kokkos::View<double*> vec(                                             \
            Kokkos::view_alloc("vec" + std::to_string(node.name_), exec_space), vec_dim);\
        vec_map.emplace_back(vec);                                             \
    }

class vector_impl : public CBase_vector_impl
{
public:
    void update_partitions(
        std::vector<std::vector<ct::vec_impl::vec_node>> const& instr_list)
    {
        for (size_t i = 0; i < instr_list.size();)
        {
            std::vector<std::vector<ct::vec_impl::vec_node>> region =
                ct::util::carveRegion<ct::vec_impl::vec_node>(instr_list, i);

            if (!region.empty())
            {
                for (const auto& instr : region)
                    CHECK_IF_EXIST_ELSE_ADD_VECTOR(instr[0]);
                execute_instruction(region);
                i += region.size();
            }
            else
            {
                execute_instruction({instr_list[i]});
                ++i;
            }
        }
    }

    // Helper method for generator initialization - must be public for CUDA lamdas
    // TODO: does not work for cuda as of now(gen_ptr is cpu ptr)
    Kokkos::View<double*> generator_init_impl(
        std::size_t vec_dim, std::shared_ptr<ct::generator> gen_ptr)
    {
        Kokkos::View<double*> gen_vec("Label", vec_dim);

        for (int dimX = 0; dimX != vec_dim; ++dimX)
        {
            gen_vec(dimX) =
                gen_ptr->generate(thisIndex * vec_block_size + dimX);
        }

        return gen_vec;
    }

    // Helper method for vector dot product - must be public for CUDA lambdas
    double vector_dot_impl(int lhs_id, int rhs_id)
    {
        auto lhs = vec_map[lhs_id];
        auto rhs = vec_map[rhs_id];

        double result = 0.0;
        Kokkos::parallel_reduce(
            RangePolicy(exec_space, 0,lhs.size()),
            KOKKOS_LAMBDA(const int i, double& local_sum) {
                local_sum += lhs(i) * rhs(i);
            },
            result);

        return result;
    }

    void execute_instruction(
        std::vector<std::vector<ct::vec_impl::vec_node>> const& region)
    {
        const std::vector<ct::vec_impl::vec_node>& instruction = region[0];
        const ct::vec_impl::vec_node& node = instruction[0];
        std::size_t node_id = node.name_;

        switch (node.operation_)
        {
        case ct::util::Operation::init_random:
        {
            CkAssert((vec_map.size() == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            std::size_t vec_dim = get_vec_dim(node.vec_len_);
            Kokkos::View<double*> vec(Kokkos::view_alloc("vec" + std::to_string(node_id), exec_space), vec_dim);
            
            unsigned int seed =
                static_cast<unsigned int>(time(nullptr)) + node_id;
            Kokkos::Random_XorShift64_Pool<> rand_pool(exec_space, seed);

            Kokkos::parallel_for(
                "init_random_" + std::to_string(node_id),
                RangePolicy(exec_space, 0, vec.size()), KOKKOS_LAMBDA(int i) {
                    auto gen = rand_pool.get_state();
                    double r = gen.drand();
                    vec(i) = r;
                    rand_pool.free_state(gen);
                });
            vec_map.emplace_back(vec);
        }
            return;
        case ct::util::Operation::init_value:
        {
            CkAssert((vec_map.size() == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            std::size_t vec_dim = get_vec_dim(node.vec_len_);

            Kokkos::View<double*> vec(Kokkos::view_alloc("vec" + std::to_string(node_id), exec_space), vec_dim);
            Kokkos::deep_copy(exec_space, vec, node.value_);
            vec_map.emplace_back(vec);
        }
            return;
        case ct::util::Operation::copy:
        {
            std::size_t copy_id = node.copy_id_;

            if (node_id == vec_map.size())
                vec_map.emplace_back(
                    Kokkos::View<double*>(Kokkos::view_alloc("FIXME", exec_space), vec_map[copy_id].size()));

            auto dest = vec_map[node_id];
            auto src = vec_map[copy_id];

            Kokkos::parallel_for(
                "copy_" + std::to_string(copy_id) + "_" +
                    std::to_string(node_id),
                    RangePolicy(exec_space, 0, dest.size()), KOKKOS_LAMBDA(int i) { dest(i) = src(i); });
        }
            return;
        case ct::util::Operation::add:
        case ct::util::Operation::sub:
        case ct::util::Operation::multiply:
        case ct::util::Operation::divide:
        case ct::util::Operation::geq:
        case ct::util::Operation::leq:
        case ct::util::Operation::greater:
        case ct::util::Operation::lesser:
        case ct::util::Operation::eq:
        case ct::util::Operation::neq:
        case ct::util::Operation::logical_and:
        case ct::util::Operation::logical_or:
        case ct::util::Operation::logical_not:
        case ct::util::Operation::unary_expr:
        case ct::util::Operation::binary_expr:
        case ct::util::Operation::where:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);
            Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node ,1>(
                exec_space,
                instruction[0].kernel, {vec_map[node_id].size()}, vec_map,
                region);
        }
            return;
        case ct::util::Operation::inplace_add:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node,
                    1>(exec_space, node.kernel, {vec_map[node_id].size()}, vec_map, region);
            }
            else
            {
                auto dest = vec_map[node_id];
                auto src = vec_map[copy_id];

                Kokkos::parallel_for(
                    "copy_" + std::to_string(copy_id) + "_" +
                        std::to_string(node_id),
                    RangePolicy(exec_space, 0, dest.size()), KOKKOS_LAMBDA(int i) { dest(i) += src(i); });
            }
        }
            return;
        case ct::util::Operation::inplace_sub:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node,
                    1>(exec_space, node.kernel, {vec_map[node_id].size()}, vec_map, region);
            }
            else
            {
                auto dest = vec_map[node_id];
                auto src = vec_map[copy_id];

                Kokkos::parallel_for(
                    "copy_" + std::to_string(copy_id) + "_" +
                        std::to_string(node_id),
                    RangePolicy(exec_space, 0, dest.size()), KOKKOS_LAMBDA(int i) { dest(i) -= src(i); });
            }
        }
            return;
        case ct::util::Operation::inplace_divide:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double*>, ct::vec_impl::vec_node,
                    1>(exec_space, node.kernel, {vec_map[node_id].size()}, vec_map, region);
            }
            else
            {
                auto dest = vec_map[node_id];
                auto src = vec_map[copy_id];

                Kokkos::parallel_for(
                    "copy_" + std::to_string(copy_id) + "_" +
                        std::to_string(node_id),
                    RangePolicy(exec_space, 0, dest.size()), KOKKOS_LAMBDA(int i) { dest(i) /= src(i); });
            }
        }
            return;
        case ct::util::Operation::dealloc: {
            /**
             * TODO: This might fail due to a double free depending on how charm runtime 
             *       destroys the charmArrays. 
             */
            Kokkos::View<double*> releivingRef;
            vec_map[node_id] = releivingRef;
        } return;
        case ct::util::Operation::custom_expr:
        {
            CHECK_IF_EXIST_ELSE_ADD_VECTOR(node);

            const ct::vec_impl::vec_node& node = instruction[0];
            auto a_host = Kokkos::create_mirror_view_and_copy(
                Kokkos::HostSpace(), vec_map[node_id]);
            const std::size_t n = a_host.extent(0);
            auto a = std::vector<double>(a_host.data(), a_host.data() + n);

            auto b_host = Kokkos::create_mirror_view_and_copy(
                Kokkos::HostSpace(), vec_map[instruction[node.left_].name_]);
            auto b = std::vector<double>(b_host.data(), b_host.data() + n);

            node.custom_expr_->operator()(n, a, b);

            Kokkos::View<double*> a_new("vec" + std::to_string(node_id), n);
            Kokkos::View<double*> b_new(
                "vec" + std::to_string(instruction[node.left_].name_), n);

            auto a_new_host = Kokkos::create_mirror_view(a_new);
            auto b_new_host = Kokkos::create_mirror_view(b_new);

            for (auto i = 0; i < n; i++)
            {
                a_new_host(i) = a[i];
                b_new_host(i) = b[i];
            }
            Kokkos::deep_copy(a_new, a_new_host);
            Kokkos::deep_copy(b_new, b_new_host);
            vec_map[node_id] = a_new;
            vec_map[instruction[node.left_].name_] = b_new;
        }
            return;
        default:
            CmiAbort("Operation not implemented");
        }
    }

public:
    vector_impl_SDAG_CODE;

    void update_vector(int vec_idx, int len, double*& data, CkDeviceBufferPost* postInfo){
        if (vec_idx == vec_map.size())
            vec_map.emplace_back(Kokkos::View<double*>(Kokkos::view_alloc("FIXME_2", exec_space), len));
        data = vec_map[vec_idx].data();
        postInfo[0].hapi_stream = exec_space.cuda_stream();
    }

    vector_impl(int num_chares_, int vec_block_size_)
      : num_chares(num_chares_)
      , SDAG_INDEX(0)
      , vec_block_size(vec_block_size_)
    {
        vec_map.reserve(1000);

        #ifdef GPU_BACKEND
        auto stream = hapiGetStream();
        exec_space = ExecSpace(stream);
        stream = hapiGetStream();
        comm_space = ExecSpace(stream);
        #else
        exec_space = ExecSpace();
        #endif

        thisProxy[thisIndex].main_kernel();
    }

private:
    std::size_t get_vec_dim(std::size_t vec_len)
    {
        if (vec_len % vec_block_size == 0)
            return vec_block_size;

        if (thisIndex != num_chares - 1)
            return vec_block_size;

        return vec_len % vec_block_size;
    }

    void print_instructions(
        std::vector<std::vector<ct::vec_impl::vec_node>> const& instr_list)
    {
        ckout << "Printing Instructions:" << endl;

        for (int num_instr = 0; num_instr != instr_list.size(); ++num_instr)
        {
            ckout << "Instruction " << num_instr << ": ";
            ct::util::parse_ast(instr_list[num_instr], 0);
            ckout << endl;
        }
    }

    int num_chares;
    std::vector<Kokkos::View<double*>> vec_map;

    int SDAG_INDEX;
    int vec_block_size;
    ExecSpace exec_space;
    ExecSpace comm_space;
    
    // context for async callback of send_to_matrix
    struct send_to_matrix_ctx_t {
        Kokkos::View<double*, HostPinnedSpace> host_cpy;
        CProxy_matrix_impl proxy;
        int rhs_sdag_idx;
        int vec_idx;
        int row_block_len;
        int col_block_len;
        int numCharesX;
        int numCharesY;
        bool is_vec_mat;
    } send_to_matrix_context;
};

#define CHECK_IF_EXIST_ELSE_ADD_MATRIX(node)                                   \
    if (node.name_ == mat_map.size())                                          \
    {                                                                          \
        std::size_t num_rows = get_mat_rows(node.mat_row_len_);                \
        std::size_t num_cols = get_mat_cols(node.mat_col_len_);                \
                                                                               \
        Kokkos::View<double**> mat(                                            \
            Kokkos::view_alloc("mat" + std::to_string(node.name_), exec_space), num_rows, num_cols);           \
        mat_map.emplace_back(mat);                                             \
    }

class matrix_impl : public CBase_matrix_impl
{
    // Helper private functions
private:
    std::size_t get_mat_rows(std::size_t row_len)
    {
        if (row_len % row_block_len == 0)
            return row_block_len;

        if (thisIndex.y != num_chares_y - 1)
            return row_block_len;

        return row_len % row_block_len;
    }

    std::size_t get_mat_cols(std::size_t col_len)
    {
        if (col_len % col_block_len == 0)
            return col_block_len;

        if (thisIndex.x != num_chares_x - 1)
            return col_block_len;

        return col_len % col_block_len;
    }

    void print_instructions(
        std::vector<std::vector<ct::mat_impl::mat_node>> const& instr_list)
    {
        ckout << "Printing Instructions:" << endl;

        for (int num_instr = 0; num_instr != instr_list.size(); ++num_instr)
        {
            ckout << "Instruction " << num_instr << ": ";
            ct::util::parse_ast(instr_list[num_instr], 0);
            ckout << endl;
        }
    }

    // Instruction related functions - must be public for CUDA lambdas
public:
    void update_partitions(
        std::vector<std::vector<ct::mat_impl::mat_node>> const& instr_list)
    {
        for (size_t i = 0; i < instr_list.size();)
        {
            std::vector<std::vector<ct::mat_impl::mat_node>> region =
                ct::util::carveRegion<ct::mat_impl::mat_node>(instr_list, i);

            if (!region.empty())
            {
                for (const auto& instr : region)
                    CHECK_IF_EXIST_ELSE_ADD_MATRIX(instr[0]);
                execute_instruction(region);
                i += region.size();
            }
            else
            {
                execute_instruction({instr_list[i]});
                ++i;
            }
        }
    }

    void mat_vec_dot_impl(int mat_idx,
        std::size_t vec_len, const double* vec_in_data , CkCallback* cb)
    {
        Kokkos::View<double**> mat = mat_map[mat_idx];
        std::size_t num_rows = mat.extent(0);
        std::size_t num_cols = mat.extent(1);

        
        // if(mat_vec_dot_context.vec_in_h.size() != num_cols)
        //     mat_vec_dot_context.vec_in_h = Kokkos::View<double*, HostPinnedSpace>("vec_in_host", num_cols);
    
        // for(int i=0; i<num_cols; ++i)
        //     mat_vec_dot_context.vec_in_h(i) = vec_in_data[i];

        // #ifdef GPU_BACKEND
        // if(mat_vec_dot_context.vec_in.size() != num_cols)
        //     mat_vec_dot_context.vec_in = Kokkos::View<double*>(Kokkos::view_alloc("vec_in_tile", exec_space), num_cols);

        // Kokkos::deep_copy(exec_space, mat_vec_dot_context.vec_in, mat_vec_dot_context.vec_in_h);
        // #else
        // mat_vec_dot_context.vec_in = mat_vec_dot_context.vec_in_h;
        // #endif

        // if (mat_vec_dot_context.local_result_h.size() != num_rows)
        //     mat_vec_dot_context.local_result_h = Kokkos::View<double*, HostPinnedSpace>("local_result_host", num_rows);


        #ifdef GPU_BACKEND
        if (mat_vec_dot_context.local_result.size() != num_rows)
            mat_vec_dot_context.local_result = Kokkos::View<double*>(Kokkos::view_alloc("local_result", exec_space), num_rows);
        #else
        mat_vec_dot_context.local_result = mat_vec_dot_context.local_result_h;
        #endif
        
        KokkosBlas::gemv(exec_space, "N",1.0,mat, mat_vec_dot_context.vec_in,0.0,mat_vec_dot_context.local_result);

        #ifdef GPU_BACKEND
        // Kokkos::deep_copy(exec_space, mat_vec_dot_context.local_result_h, mat_vec_dot_context.local_result);
        hapiAddCallback(exec_space.cuda_stream(), (void*)cb);
        #else
        ((CkCallback *)cb)->send();
        #endif
    }

    // Helper method for vector-matrix multiplication - must be public for CUDA lambdas
    void vec_mat_dot_impl(int mat_idx, const double* vec_in_data, Kokkos::View<double*>& local_result)
    {
        Kokkos::View<double**> mat = mat_map[mat_idx];
        std::size_t num_rows = mat.extent(0);
        std::size_t num_cols = mat.extent(1);

        using HostConstVector = Kokkos::View<const double*, Kokkos::HostSpace,
            Kokkos::MemoryTraits<Kokkos::Unmanaged>>;
        HostConstVector vec_in_host(vec_in_data, num_rows);

        using DeviceVector = Kokkos::View<double*,
            typename Kokkos::DefaultExecutionSpace::memory_space>;
        DeviceVector vec_in("vec_in_tile", num_rows);
        Kokkos::deep_copy(vec_in, vec_in_host);

        // Perform vector-matrix multiplication: result = vec * mat
        KokkosBlas::gemv("T",1.0,mat,vec_in,0.0,local_result);
        Kokkos::fence();
    }

    void execute_instruction(
        std::vector<std::vector<ct::mat_impl::mat_node>> const& region)
    {
        const std::vector<ct::mat_impl::mat_node>& instruction = region[0];
        ct::mat_impl::mat_node const& node = instruction[0];
        std::size_t node_id = node.name_;

        switch (node.operation_)
        {
        case ct::util::Operation::init_random:
        {
            CkAssert((mat_map.size() == node_id) &&
                "A matrix is initialized before a dependent matrix "
                "initialization.");

            std::size_t num_rows = get_mat_rows(node.mat_row_len_);
            std::size_t num_cols = get_mat_cols(node.mat_col_len_);

            Kokkos::View<double**> mat(
                Kokkos::view_alloc("mat" + std::to_string(node_id), exec_space), num_rows, num_cols);
            unsigned int seed =
                static_cast<unsigned int>(time(nullptr)) + node_id;
            Kokkos::Random_XorShift64_Pool<> rand_pool(exec_space, seed);

            Kokkos::parallel_for(
                "init_random_mat_" + std::to_string(node_id),
                MDRangePolicy(exec_space,
                    {0, 0}, {num_rows, num_cols}),
                KOKKOS_LAMBDA(int i, int j) {
                    auto gen = rand_pool.get_state();
                    double r = gen.drand();
                    mat(i, j) = r;
                    rand_pool.free_state(gen);
                });
            mat_map.emplace_back(mat);
        }
            return;
        case ct::util::Operation::init_value:
        {
            CkAssert((mat_map.size() == node_id) &&
                "A matrix is initialized before a dependent matrix "
                "initialization.");

            std::size_t num_rows = get_mat_rows(node.mat_row_len_);
            std::size_t num_cols = get_mat_cols(node.mat_col_len_);

            Kokkos::View<double**> mat(
                Kokkos::view_alloc("mat" + std::to_string(node_id), exec_space), num_rows, num_cols);
            Kokkos::deep_copy(exec_space, mat, node.value_);
            mat_map.emplace_back(mat);
        }
            return;
        case ct::util::Operation::copy:
        {
            std::size_t copy_id = node.copy_id_;
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            auto dest = mat_map[node_id];
            auto src = mat_map[copy_id];

            Kokkos::deep_copy(exec_space, dest, src);

            // Kokkos::parallel_for(
            //     "copy_mat_" + std::to_string(copy_id) + "_" +
            //         std::to_string(node_id),
            //     Kokkos::MDRangePolicy<Kokkos::Rank<2>>(
            //         {0, 0}, {dest.extent(0), dest.extent(1)}),
            //     KOKKOS_LAMBDA(int i, int j) { dest(i, j) = src(i, j); });
        }
            return;
        case ct::util::Operation::add:
        case ct::util::Operation::sub:
        case ct::util::Operation::multiply:
        case ct::util::Operation::divide:
        case ct::util::Operation::geq:
        case ct::util::Operation::leq:
        case ct::util::Operation::greater:
        case ct::util::Operation::lesser:
        case ct::util::Operation::eq:
        case ct::util::Operation::neq:
        case ct::util::Operation::unary_expr:
        case ct::util::Operation::binary_expr:
        case ct::util::Operation::logical_and:
        case ct::util::Operation::logical_or:
        case ct::util::Operation::logical_not:
        case ct::util::Operation::where:
        {
            // exec_space.fence();
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node, 2>(
                exec_space,
                instruction[0].kernel,
                {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                mat_map, region);
        }
            return;
        case ct::util::Operation::inplace_add:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node,
                    2>(exec_space,
                    instruction[0].kernel,
                    {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                    mat_map, region);
            }
            else
            {
                auto dest = mat_map[node_id];
                auto src = mat_map[copy_id];

                Kokkos::parallel_for(
                    "inplace_add_mat_" + std::to_string(node_id),
                    MDRangePolicy(exec_space,
                        {0, 0}, {dest.extent(0), dest.extent(1)}),
                    KOKKOS_LAMBDA(int i, int j) { dest(i, j) += src(i, j); });
            }
        }
            return;
        case ct::util::Operation::inplace_sub:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node,
                    2>(exec_space, instruction[0].kernel,
                    {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                    mat_map, region);
            }
            else
            {
                auto dest = mat_map[node_id];
                auto src = mat_map[copy_id];

                Kokkos::parallel_for(
                    "inplace_add_mat_" + std::to_string(node_id),
                    MDRangePolicy(exec_space,
                        {0, 0}, {dest.extent(0), dest.extent(1)}),
                    KOKKOS_LAMBDA(int i, int j) { dest(i, j) -= src(i, j); });
            }
        }
            return;
        case ct::util::Operation::inplace_divide:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);
            std::size_t copy_id = node.copy_id_;
            if (copy_id == -1)
            {
                Codegen::execute<Kokkos::View<double**>, ct::mat_impl::mat_node,
                    2>(
                    exec_space,    
                    instruction[0].kernel,
                    {mat_map[node_id].extent(0), mat_map[node_id].extent(1)},
                    mat_map, region);
            }
            else
            {
                auto dest = mat_map[node_id];
                auto src = mat_map[copy_id];

                Kokkos::parallel_for(
                    "inplace_add_mat_" + std::to_string(node_id),
                    MDRangePolicy(exec_space,
                        {0, 0}, {dest.extent(0), dest.extent(1)}),
                    KOKKOS_LAMBDA(int i, int j) { dest(i, j) /= src(i, j); });
            }
        }
            return;
        case ct::util::Operation::dealloc: {
            /**
             * TODO: This might fail due to a double free depending on how charm runtime 
             *       destroys the charmArrays. 
             */
            Kokkos::View<double**> releivingRef;
            mat_map[node_id] = releivingRef;
        } return;
        case ct::util::Operation::custom_expr:
        {
            CHECK_IF_EXIST_ELSE_ADD_MATRIX(node);

            const ct::mat_impl::mat_node& node = instruction[0];
            auto a_host = Kokkos::create_mirror_view_and_copy(
                Kokkos::HostSpace(), mat_map[node_id]);
            const std::size_t rows = a_host.extent(0);
            const std::size_t cols = a_host.extent(1);

            std::vector<std::vector<double>> a;
            for (int i = 0; i < rows; i++)
            {
                a.push_back(std::vector<double>(
                    a_host.data() + i * cols, a_host.data() + i * cols + cols));
            }

            auto b_host = Kokkos::create_mirror_view_and_copy(
                Kokkos::HostSpace(), mat_map[instruction[node.left_].name_]);
            std::vector<std::vector<double>> b;
            for (int i = 0; i < rows; i++)
            {
                b.push_back(std::vector<double>(
                    b_host.data() + i * cols, b_host.data() + i * cols + cols));
            }

            node.custom_expr_->operator()(rows, cols, a, b);

            Kokkos::View<double**> a_new(
                "mat" + std::to_string(node_id), rows, cols);
            Kokkos::View<double**> b_new(
                "mat" + std::to_string(instruction[node.left_].name_), rows,
                cols);

            auto a_new_host = Kokkos::create_mirror_view(a_new);
            auto b_new_host = Kokkos::create_mirror_view(b_new);

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    a_new_host(i, j) = a[i][j];
                    b_new_host(i, j) = b[i][j];
                }
            }

            Kokkos::deep_copy(a_new, a_new_host);
            Kokkos::deep_copy(b_new, b_new_host);
            mat_map[node_id] = a_new;
            mat_map[instruction[node.left_].name_] = b_new;
        }
            return;
        default:
            CmiAbort("Operation not implemented");
        }
    }

public:
    matrix_impl_SDAG_CODE;

    void receive_to_matrix(int sdag_indx, int &len, double*& data, CkDeviceBufferPost* postInfo){
        if(mat_vec_dot_context.vec_in.size()!=len)
            mat_vec_dot_context.vec_in = Kokkos::View<double*>(Kokkos::view_alloc("vec_in_tile", exec_space), len);
        exec_space.fence();
        data = mat_vec_dot_context.vec_in.data();
        postInfo[0].hapi_stream = exec_space.cuda_stream();
    }

    matrix_impl(int num_chares_y_, int num_chares_x_, int row_block_len_,
        int col_block_len_)
      : num_chares_y(num_chares_y_)
      , num_chares_x(num_chares_x_)
      , row_block_len(row_block_len_)
      , col_block_len(col_block_len_)
      , SDAG_INDEX(0)
    {
        mat_map.reserve(1000);
        #ifdef GPU_BACKEND
        auto stream = hapiGetStream();
        exec_space = ExecSpace(stream);
        stream = hapiGetStream();
        comm_space = ExecSpace(stream);
        #else
        exec_space = ExecSpace();
        #endif
        thisProxy(thisIndex.x, thisIndex.y).main_kernel();
    }

private:
    std::vector<Kokkos::View<double**>> mat_map;

    int num_chares_y;
    int num_chares_x;

    int row_block_len;
    int col_block_len;
    int SDAG_INDEX;
    int block;
    ExecSpace exec_space;
    ExecSpace comm_space;

    struct mat_vec_dot_ctx_t {
        size_t result_size;
        size_t local_result_size;
        int result_index;
        Kokkos::View<double*> local_result;
        Kokkos::View<double*, HostPinnedSpace> local_result_h;
        Kokkos::View<double*> vec_in;
        Kokkos::View<double*, HostPinnedSpace> vec_in_h;
        CProxy_vector_impl result_proxy;
    } mat_vec_dot_context;
    
};
