#include <charmtyles/charmtyles.hpp>

#include "unary.decl.h"

class identity_t : public ct::unary_operator
{
public:
    identity_t() = default;
    ~identity_t() {}

    using ct::unary_operator::unary_operator;

    inline double operator()(std::size_t index, double value) override final
    {
        return value + 5;
    }

    inline double operator()(
        std::size_t rows, std::size_t cols, double value) override final
    {
        if (rows == cols)
        {
            return 1.0;
        }
        else
        {
            return 0.;
        }
        return 1;
    }

    PUPable_decl(identity_t);
    identity_t(CkMigrateMessage* m)
      : ct::unary_operator(m)
    {
    }

    void pup(PUP::er& p) final
    {
        ct::unary_operator::pup(p);
    }
};

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        int num_pes = 6;
        if (msg->argc > 1)
            num_pes = atoi(msg->argv[1]);

        ct::init();
        thisProxy.benchmark();
    }

    void benchmark()
    {
        std::shared_ptr<ct::unary_operator> identity =
            std::make_shared<identity_t>();
        ct::matrix orig{1000, 1000, 1.0};
        ct::matrix identity_mat = ct::unary_expr(orig, identity);
        ct::vector v1{1000, 1.0};
        ct::vector cpy = ct::unary_expr(v1, identity);
        ct::vector vres = ct::dot(orig, v1);
        ct::vector v_id = ct::dot(identity_mat, v1);
        ct::scalar sval = ct::dot(vres, vres);
        ct::scalar s_id = ct::dot(v_id, v_id);
        double uval = sval.get();
        double val_id = s_id.get();
        std::vector<double> vec = v_id.get();
        std::cout << "The vector is: ";
        for (size_t i = 0; i < vec.size(); i++)
        {
            std::cout << vec[i] << " ";
        }
        std::cout << std::endl;
        ckout << "[Result] Mat-mul sum: " << uval << endl;
        ckout << "[Result] Identity: " << val_id << endl;

        ct::vector v2{100, 0.0};
        ct::vector v3 = ct::unary_expr(v2, identity);
        ct::scalar s2 = ct::max(v3);
        double val2 = s2.get();
        ckout << "[Result] Vector max: " << val2 << endl;

        CkExit();
    }
};

#include "unary.def.h"
