#pragma once

#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
#include <ctime>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/generator.hpp>
#include <charmtyles/util/matrix_view.hpp>
#include <charmtyles/util/sizes.hpp>

class CProxy_vector_impl;
class CProxy_matrix_impl;
class CProxy_scalar_impl;
class CProxy_get_partial_vec_future;

#include <charmtyles/backend/libcharmtyles.decl.h>

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

private:
    std::vector<double> scal_map;

    int SDAG_INDEX;
};

class vector_impl : public CBase_vector_impl
{
    // Helper private functions
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

    // Instruction related private functions
private:
    void update_partitions(
        std::vector<std::vector<ct::vec_impl::vec_node>> const& instr_list)
    {
        for (auto const& ast : instr_list)
            execute_instruction(ast);
    }

#define CHECK_IF_EXIST_ELSE_ADD(node_id)                                       \
    if (node_id == vec_map.size())                                             \
    {                                                                          \
        vec_dim = get_vec_dim(node.vec_len_);                                  \
                                                                               \
        Kokkos::View<double*> vec("vec" + std::to_string(node_id), vec_dim);   \
        vec_map.emplace_back(vec);                                             \
    }

    void execute_instruction(
        std::vector<ct::vec_impl::vec_node> const& instruction,
        std::size_t index = 0)
    {
        codegen_prologue();
        ct::vec_impl::vec_node const& node = instruction[index];
        std::size_t node_id = node.name_;

        // Useful variables in switch statement
        std::size_t vec_dim{0};
        std::size_t total_size{0};
        std::size_t unrolled_size{0};
        std::size_t remainder_start{0};
        std::size_t copy_id{0};

        std::shared_ptr<ct::unary_operator> const& unary_expr =
            node.unary_expr_;

        std::shared_ptr<ct::binary_operator> const& binary_expr =
            node.binary_expr_;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(0., 1.);

        switch (instruction[index].operation_)
        {
        case ct::util::Operation::init_random:
        {
            CkAssert((vec_map.size() == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            vec_dim = get_vec_dim(node.vec_len_);
            Kokkos::View<double*> vec("vec" + std::to_string(node_id), vec_dim);
            vec_map.emplace_back(vec);
            unsigned int seed =
                static_cast<unsigned int>(time(nullptr)) + node_id;
            Kokkos::Random_XorShift64_Pool<> rand_pool(seed);

            Kokkos::parallel_for(
                "init_random_" + std::to_string(node_id),
                vec_map[node_id].size(), KOKKOS_LAMBDA(int i) {
                    auto gen = rand_pool.get_state();
                    double r = gen.drand();
                    vec_map[node_id](i) = r;
                    rand_pool.free_state(gen);
                });
        }
            return;
        case ct::util::Operation::init_value:
        {
            CkAssert((vec_map.size() == node_id) &&
                "A vector is initialized before a dependent vector "
                "initialization.");

            vec_dim = get_vec_dim(node.vec_len_);

            // TODO: Do Random Initialization here
            Kokkos::View<double*> vec("vec" + std::to_string(node_id), vec_dim);
            Kokkos::deep_copy(vec, node.value_);
            vec_map.emplace_back(vec);
        }
            return;
        case ct::util::Operation::copy:
        {
            copy_id = node.copy_id_;

            if (node_id == vec_map.size())
                vec_map.emplace_back(
                    Kokkos::View<double*>("FIXME", vec_map[copy_id].size()));

            Kokkos::parallel_for(
                "copy_" + std::to_string(copy_id) + "_" +
                    std::to_string(node_id),
                vec_map[node_id].size(), KOKKOS_LAMBDA(int i) {
                    vec_map[node_id](i) = vec_map[copy_id](i);
                });
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
        case ct::util::Operation::where: {
            CHECK_IF_EXIST_ELSE_ADD(node_id);

            long long resid = codegen_ast(instruction, 0);
            kk << "vec_map[" << node_id << "](i) = tmp" << -resid << ";\n";
        } break;
        case ct::util::Operation::inplace_add:
        {
            CHECK_IF_EXIST_ELSE_ADD(node_id);
            copy_id = node.copy_id_;
            if(copy_id == static_cast<std::size_t>(-1)) {
                long long resid = codegen_ast(instruction, 0);
                kk << "vec_map[" << node_id << "](i) += tmp" << -resid << ";\n";
            } else {
                Kokkos::parallel_for(
                    "copy_" + std::to_string(copy_id) + "_" +
                        std::to_string(node_id),
                    vec_map[node_id].size(), KOKKOS_LAMBDA(int i) {
                        vec_map[node_id](i) += vec_map[copy_id](i);
                    });
                return;
            }
        } break;
        case ct::util::Operation::inplace_sub:
        {
            CHECK_IF_EXIST_ELSE_ADD(node_id);
            copy_id = node.copy_id_;
            if(copy_id == static_cast<std::size_t>(-1)) {
                long long resid = codegen_ast(instruction, 0);
                kk << "vec_map[" << node_id << "](i) -= tmp" << -resid << ";\n";
            } else {
                Kokkos::parallel_for(
                    "copy_" + std::to_string(copy_id) + "_" +
                        std::to_string(node_id),
                    vec_map[node_id].size(), KOKKOS_LAMBDA(int i) {
                        vec_map[node_id](i) -= vec_map[copy_id](i);
                    });
                return;
            }
        } break;
        case ct::util::Operation::inplace_divide:
        {
            CHECK_IF_EXIST_ELSE_ADD(node_id);
            copy_id = node.copy_id_;
            if(copy_id == static_cast<std::size_t>(-1)) {
                long long resid = codegen_ast(instruction, 0);
                kk << "vec_map[" << node_id << "](i) /= tmp" << -resid << ";\n";
            } else {
                Kokkos::parallel_for(
                    "copy_" + std::to_string(copy_id) + "_" +
                        std::to_string(node_id),
                    vec_map[node_id].size(), KOKKOS_LAMBDA(int i) {
                        vec_map[node_id](i) /= vec_map[copy_id](i);
                    });
                return;
            }
        } break;
        case ct::util::Operation::axpy:
        {
            CHECK_IF_EXIST_ELSE_ADD(node_id);

            double alpha = node.value_;
            Kokkos::View<double*> x = vec_map[node.left_];
            Kokkos::View<double*> y = vec_map[node.right_];
            Kokkos::View<double*> res = vec_map[node.name_];

            Kokkos::parallel_for(
                "axpy", x.extent(0),
                KOKKOS_LAMBDA(const int i) { res(i) = alpha * x(i) + y(i); });

            return;
        }
        case ct::util::Operation::custom_expr:
        {
            CHECK_IF_EXIST_ELSE_ADD(node_id);

            const ct::vec_impl::vec_node& node = instruction[0];
            /**
             * TODO: The idea of custom operator does not make sense with a kokkos backend
             */
            // node.custom_expr_->operator()(vec_dim, vec_map[node_id].data(),
            //     vec_map[instruction[node.left_].name_].data());
            return;
        }

        default:
            CmiAbort("Operation not implemented");
        }
        codegen_epilogue(vec_map[node_id].size());
}

void codegen_prologue() {
    kk.str("");
    kkTmpVar = 0;
    kkCustomOpIdx = 0;
    kkCustomOps.clear();
}

std::string kernel_hash(const std::string &data) {
    const uint64_t FNV_OFFSET = 0xcbf29ce484222325ULL;
    const uint64_t FNV_PRIME  = 0x100000001b3ULL;
    uint64_t hash = FNV_OFFSET;
    for (unsigned char c : data) {
        hash ^= static_cast<uint64_t>(c);
        hash *= FNV_PRIME;
    }
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return oss.str();
}

void* getFuncPtr(ct::unary_operator* op) {
    void** vtable = *reinterpret_cast<void***>(op);
    void* fun_ptr = vtable[5];
    using RawFun = double(*)(double);
    RawFun rf = reinterpret_cast<RawFun>(fun_ptr);
    return reinterpret_cast<void*>(rf);
}

void* getFuncPtr(ct::binary_operator* op) {
    void** vtable = *reinterpret_cast<void***>(op);
    void* fun_ptr = vtable[5];
    using RawFun = double(*)(double, double);
    RawFun rf = reinterpret_cast<RawFun>(fun_ptr);
    return reinterpret_cast<void*>(rf);
}

void codegen_epilogue(std::size_t vec_dim) {
    std::string kernel_ops = kk.str();
    std::string hash = kernel_hash(kernel_ops);
    if(kernel_cache.find(hash) != kernel_cache.end()) {
        void* functor = kernel_cache[hash];
        ((void (*)(std::vector<Kokkos::View<double*>>, std::vector<void*>, std::size_t)) functor)(vec_map, kkCustomOps, vec_dim);
        return;
    }
    std::string file_name = std::string("kernel-")   + hash + ".cc";
    std::string lib_name  = std::string("libkernel-") + hash + ".so";
    std::string kernel = R"(
#include <Kokkos_Core.hpp>

struct ASTFunctor {
    std::vector<Kokkos::View<double*>> vec_map;
    std::vector<void*> custom_ops;

    KOKKOS_INLINE_FUNCTION ASTFunctor(std::vector<Kokkos::View<double*>> _vec_map, std::vector<void*> _custom_ops)
        : vec_map(_vec_map), custom_ops(_custom_ops) {}

    KOKKOS_INLINE_FUNCTION
    void operator()(const int i) const {
)" + kernel_ops + R"(
    }
};

extern "C" void run_kernel(std::vector<Kokkos::View<double*>> vec_map, std::vector<void*> custom_ops, std::size_t vec_dim) {
    ASTFunctor kernel(vec_map, custom_ops);
    Kokkos::parallel_for("debug_label", Kokkos::RangePolicy<>(0, vec_dim), kernel);
}
)";

    std::fstream ofs(file_name, std::ios::out);
    if (!ofs.is_open())
    {
        ckout << "Cannot open file: kernel.cc" << '\n';
        return;
    }
    ofs << kernel;
    ofs.close();
    system(std::string("g++ -O3 -march=native -std=c++20 -I$PWD/_deps/kokkos-src/tpls/mdspan/include "
           "-I$PWD/_deps/kokkos-src/core/src -I$PWD/_deps/kokkos-build -shared "
           "-fPIC -o " + lib_name + " " + file_name + " -L$PWD/_deps/kokkos-build/core/src "
           "-lkokkoscore").c_str());

    void* handle = dlopen(std::string("./" + lib_name).c_str(), RTLD_NOW);
    if (!handle)
    {
        ckout << "Cannot open library: " << dlerror() << '\n';
        return;
    }
    else
    {
        ckout << "Library loaded successfully" << endl;
    }
    void* functor = dlsym(handle, "run_kernel");
    if (!functor)
    {
        ckout << "Cannot load symbol 'kernel': " << dlerror() << '\n';
        dlclose(handle);
        return;
    }
    else
    {
        ckout << "Symbol loaded successfully" << endl;
    }
    ((void (*)(std::vector<Kokkos::View<double*>>, std::vector<void*>, std::size_t)) functor)(vec_map, kkCustomOps, vec_dim);
    kernel_cache[hash] = functor;
}

#define BINOP_CODEGEN(op)                                                      \
    {                                                                          \
        long long leftid = codegen_ast(instruction, node.left_);               \
        long long rightid = codegen_ast(instruction, node.right_);             \
        kkTmpVar++;                                                            \
        kk << "auto tmp" << kkTmpVar << " = ";                                 \
        if (leftid < 0)                                                        \
        {                                                                      \
            kk << "tmp" << -leftid;                                            \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            kk << "vec_map[" << leftid << "](i)";                              \
        }                                                                      \
        kk << " " << op << " ";                                                \
        if (rightid < 0)                                                       \
        {                                                                      \
            kk << "tmp" << -rightid;                                           \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            kk << "vec_map[" << rightid << "](i)";                             \
        }                                                                      \
        kk << ";\n";                                                           \
    }                                                                          \
    return -static_cast<long long>(kkTmpVar);

    long long codegen_ast(
        std::vector<ct::vec_impl::vec_node> const& instruction,
        std::size_t curr_idx)
    {
        const ct::vec_impl::vec_node& node = instruction[curr_idx];

        switch (node.operation_)
        {
        case ct::util::Operation::noop: {
            ;
        } return node.name_;
        case ct::util::Operation::add:         BINOP_CODEGEN("+")
        case ct::util::Operation::sub:         BINOP_CODEGEN("-")
        case ct::util::Operation::divide:      BINOP_CODEGEN("/")
        case ct::util::Operation::multiply:    BINOP_CODEGEN("*")
        case ct::util::Operation::eq:          BINOP_CODEGEN("==")
        case ct::util::Operation::neq:         BINOP_CODEGEN("!=")
        case ct::util::Operation::geq:         BINOP_CODEGEN(">=")
        case ct::util::Operation::leq:         BINOP_CODEGEN("<=")
        case ct::util::Operation::greater:     BINOP_CODEGEN(">")
        case ct::util::Operation::lesser:      BINOP_CODEGEN("<")
        case ct::util::Operation::logical_and: BINOP_CODEGEN("&&")
        case ct::util::Operation::logical_or:  BINOP_CODEGEN("||")
        case ct::util::Operation::logical_not: {
            long long leftid = codegen_ast(instruction, node.left_);
            kkTmpVar++;
            kk << "auto tmp" << kkTmpVar << " = !";
            if (leftid < 0) {
                kk << "tmp" << -leftid;
            } else {
                kk << "vec_map[" << leftid << "](i)";
            }
            kk << ";\n";
        } return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::unary_expr: {
            long long leftid = codegen_ast(instruction, node.left_);
            kkTmpVar++;
            kkCustomOps.push_back(getFuncPtr(node.unary_expr_.get()));
            kk << "auto tmp" << kkTmpVar << " = ";
            kk << "((double(*)(double))custom_ops[" << kkCustomOpIdx << "])(";
            if (leftid < 0) {
                kk << "tmp" << -leftid;
            } else {
                kk << "vec_map[" << leftid << "](i)";
            }
            kk << ");\n";
            kkCustomOpIdx++;
        } return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::binary_expr: {
            long long leftid = codegen_ast(instruction, node.left_);
            long long rightid = codegen_ast(instruction, node.right_);
            kkTmpVar++;
            kkCustomOps.push_back(getFuncPtr(node.binary_expr_.get()));
            kk << "auto tmp" << kkTmpVar << " = ";
            kk << "((double(*)(double,double))custom_ops[" << kkCustomOpIdx
               << "])(";
            if (leftid < 0) {
                kk << "tmp" << -leftid;
            } else {
                kk << "vec_map[" << leftid << "](i)";
            }
            kk << ", ";
            if (rightid < 0) {
                kk << "tmp" << -rightid;
            } else {
                kk << "vec_map[" << rightid << "](i)";
            }
            kk << ");\n";
            kkCustomOpIdx++;
        } return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::broadcast: {
            kkTmpVar++;
            kk << "auto tmp" << kkTmpVar << " = " << node.value_ << ";\n";
        } return -static_cast<long long>(kkTmpVar);
        case ct::util::Operation::where: {
            long long terid = codegen_ast(instruction, node.ter_);
            kkTmpVar++;
            kk << "double tmp" << kkTmpVar << ";\n";
            kk << "if (";
            if (terid < 0) {
                kk << "tmp" << -terid;
            } else {
                kk << "vec_map[" << terid << "]";
            }
            kk << "(i)) {\n";
            long long leftid = codegen_ast(instruction, node.left_);
            kk << "tmp" << kkTmpVar << " = ";
            if (leftid < 0) {
                kk << "tmp" << -leftid;
            } else {
                kk << "vec_map[" << leftid << "]";
            }
            kk << "(i);\n";
            kk << "} else {\n";
            long long rightid = codegen_ast(instruction, node.right_);
            kk << "tmp" << kkTmpVar << " = ";
            if (rightid < 0) {
                kk << "tmp" << -rightid;
            } else {
                kk << "vec_map[" << rightid << "]";
            }
            kk << "(i);\n";
            kk << "}\n";
        } return -static_cast<long long>(kkTmpVar);
        default:
            CmiAbort("Operation not implemented");
        }
        return 0.;
    }
public:
    vector_impl_SDAG_CODE;

    vector_impl(int num_chares_, int vec_block_size_)
      : num_chares(num_chares_)
      , SDAG_INDEX(0)
      , vec_block_size(vec_block_size_)
    {
        vec_map.reserve(1000);

        thisProxy[thisIndex].main_kernel();
    }

private:
    int num_chares;
    std::vector<Kokkos::View<double*>> vec_map;
    std::stringstream kk;
    std::size_t kkTmpVar;
    std::vector<void*> kkCustomOps;
    std::size_t kkCustomOpIdx = 0;
    std::map<std::string, void*> kernel_cache;

    int SDAG_INDEX;
    int vec_block_size;
    int dot_counter = 0;
};

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

    // Instruction related private functions
private:
    void update_partitions(
        std::vector<std::vector<ct::mat_impl::mat_node>> const& instr_list)
    {
        for (auto const& ast : instr_list)
            execute_instruction(ast);
    }

    void execute_instruction(
        std::vector<ct::mat_impl::mat_node> const& instruction,
        std::size_t index = 0)
    {
        ct::mat_impl::mat_node const& node = instruction[index];
        std::size_t node_id = node.name_;
        std::shared_ptr<ct::unary_operator> const& unary_expr =
            node.unary_expr_;
        std::shared_ptr<ct::binary_operator> const& binary_expr =
            node.binary_expr_;

        // Useful variables in switch statement
        std::size_t num_rows{0};
        std::size_t num_cols{0};
        std::size_t total_size{0};
        std::size_t unrolled_size{0};
        std::size_t remainder_start{0};
        std::size_t copy_id{0};
        ct::util::matrix_view mat{};

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(0., 1.);

        switch (node.operation_)
        {
        case ct::util::Operation::init_random:

            CkAssert((mat_map.size() == node_id) &&
                "A matrix is initialized before a dependent matrix "
                "initialization.");

            num_rows = get_mat_rows(node.mat_row_len_);
            num_cols = get_mat_cols(node.mat_col_len_);

            mat = ct::util::matrix_view{num_rows, num_cols};
            for (int row = 0; row != mat.rows(); ++row)
                for (int col = 0; col != mat.cols(); ++col)
                    mat(row, col) = dist(gen);

            mat_map.emplace_back(std::move(mat));

            return;

        case ct::util::Operation::init_value:
            CkAssert((mat_map.size() == node_id) &&
                "A matrix is initialized before a dependent matrix "
                "initialization.");

            num_rows = get_mat_rows(node.mat_row_len_);
            num_cols = get_mat_cols(node.mat_col_len_);

            mat = ct::util::matrix_view{num_rows, num_cols, node.value_};
            mat_map.emplace_back(std::move(mat));

            return;

        case ct::util::Operation::copy:
            copy_id = node.copy_id_;

            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);

                mat = ct::util::matrix_view{num_rows, num_cols};

                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    mat_map[node_id](i, j) = mat_map[copy_id](i, j);
                    mat_map[node_id](i + 1, j) = mat_map[copy_id](i + 1, j);
                    mat_map[node_id](i + 2, j) = mat_map[copy_id](i + 2, j);
                    mat_map[node_id](i + 3, j) = mat_map[copy_id](i + 3, j);
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    mat_map[node_id](i, j) = mat_map[copy_id](i, j);
                }
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
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);

