#include <charmtyles/charmtyles.hpp>
#include <string>

#include "logistic.decl.h"

#include <Eigen/Dense>

/* readonly */ size_t DIMENSION;

// features -> X_train (Data Points * Num Features)
// target -> y_train (Data Points)
// weights -> weight_vector (Num Features)

class sigmoid_t : public ct::unary_operator
{
public:
    sigmoid_t() = default;
    ~sigmoid_t() {}

    using ct::unary_operator::unary_operator;

    virtual double operator()(std::size_t index, double& value) final
    {
        value = 1 / (1 + std::exp(-value));
        return 1.0;
    }

    PUPable_decl(sigmoid_t);
    sigmoid_t(CkMigrateMessage* m)
      : ct::unary_operator(m)
    {
    }

    void pup(PUP::er& p) final
    {
        ct::unary_operator::pup(p);
    }
};

class logistic_regression
{
public:
    explicit logistic_regression(double learning_rate, std::size_t iterations,
        std::size_t weight_vector_size)
      : learning_rate_(learning_rate)
      , iterations_(iterations)
      , weight_vector_(weight_vector_size, 0.)
    {
    }

    void train(ct::matrix const& X_train, ct::vector const& y_train)
    {
        ct::vector predictions = ct::dot(X_train, weight_vector_);
        ct::unary_expr(predictions, sigmoid_);

        ct::vector error = y_train - predictions;
        ct::vector gradient = ct::dot(X_train, error);
        weight_vector_ = ct::axpy(learning_rate_, gradient, weight_vector_);

        for (std::size_t iters = 1; iters != iterations_; ++iters)
        {
            predictions = ct::dot(X_train, weight_vector_);
            ct::unary_expr(predictions, sigmoid_);

            error = y_train - predictions;
            gradient = ct::dot(X_train, error);
            weight_vector_ = ct::axpy(learning_rate_, gradient, weight_vector_);
        }
    }

private:
    double learning_rate_;
    std::size_t iterations_;
    ct::vector weight_vector_;
    std::shared_ptr<sigmoid_t> sigmoid_;
};

class training_generator : public ct::generator
{
public:
    training_generator() = default;
    ~training_generator() {}

    using ct::generator::generator;

    double generate(int row_id, int col_id) final
    {
        return static_cast<double>(row_id + col_id) / 1E4;
    }

    double generate(int dimX) final
    {
        return static_cast<double>(dimX) / 1E4;
    }

    PUPable_decl(training_generator);
    training_generator(CkMigrateMessage* m)
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
        DIMENSION = 1 << 14;
        if (msg->argc > 1)
            DIMENSION = std::stoull(msg->argv[1]);

        ct::init();
        thisProxy.benchmark(DIMENSION);
    }

    void benchmark(size_t dim)
    {
        std::shared_ptr<training_generator> new_training_generator =
            std::make_shared<training_generator>();

        ct::matrix X_train{dim, dim, new_training_generator};
        ct::vector y_train{dim, new_training_generator};

        double start = CkWallTimer();

        logistic_regression logit{0.01, 100, dim};
        logit.train(X_train, y_train);

        ct::sync();
        double end = CkWallTimer();
        ckout << "[Charmtyles] Execution Time: " << end - start << endl;

        CkExit();
    }
};

#include "logistic.def.h"
