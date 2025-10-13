#pragma once

#include <charmtyles/util/generator.hpp>
#include <cmath>

namespace ct {

    class negate_op : public ct::unary_operator
    {
    public:
        negate_op() = default;
        ~negate_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(negate_op);
        negate_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "negate";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return -val;}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val){return -val;}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class abs_op : public ct::unary_operator
    {
    public:
        abs_op() = default;
        ~abs_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(abs_op);
        abs_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "abs";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return Kokkos::abs(val);}";
        }
        std::string get_mat_signature()
        {
            return "(int a, int b, double val){Kokkos::abs(val);}";
        }
        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class square_op : public ct::unary_operator
    {
    public:
        square_op() = default;
        ~square_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(square_op);
        square_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "square";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return Kokkos::sqaure(val);}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val){return Kokkos::sqaure(val);}";
        }

        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class sqrt_op : public ct::unary_operator
    {
    public:
        sqrt_op() = default;
        ~sqrt_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(sqrt_op);
        sqrt_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "sqrt";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return Kokkos::sqrt(val);}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val){return Kokkos::sqrt(val);}";
        }

        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class reciprocal_op : public ct::unary_operator
    {
    public:
        reciprocal_op() = default;
        ~reciprocal_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(reciprocal_op);
        reciprocal_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "reciprocal";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return 1.0/val;}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val){return 1.0/val;}";
        }

        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class sin_op : public ct::unary_operator
    {
    public:
        sin_op() = default;
        ~sin_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(sin_op);
        sin_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "sin";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return Kokkos::sin(val);}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val){return Kokkos::sin(val);}";
        }

        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class cos_op : public ct::unary_operator
    {
    public:
        cos_op() = default;
        ~cos_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(cos_op);
        cos_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "cos";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return Kokkos::cos(val);}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val){return Kokkos::cos(val);}";
        }

        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class log_op : public ct::unary_operator
    {
    public:
        log_op() = default;
        ~log_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(log_op);
        log_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "log";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return Kokkos::log(val);}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val){return Kokkos::log(val);}";
        }

        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class exp_op : public ct::unary_operator
    {
    public:
        exp_op() = default;
        ~exp_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(exp_op);
        exp_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "exp";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return Kokkos::exp(val);}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val){return Kokkos::exp(val);}";
        }

        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    class scale_op : public ct::unary_operator
    {
    public:
        scale_op() = delete;    // Require scale factor
        scale_op(double scale_factor)
          : scale_factor_(scale_factor)
        {
        }
        ~scale_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(scale_op);
        scale_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }

        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
            p | scale_factor_;
        }
        std::string get_name()
        {
            return "scale";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val, double scale_factor){return "
                   "scale_factor*val;}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val, double scale_factor){return "
                   "scale_factor*val;}";
        }

        std::vector<double> get_extra_params()
        {
            return {scale_factor_};
        }

    private:
        double scale_factor_;
    };

    class add_constant_op : public ct::unary_operator
    {
    public:
        add_constant_op() = delete;    // Require constant
        add_constant_op(double constant)
          : constant_(constant)
        {
        }
        ~add_constant_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(add_constant_op);
        add_constant_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }

        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
            p | constant_;
        }
        std::string get_name()
        {
            return "add_constant";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val, double const){return val + const;}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val, double const){return val + "
                   "const;}";
        }

        std::vector<double> get_extra_params()
        {
            return {constant_};
        }

    private:
        double constant_;
    };

    class relu_op : public ct::unary_operator
    {
    public:
        relu_op() = default;
        ~relu_op() = default;

        using ct::unary_operator::unary_operator;

        PUPable_decl(relu_op);
        relu_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
        std::string get_name()
        {
            return "relu";
        }
        std::string get_vec_signature()
        {
            return "(int a, double val){return Kokkos::max(0.0,val);}";
        }

        std::string get_mat_signature()
        {
            return "(int a, int b, double val){return Kokkos::max(0.0,val);}";
        }

        std::vector<double> get_extra_params()
        {
            return {};
        }
    };

    namespace unary_ops {

        inline std::shared_ptr<ct::unary_operator> negate()
        {
            return std::make_shared<negate_op>();
        }

        inline std::shared_ptr<ct::unary_operator> abs(const std::vector<double>& args)
        {
            return std::make_shared<abs_op>();
        }

        inline std::shared_ptr<ct::unary_operator> square()
        {
            return std::make_shared<square_op>();
        }

        inline std::shared_ptr<ct::unary_operator> sqrt()
        {
            return std::make_shared<sqrt_op>();
        }

        inline std::shared_ptr<ct::unary_operator> reciprocal()
        {
            return std::make_shared<reciprocal_op>();
        }

        inline std::shared_ptr<ct::unary_operator> sin()
        {
            return std::make_shared<sin_op>();
        }

        inline std::shared_ptr<ct::unary_operator> cos()
        {
            return std::make_shared<cos_op>();
        }

        inline std::shared_ptr<ct::unary_operator> log()
        {
            return std::make_shared<log_op>();
        }

        inline std::shared_ptr<ct::unary_operator> exp()
        {
            return std::make_shared<exp_op>();
        }

        inline std::shared_ptr<ct::unary_operator> scale(double factor)
        {
            return std::make_shared<scale_op>(factor);
        }

        inline std::shared_ptr<ct::unary_operator> add_constant(double constant)
        {
            return std::make_shared<add_constant_op>(constant);
        }

        inline std::shared_ptr<ct::unary_operator> relu()
        {
            return std::make_shared<relu_op>();
        }
    }    // namespace unary_ops

}    // namespace ct