                mat = ct::util::matrix_view{num_rows, num_cols};

                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    mat_map[node_id](i, j) =
                        execute_ast_for_idx(instruction, 0, i, j);
                    mat_map[node_id](i + 1, j) =
                        execute_ast_for_idx(instruction, 0, i + 1, j);
                    mat_map[node_id](i + 2, j) =
                        execute_ast_for_idx(instruction, 0, i + 2, j);
                    mat_map[node_id](i + 3, j) =
                        execute_ast_for_idx(instruction, 0, i + 3, j);
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    mat_map[node_id](i, j) =
                        execute_ast_for_idx(instruction, 0, i, j);
                }
            }

            return;
        case ct::util::Operation::inplace_add:
            copy_id = node.copy_id_;
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);

                mat = ct::util::matrix_view{num_rows, num_cols};

                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == -1)
                    {
                        mat_map[node_id](i, j) +=
                            execute_ast_for_idx(instruction, 1, i, j);
                        mat_map[node_id](i + 1, j) +=
                            execute_ast_for_idx(instruction, 1, i + 1, j);
                        mat_map[node_id](i + 2, j) +=
                            execute_ast_for_idx(instruction, 1, i + 2, j);
                        mat_map[node_id](i + 3, j) +=
                            execute_ast_for_idx(instruction, 1, i + 3, j);
                    }
                    else
                    {
                        mat_map[node_id](i, j) += mat_map[copy_id](i, j);
                        mat_map[node_id](i + 1, j) +=
                            mat_map[copy_id](i + 1, j);
                        mat_map[node_id](i + 2, j) +=
                            mat_map[copy_id](i + 2, j);
                        mat_map[node_id](i + 3, j) +=
                            mat_map[copy_id](i + 3, j);
                    }
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == static_cast<std::size_t>(-1))
                        mat_map[node_id](i, j) +=
                            execute_ast_for_idx(instruction, 1, i, j);
                    else
                        mat_map[node_id](i, j) += mat_map[copy_id](i, j);
                }
            }

            return;
        case ct::util::Operation::inplace_sub:
            copy_id = node.copy_id_;
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);
                mat = ct::util::matrix_view{num_rows, num_cols};
                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == static_cast<std::size_t>(-1))
                    {
                        mat_map[node_id](i, j) -=
                            execute_ast_for_idx(instruction, 1, i, j);
                        mat_map[node_id](i + 1, j) -=
                            execute_ast_for_idx(instruction, 1, i + 1, j);
                        mat_map[node_id](i + 2, j) -=
                            execute_ast_for_idx(instruction, 1, i + 2, j);
                        mat_map[node_id](i + 3, j) -=
                            execute_ast_for_idx(instruction, 1, i + 3, j);
                    }
                    else
                    {
                        mat_map[node_id](i, j) -= mat_map[copy_id](i, j);
                        mat_map[node_id](i + 1, j) -=
                            mat_map[copy_id](i + 1, j);
                        mat_map[node_id](i + 2, j) -=
                            mat_map[copy_id](i + 2, j);
                        mat_map[node_id](i + 3, j) -=
                            mat_map[copy_id](i + 3, j);
                    }
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == static_cast<std::size_t>(-1))
                        mat_map[node_id](i, j) -=
                            execute_ast_for_idx(instruction, 1, i, j);
                    else
                        mat_map[node_id](i, j) -= mat_map[copy_id](i, j);
                }
            }
            return;

        case ct::util::Operation::inplace_divide:
            copy_id = node.copy_id_;
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);
                mat = ct::util::matrix_view{num_rows, num_cols};
                mat_map.emplace_back(std::move(mat));
            }

            total_size = mat_map[node_id].rows();
            unrolled_size = mat_map[node_id].rows() / 4;
            remainder_start = unrolled_size * 4;

            for (std::size_t i = 0; i != remainder_start; i += 4)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == static_cast<std::size_t>(-1))
                    {
                        mat_map[node_id](i, j) /=
                            execute_ast_for_idx(instruction, 1, i, j);
                        mat_map[node_id](i + 1, j) /=
                            execute_ast_for_idx(instruction, 1, i + 1, j);
                        mat_map[node_id](i + 2, j) /=
                            execute_ast_for_idx(instruction, 1, i + 2, j);
                        mat_map[node_id](i + 3, j) /=
                            execute_ast_for_idx(instruction, 1, i + 3, j);
                    }
                    else
                    {
                        mat_map[node_id](i, j) /= mat_map[copy_id](i, j);
                        mat_map[node_id](i + 1, j) /=
                            mat_map[copy_id](i + 1, j);
                        mat_map[node_id](i + 2, j) /=
                            mat_map[copy_id](i + 2, j);
                        mat_map[node_id](i + 3, j) /=
                            mat_map[copy_id](i + 3, j);
                    }
                }
            }

            for (std::size_t i = remainder_start; i != total_size; ++i)
            {
                for (std::size_t j = 0; j != mat_map[node_id].cols(); ++j)
                {
                    if (copy_id == static_cast<std::size_t>(-1))
                        mat_map[node_id](i, j) /=
                            execute_ast_for_idx(instruction, 1, i, j);
                    else
                        mat_map[node_id](i, j) /= mat_map[copy_id](i, j);
                }
            }
            return;

        case ct::util::Operation::custom_expr:
        {
            if (node_id == mat_map.size())
            {
                num_rows = get_mat_rows(node.mat_row_len_);
                num_cols = get_mat_cols(node.mat_col_len_);

                mat = ct::util::matrix_view{num_rows, num_cols};

                mat_map.emplace_back(std::move(mat));
            }

            const ct::mat_impl::mat_node& node = instruction[0];
            node.custom_expr_->operator()(num_rows, num_cols, mat_map[node_id],
                mat_map[instruction[node.left_].name_]);
            return;
        }

        default:
            CmiAbort("Operation not implemented");
        }
    }

    double execute_ast_for_idx(
        std::vector<ct::mat_impl::mat_node> const& instruction,
        std::size_t curr_idx, std::size_t iter_i, std::size_t iter_j)
    {
        const ct::mat_impl::mat_node& node = instruction[curr_idx];

        switch (node.operation_)
        {
        case ct::util::Operation::noop:
            return mat_map[node.name_](iter_i, iter_j);

        case ct::util::Operation::add:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) +
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);

        case ct::util::Operation::sub:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) -
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::multiply:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) *
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::divide:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) /
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::greater:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) >
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::lesser:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) <
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::geq:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) >=
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::leq:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) <=
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::eq:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) ==
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::neq:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) !=
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::unary_expr:
            return node.unary_expr_->operator()(iter_i, iter_j,
                execute_ast_for_idx(instruction, node.left_, iter_i, iter_j));
        case ct::util::Operation::binary_expr:
            return node.binary_expr_->operator()(iter_i, iter_j,
                execute_ast_for_idx(instruction, node.left_, iter_i, iter_j),
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j));
        case ct::util::Operation::broadcast:
            return node.value_;
        case ct::util::Operation::logical_and:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) &&
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::logical_or:
            return execute_ast_for_idx(
                       instruction, node.left_, iter_i, iter_j) ||
                execute_ast_for_idx(instruction, node.right_, iter_i, iter_j);
        case ct::util::Operation::logical_not:
            return !execute_ast_for_idx(
                instruction, node.left_, iter_i, iter_j);
        case ct::util::Operation::where:
            if (execute_ast_for_idx(instruction, node.ter_, iter_i, iter_j))
            {
                return execute_ast_for_idx(
                    instruction, node.left_, iter_i, iter_j);
            }
            else
            {
                return execute_ast_for_idx(
                    instruction, node.right_, iter_i, iter_j);
            }
        default:
            CmiAbort("Operation not implemented");
        }

        // Control should not reach here!
        return 0.;
    }

public:
    matrix_impl_SDAG_CODE;

    matrix_impl(int num_chares_y_, int num_chares_x_, int row_block_len_,
        int col_block_len_)
      : num_chares_y(num_chares_y_)
      , num_chares_x(num_chares_x_)
      , row_block_len(row_block_len_)
      , col_block_len(col_block_len_)
      , SDAG_INDEX(0)
    {
        mat_map.reserve(1000);
        thisProxy(thisIndex.x, thisIndex.y).main_kernel();
    }

private:
    std::vector<ct::util::matrix_view> mat_map;

    int num_chares_y;
    int num_chares_x;

    int row_block_len;
    int col_block_len;
    int SDAG_INDEX;
    int block;
};

class KokkosGroup : public CBase_KokkosGroup
{
private:
    int device;
    // cudaStream_t stream;
public:
    KokkosGroup()
    {
        Kokkos::initialize();
        // int n_devices;
        // cudaGetDeviceCount(&n_devices);
        // device = CkMyPe() % n_devices;
        // cudaSetDevice(device);
        // cudaStreamCreate(&stream);
    }

    void finalize()
    {
        Kokkos::finalize();
        // cudaSetDevice(device);
        // cudaStreamDestroy(&stream);
    }
};