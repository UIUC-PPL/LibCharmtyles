#include <charmtyles/charmtyles.hpp>

#include "generate.decl.h"

class custom_generator : public ct::generator
{
public:
    custom_generator() = default;
    ~custom_generator() {}

    using ct::generator::generator;

    double generate(int row_id, int col_id) final
    {
        return row_id + col_id;
    }

    double generate(int dimX) final
    {
        return dimX;
    }

    PUPable_decl(custom_generator);
    custom_generator(CkMigrateMessage* m)
      : ct::generator(m)
    {
    }

    void pup(PUP::er& p) final
    {
        ct::generator::pup(p);
    }
};

class Main : public CBase_Main
{
public:
    Main(CkArgMsg* msg)
    {
        ct::init();
        thisProxy.benchmark();
    }

    void benchmark()
    {
        constexpr std::size_t mat_row_len = 1 << 11;
        constexpr std::size_t mat_col_len = 1 << 11;

        constexpr std::size_t vec_len = 1 << 11;

        std::shared_ptr<custom_generator> new_custom_generator =
            std::make_shared<custom_generator>();

        double start = CkWallTimer();

        ct::matrix mat{mat_row_len, mat_col_len, new_custom_generator};
        ct::vector vec{vec_len, new_custom_generator};

        ct::vector res = ct::dot(mat, vec);
        ct::scalar dres = ct::dot(res, res);

        double dres_get = dres.get();

        double end = CkWallTimer();
        ckout << "[Charmtyles] Execution Time: " << end - start << endl;
        ckout << "[Charmtyles] Final Result: " << dres_get << endl;

        start = CkWallTimer();
        Eigen::MatrixXd emat{mat_row_len, mat_col_len};
        Eigen::VectorXd evec{vec_len};
        Eigen::ArrayXd avec{vec_len};

        for (int i = 0; i != vec_len; ++i)
        {
            evec[i] = i;
            avec[i] = i;
        }

        for (int row = 0; row != emat.rows(); ++row)
            for (int col = 0; col != emat.cols(); ++col)
                emat(row, col) = row + col;

        Eigen::VectorXd eres = emat * evec;
        double edres = eres.dot(eres);
        end = CkWallTimer();

        ckout << "[Eigen] Execution Time: " << end - start << endl;
        ckout << "[Eigen] Final Result: " << edres << endl;

        start = CkWallTimer();
        double mat_sum = ct::sum(mat).get();
        double vec_sum = ct::sum(vec).get();
        end = CkWallTimer();

        ckout << "[Charmtyles] Execution Time: " << end - start << endl;
        ckout << "[Charmtyles] Mat Sum: " << mat_sum << ", Vec Sum: " << vec_sum
              << endl;

        start = CkWallTimer();
        double emat_sum = emat.sum();
        double evec_sum = evec.sum();
        end = CkWallTimer();

        ckout << "[Eigen] Execution Time: " << end - start << endl;
        ckout << "[Eigen] Mat Sum: " << emat_sum << ", Vec Sum: " << evec_sum
              << endl;

        start = CkWallTimer();
        double vec_norm = ct::squared_norm(vec).get();
        end = CkWallTimer();

        ckout << "[Charmtyles] Execution Time: " << end - start << endl;
        ckout << "[Charmtyles] Squared Norm: " << vec_norm << endl;

        start = CkWallTimer();

        double evec_norm = Eigen::pow(avec, 2).sum();
        end = CkWallTimer();

        ckout << "[Eigen] Execution Time: " << end - start << endl;
        ckout << "[Eigen] Squared Norm: " << evec_norm << endl;

        start = CkWallTimer();
        double vec_min = ct::min(vec).get();
        end = CkWallTimer();

        ckout << "[Charmtyles] Execution Time: " << end - start << endl;
        ckout << "[Charmtyles] Min Value: " << vec_min << endl;

        start = CkWallTimer();
        Eigen::Index index;
        double evec_min = avec.minCoeff(&index);
        end = CkWallTimer();

        ckout << "[Eigen] Execution Time: " << end - start << endl;
        ckout << "[Eigen] Min Value: " << evec_min << endl;

        start = CkWallTimer();
        double vec_max = ct::max(vec).get();
        end = CkWallTimer();

        ckout << "[Charmtyles] Execution Time: " << end - start << endl;
        ckout << "[Charmtyles] Max Value: " << vec_max << endl;

        start = CkWallTimer();
        double evec_max = avec.maxCoeff(&index);
        end = CkWallTimer();

        ckout << "[Eigen] Execution Time: " << end - start << endl;
        ckout << "[Eigen] Max Value: " << evec_max << endl;

        CkExit();
    }
};

#include "generate.def.h"
