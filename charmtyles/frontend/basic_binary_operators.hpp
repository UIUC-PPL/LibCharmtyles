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

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return lhs + rhs;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return lhs + rhs;
        }

        PUPable_decl(add_op);
        add_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
    };

    class subtract_op : public ct::binary_operator
    {
    public:
        subtract_op() = default;
        ~subtract_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return lhs - rhs;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return lhs - rhs;
        }

        PUPable_decl(subtract_op);
        subtract_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
    };

    class multiply_op : public ct::binary_operator
    {
    public:
        multiply_op() = default;
        ~multiply_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return lhs * rhs;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return lhs * rhs;
        }

        PUPable_decl(multiply_op);
        multiply_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
    };

    class divide_op : public ct::binary_operator
    {
    public:
        divide_op() = default;
        ~divide_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return lhs / rhs;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return lhs / rhs;
        }

        PUPable_decl(divide_op);
        divide_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
    };

    class power_op : public ct::binary_operator
    {
    public:
        power_op() = default;
        ~power_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return std::pow(lhs, rhs);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return std::pow(lhs, rhs);
        }

        PUPable_decl(power_op);
        power_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
    };

    class modulo_op : public ct::binary_operator
    {
    public:
        modulo_op() = default;
        ~modulo_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double& lhs, double& rhs) final
        {
            return std::fmod(lhs, rhs);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return std::fmod(lhs, rhs);
        }

        PUPable_decl(modulo_op);
        modulo_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
    };

    class max_op : public ct::binary_operator
    {
    public:
        max_op() = default;
        ~max_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double& lhs, double& rhs) final
        {
            return std::max(lhs, rhs);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return std::max(lhs, rhs);
        }

        PUPable_decl(max_op);
        max_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
    };

    class min_op : public ct::binary_operator
    {
    public:
        min_op() = default;
        ~min_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return std::min(lhs, rhs);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return std::min(lhs, rhs);
        }

        PUPable_decl(min_op);
        min_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
    };

    class greater_than_op : public ct::binary_operator
    {
    public:
        greater_than_op() = default;
        ~greater_than_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return (lhs > rhs) ? 1.0 : 0.0;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return (lhs > rhs) ? 1.0 : 0.0;
        }

        PUPable_decl(greater_than_op);
        greater_than_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
        }
    };

    class less_than_op : public ct::binary_operator
    {
    public:
        less_than_op() = default;
        ~less_than_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return (lhs < rhs) ? 1.0 : 0.0;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return (lhs < rhs) ? 1.0 : 0.0;
        }

        PUPable_decl(less_than_op);
        less_than_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
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

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return (std::abs(lhs - rhs) < epsilon_) ? 1.0 : 0.0;
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return (std::abs(lhs - rhs) < epsilon_) ? 1.0 : 0.0;
        }

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

    private:
        double epsilon_;
    };

    class atan2_op : public ct::binary_operator
    {
    public:
        atan2_op() = default;
        ~atan2_op() = default;

        using ct::binary_operator::binary_operator;

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return std::atan2(lhs, rhs);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return std::atan2(lhs, rhs);
        }

        PUPable_decl(atan2_op);
        atan2_op(CkMigrateMessage* m)
          : ct::binary_operator(m)
        {
        }
        void pup(PUP::er& p) final
        {
            ct::binary_operator::pup(p);
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

        virtual double operator()(
            std::size_t index, double lhs, double rhs) final
        {
            return (w1_ * lhs + w2_ * rhs) / (w1_ + w2_);
        }

        virtual double operator()(
            std::size_t i, std::size_t j, double lhs, double rhs) final
        {
            return (w1_ * lhs + w2_ * rhs) / (w1_ + w2_);
        }

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

    private:
        double w1_, w2_;
    };

    namespace binary_ops {

        inline std::shared_ptr<ct::binary_operator> add()
        {
            return std::make_shared<add_op>();
        }

        inline std::shared_ptr<ct::binary_operator> subtract()
        {
            return std::make_shared<subtract_op>();
        }

        inline std::shared_ptr<ct::binary_operator> multiply()
        {
            return std::make_shared<multiply_op>();
        }

        inline std::shared_ptr<ct::binary_operator> divide()
        {
            return std::make_shared<divide_op>();
        }

        inline std::shared_ptr<ct::binary_operator> power()
        {
            return std::make_shared<power_op>();
        }

        inline std::shared_ptr<ct::binary_operator> modulo()
        {
            return std::make_shared<modulo_op>();
        }

        inline std::shared_ptr<ct::binary_operator> max()
        {
            return std::make_shared<max_op>();
        }

        inline std::shared_ptr<ct::binary_operator> min()
        {
            return std::make_shared<min_op>();
        }

        inline std::shared_ptr<ct::binary_operator> greater_than()
        {
            return std::make_shared<greater_than_op>();
        }

        inline std::shared_ptr<ct::binary_operator> less_than()
        {
            return std::make_shared<less_than_op>();
        }

        inline std::shared_ptr<ct::binary_operator> equal(
            double epsilon = 1e-10)
        {
            return std::make_shared<equal_op>(epsilon);
        }

        inline std::shared_ptr<ct::binary_operator> atan2()
        {
            return std::make_shared<atan2_op>();
        }

        inline std::shared_ptr<ct::binary_operator> weighted_average(
            double w1, double w2)
        {
            return std::make_shared<weighted_average_op>(w1, w2);
        }
    }    // namespace binary_ops

}    // namespace ct
