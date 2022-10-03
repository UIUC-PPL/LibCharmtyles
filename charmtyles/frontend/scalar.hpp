#pragma once

#include <charmtyles/util/AST.hpp>
#include <charmtyles/util/singleton.hpp>
#include <charmtyles/util/sizes.hpp>

#include <charmtyles/backend/charmtyles_base.hpp>

namespace ct {

    namespace scal_impl {

        CT_GENERATE_SINGLETON(std::size_t, scalar_id);

        std::size_t get_scalar_id()
        {
            std::size_t& id = CT_ACCESS_SINGLETON(scalar_id);
            std::size_t curr_id = id++;

            return curr_id;
        }

        CT_GENERATE_SINGLETON(std::size_t, scalar_sdag_idx);

    }    // namespace scal_impl

    class scalar
    {
    public:
        scalar()
          : scalar_id_(ct::scal_impl::get_scalar_id())
          , most_recent_value_(0)
        {
            std::size_t& sdag_idx =
                CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
            proxy.add_scalar(sdag_idx, scalar_id_, 0);
            ++sdag_idx;
        }

        explicit scalar(double value)
          : scalar_id_(ct::scal_impl::get_scalar_id())
          , most_recent_value_(value)
        {
            std::size_t& sdag_idx =
                CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
            proxy.add_scalar(sdag_idx, scalar_id_, value);
            ++sdag_idx;
        }

        std::size_t scalar_id() const
        {
            return scalar_id_;
        }

        double get() const
        {
            ck::future<double> fval;
            std::size_t& sdag_idx =
                CT_ACCESS_SINGLETON(ct::scal_impl::scalar_sdag_idx);
            proxy.get_value(sdag_idx, scalar_id_, fval);

            ++sdag_idx;
            return fval.get();
        }

    private:
        std::size_t scalar_id_;
        double most_recent_value_;
        mutable CProxy_scalar_impl proxy = scalar_impl_proxy;
    };
}    // namespace ct
