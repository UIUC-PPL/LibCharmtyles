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

        virtual double operator()(std::size_t index, double value) final
        {
            return -value;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return -value;
        }

        PUPable_decl(negate_op);
        negate_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
    };

    class abs_op : public ct::unary_operator
    {
    public:
        abs_op() = default;
        ~abs_op() = default;

        using ct::unary_operator::unary_operator;

        virtual double operator()(std::size_t index, double value) final
        {
            return std::abs(value);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return std::abs(value);
        }

        PUPable_decl(abs_op);
        abs_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
    };

    class square_op : public ct::unary_operator
    {
    public:
        square_op() = default;
        ~square_op() = default;

        using ct::unary_operator::unary_operator;

        virtual double operator()(std::size_t index, double value) final
        {
            return value * value;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return value * value;
        }

        PUPable_decl(square_op);
        square_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
    };

    class sqrt_op : public ct::unary_operator
    {
    public:
        sqrt_op() = default;
        ~sqrt_op() = default;

        using ct::unary_operator::unary_operator;

        virtual double operator()(std::size_t index, double value) final
        {
            return std::sqrt(value);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return std::sqrt(value);
        }

        PUPable_decl(sqrt_op);
        sqrt_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
    };

    class reciprocal_op : public ct::unary_operator
    {
    public:
        reciprocal_op() = default;
        ~reciprocal_op() = default;

        using ct::unary_operator::unary_operator;

        virtual double operator()(std::size_t index, double value) final
        {
            return 1.0 / value;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return 1.0 / value;
        }

        PUPable_decl(reciprocal_op);
        reciprocal_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
    };

    class sin_op : public ct::unary_operator
    {
    public:
        sin_op() = default;
        ~sin_op() = default;

        using ct::unary_operator::unary_operator;

        virtual double operator()(std::size_t index, double value) final
        {
            return std::sin(value);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return std::sin(value);
        }

        PUPable_decl(sin_op);
        sin_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
    };

    class cos_op : public ct::unary_operator
    {
    public:
        cos_op() = default;
        ~cos_op() = default;

        using ct::unary_operator::unary_operator;

        virtual double operator()(std::size_t index, double value) final
        {
            return std::cos(value);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return std::cos(value);
        }

        PUPable_decl(cos_op);
        cos_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
    };

    class log_op : public ct::unary_operator
    {
    public:
        log_op() = default;
        ~log_op() = default;

        using ct::unary_operator::unary_operator;

        virtual double operator()(std::size_t index, double value) final
        {
            return std::log(value);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return std::log(value);
        }

        PUPable_decl(log_op);
        log_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
    };

    class exp_op : public ct::unary_operator
    {
    public:
        exp_op() = default;
        ~exp_op() = default;

        using ct::unary_operator::unary_operator;

        virtual double operator()(std::size_t index, double value) final
        {
            return std::exp(value);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return std::exp(value);
        }

        PUPable_decl(exp_op);
        exp_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
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

        virtual double operator()(std::size_t index, double value) final
        {
            return scale_factor_ * value;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return scale_factor_ * value;
        }

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

        virtual double operator()(std::size_t index, double value) final
        {
            return value + constant_;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return value + constant_;
        }

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

    private:
        double constant_;
    };

    class relu_op : public ct::unary_operator
    {
    public:
        relu_op() = default;
        ~relu_op() = default;

        using ct::unary_operator::unary_operator;

        virtual double operator()(std::size_t index, double value) final
        {
            return std::max(0.0, value);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double value) final
        {
            return std::max(0.0, value);
        }

        PUPable_decl(relu_op);
        relu_op(CkMigrateMessage* m)
          : ct::unary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::unary_operator::pup(p);
        }
    };

    namespace unary_ops {

        inline std::shared_ptr<ct::unary_operator> negate()
        {
            return std::make_shared<negate_op>();
        }

        inline std::shared_ptr<ct::unary_operator> abs()
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
