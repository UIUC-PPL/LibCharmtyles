#include <charmtyles/charmtyles.hpp>

#include "base.decl.h"

class custom_generator : public ct::util::generator
{
public:
    custom_generator() = default;
    ~custom_generator() {}

    using ct::util::generator::generator;

    void generate(std::size_t chare_idx_x, std::size_t chare_idx_y,
        ct::util::matrix_view& mat)
    {
        std::size_t col_block_len =
            CT_ACCESS_SINGLETON(ct::util::matrix_block_cols);
        std::size_t row_block_len =
            CT_ACCESS_SINGLETON(ct::util::matrix_block_rows);

        for (std::size_t row = 0; row != mat.rows(); ++row)
        {
            for (std::size_t col = 0; col != mat.cols(); ++col)
            {
                mat(row, col) = (chare_idx_x * col_block_len) + col +
                    (chare_idx_y * row_block_len) + row;
            }
        }
    }

    void generate(std::size_t chare_idx, std::vector<double>& vec)
    {
        std::size_t vec_size = CT_ACCESS_SINGLETON(ct::util::array_block_len);

        for (std::size_t i = 0; i != vec.size(); ++i)
        {
            vec[i] = (chare_idx * vec_size) + i;
        }
    }

    PUPable_decl(custom_generator);
    custom_generator(CkMigrateMessage* m)
      : ct::util::generator(m)
    {
    }

    void pup(PUP::er& p)
    {
        ct::util::generator::pup(p);
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
        ckout << "Execution Time: " << end - start << endl;
        // ckout << "Final Result: " << dres_get << endl;

        start = CkWallTimer();
        Eigen::MatrixXd emat{mat_row_len, mat_col_len};
        Eigen::VectorXd evec{vec_len};

        for (int i = 0; i != vec_len; ++i)
            evec[i] = i;

        for (int row = 0; row != emat.rows(); ++row)
            for (int col = 0; col != emat.cols(); ++col)
                emat(row, col) = row + col;

        Eigen::VectorXd eres = emat * evec;
        double edres = eres.dot(eres);
        end = CkWallTimer();

        ckout << "[Eigen] Execution Time: " << end - start << endl;
        ckout << "[Eigen] Final Result: " << edres << endl;

        CkExit();
    }
};

#include "base.def.h"
