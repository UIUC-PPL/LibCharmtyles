#pragma once

#include <algorithm>
#include <charmtyles/util/generator.hpp>
#include <cmath>

namespace ct {

    class add_op : public ct::binary_operator
    {
    public:
        add_op() = default;
        ~add_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(add_op);
        add_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "add";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return lhs+rhs;}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return lhs+rhs;}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class subtract_op : public ct::binary_operator
    {
    public:
        subtract_op() = default;
        ~subtract_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(subtract_op);
        subtract_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "subtract";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return lhs-rhs;}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return lhs-rhs;}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class multiply_op : public ct::binary_operator
    {
    public:
        multiply_op() = default;
        ~multiply_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(multiply_op);
        multiply_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "multiply";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return lhs*rhs;}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return lhs*rhs;}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class divide_op : public ct::binary_operator
    {
    public:
        divide_op() = default;
        ~divide_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(divide_op);
        divide_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "divide";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return lhs/rhs;}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return lhs/rhs;}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class power_op : public ct::binary_operator
    {
    public:
        power_op() = default;
        ~power_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(power_op);
        power_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "power";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return Kokkos::pow(lhs, "
                   "rhs);}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return "
                   "Kokkos::pow(lhs, rhs);}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class modulo_op : public ct::binary_operator
    {
    public:
        modulo_op() = default;
        ~modulo_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(modulo_op);
        modulo_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "modulo";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return lhs%rhs;}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return lhs%rhs;}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class max_op : public ct::binary_operator
    {
    public:
        max_op() = default;
        ~max_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(max_op);
        max_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "max";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return Kokkos::max(lhs, "
                   "rhs);}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return "
                   "Kokkos::max(lhs, rhs);}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class min_op : public ct::binary_operator
    {
    public:
        min_op() = default;
        ~min_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(min_op);
        min_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "min";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return Kokkos::min(lhs, "
                   "rhs);}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return "
                   "Kokkos::min(lhs, rhs);}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class greater_than_op : public ct::binary_operator
    {
    public:
        greater_than_op() = default;
        ~greater_than_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(greater_than_op);
        greater_than_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "greater_than";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return (lhs>rhs)?1.0:0.0;}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return "
                   "(lhs>rhs)?1.0:0.0;}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class less_than_op : public ct::binary_operator
    {
    public:
        less_than_op() = default;
        ~less_than_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(less_than_op);
        less_than_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "less_than";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return (lhs<rhs)?1.0:0.0;}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return "
                   "(lhs<rhs)?1.0:0.0;}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class equal_op : public ct::binary_operator
    {
    public:
        equal_op()
          : epsilon_(1e-10)
        {
        }
        equal_op(double epsilon)
          : epsilon_(epsilon)
        {
        }
        ~equal_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(equal_op);
        equal_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }

        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
            p | epsilon_;
        }
        std::string get_name()
        {
            return "equal";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs, double epsilon){return "
                   "(Kokkos::abs(lhs-rhs)<epsilon)?1.0:0.0;}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs, double "
                   "epsilon){return "
                   "(Kokkos::abs(lhs-rhs)<epsilon)?1.0:0.0;}";
        }
        std::vector<double> get_extra_params()
        {
            return {epsilon_};
        }

    private:
        double epsilon_;
    };

    class atan2_op : public ct::binary_operator
    {
    public:
        atan2_op() = default;
        ~atan2_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(atan2_op);
        atan2_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
        std::string get_name()
        {
            return "atan2";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs){return Kokkos::atan2(lhs, "
                   "rhs);}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs){return "
                   "Kokkos::atan2(lhs, rhs);}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class weighted_average_op : public ct::binary_operator
    {
    public:
        weighted_average_op()
          : w1_(0.5)
          , w2_(0.5)
        {
        }    // Equal weights by default
        weighted_average_op(double w1, double w2)
          : w1_(w1)
          , w2_(w2)
        {
        }
        ~weighted_average_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(weighted_average_op);
        weighted_average_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }

        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
            p | w1_;
            p | w2_;
        }
        std::string get_name()
        {
            return "weighted_average";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs, double w1, double "
                   "w2){return (w1 * lhs + w2 * rhs) / (w1 + w2);}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs, double w1, double "
                   "w2){return (w1 * lhs + w2 * rhs) / (w1 + w2);}";
        }
        std::vector<double> get_extra_params()
        {
            return {w1_, w2_};
        }

    private:
        double w1_, w2_;
    };

    class axpy_op : public ct::binary_operator{
        public:
        axpy_op() = delete;
        axpy_op(double alpha)
          : alpha_(alpha)
        {
        }
        ~axpy_op() = default;

        using ct::binary_operator::binary_operator;

        PUPable_decl(axpy_op);
        axpy_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
            p | alpha_;
        }
        std::string get_name()
        {
            return "axpy";
        }
        std::string get_vec_signature()
        {
            return "(int a, double lhs, double rhs, double alpha){return alpha*lhs + rhs;}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double lhs, double rhs, double alpha){return alpha*lhs + rhs;;}";
        }
        std::vector<double> get_extra_params()
        {
            return {alpha_};
        }

    private:
        double alpha_;        
    };

    namespace binary_ops {

        inline std::shared_ptr<ct::binary_operator> add(const std::vector<double>& args)
        {
            return std::make_shared<add_op>();
        }

        inline std::shared_ptr<ct::binary_operator> subtract(const std::vector<double>& args)
        {
            return std::make_shared<subtract_op>();
        }

        inline std::shared_ptr<ct::binary_operator> multiply(const std::vector<double>& args)
        {
            return std::make_shared<multiply_op>();
        }

        inline std::shared_ptr<ct::binary_operator> divide(const std::vector<double>& args)
        {
            return std::make_shared<divide_op>();
        }

        inline std::shared_ptr<ct::binary_operator> power(const std::vector<double>& args)
        {
            return std::make_shared<power_op>();
        }

        inline std::shared_ptr<ct::binary_operator> modulo(const std::vector<double>& args)
        {
            return std::make_shared<modulo_op>();
        }

        inline std::shared_ptr<ct::binary_operator> max(const std::vector<double>& args)
        {
            return std::make_shared<max_op>();
        }

        inline std::shared_ptr<ct::binary_operator> min(const std::vector<double>& args)
        {
            return std::make_shared<min_op>();
        }

        inline std::shared_ptr<ct::binary_operator> greater_than(const std::vector<double>& args)
        {
            return std::make_shared<greater_than_op>();
        }

        inline std::shared_ptr<ct::binary_operator> less_than(const std::vector<double>& args)
        {
            return std::make_shared<less_than_op>();
        }

        inline std::shared_ptr<ct::binary_operator> equal(const std::vector<double>& args)
        {
            return std::make_shared<equal_op>(args[0]);
        }

        inline std::shared_ptr<ct::binary_operator> atan2(const std::vector<double>& args)
        {
            return std::make_shared<atan2_op>();
        }

        inline std::shared_ptr<ct::binary_operator> weighted_average(const std::vector<double>& args)
        {
            return std::make_shared<weighted_average_op>(args[0], args[1]);
        }
        inline std::shared_ptr<ct::binary_operator> axpy(const std::vector<double>& args)
        {
            return std::make_shared<axpy_op>(args[0]);
        }
    }    // namespace binary_ops

}    // namespace ct
