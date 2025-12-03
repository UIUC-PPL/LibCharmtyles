#include <Kokkos_Core.hpp>
#include <unordered_map>
#include "region.hpp"

template <typename T>
struct AddOperator
{
    KOKKOS_INLINE_FUNCTION
    static T compute(T a, T b)
    {
        return a + b;
    }
};

template <typename T>
struct SubtractOperator
{
    KOKKOS_INLINE_FUNCTION
    static T compute(T a, T b)
    {
        return a - b;
    }
};

template <typename T>
struct MultiplyOperator
{
    KOKKOS_INLINE_FUNCTION
    static T compute(T a, T b)
    {
        return a * b;
    }
};

template <typename T>
struct DivideOperator
{
    KOKKOS_INLINE_FUNCTION
    static T compute(T a, T b)
    {   
        return a / b;
    }
};

template <typename T>
struct CopyOperator
{
    KOKKOS_INLINE_FUNCTION
    static T compute(T a)
    {
        return a;
    }
};

template<int N, typename T, typename Operator>
struct BinopFunctor;

template <int N, typename T, typename Operator>
struct UnopFunctor;

template <typename T, typename Operator>
struct BinopFunctor<1, T, Operator>
{
    array_view<1, T> a;
    array_view<1, T> b;
    array_view<1, T> result;

    KOKKOS_INLINE_FUNCTION
    void operator()(const int i) const
    {
        int j = i * result.region.step[0];
        int idx_result = j - result.offset[0];
        int idx_a = (j + (a.region.start[0] - result.region.start[0]) - a.offset[0]) * a.stride[0];
        int idx_b = (j + (b.region.start[0] - result.region.start[0]) - b.offset[0]) * b.stride[0];
        result.data(idx_result) = Operator::compute(a.data(idx_a), b.data(idx_b));
    }
};

template <typename T, typename Operator>
struct UnopFunctor<1, T, Operator>
{
    array_view<1, T> a;
    array_view<1, T> result;

    KOKKOS_INLINE_FUNCTION
    void operator()(const int i) const
    {
        int j = i * result.region.step[0];
        int idx_result = j - result.offset[0];
        int idx_a = (j + (a.region.start[0] - result.region.start[0]) - a.offset[0]) * a.stride[0];
        result.data(idx_result) = Operator::compute(a.data(idx_a));
    }
};

template <int N, typename T>
class array_view
{
public:
    using kokkos_view_type = Kokkos::View<KokkosViewDataType<T, N>>;

    template <typename... Extents>
    array_view(const std::string& label_, subregion<N> region_, Extents... extents)
        : region(region_)
        , data(label_, extents...)
    {
        for (int i = 0; i < N; ++i)
        {
            offset[i] = region_.start[i];
            stride[i] = region_.step[i];
        }
    }

    array_view<N, T> get_subview(subregion<N> const& subreg)
    {
        array_view<N, T> subview;
        subview.region = subreg;
        for (int i = 0; i < N; ++i)
            subview.offset[i] = offset[i];
        subview.data = data;
        return subview;
    }

    void add(array_view<N, T> const& a, array_view<N, T> const& b)
    {
        std::vector<int> starts(N);
        std::vector<int> stops(N);
        for (int i = 0; i < N; ++i)
        {
            starts[i] = region.start[i];
            stops[i] = region.stop[i] / region.step[i];
        }
        Kokkos::MDRangePolicy<Kokkos::Rank<N>> policy(starts, stops);
        Kokkos::parallel_for(policy, BinopFunctor<N, AddOperator<T>, T>{a, b, *this});
    }

    void sub(array_view<N, T> const& a, array_view<N, T> const& b)
    {
        std::vector<int> starts(N);
        std::vector<int> stops(N);
        for (int i = 0; i < N; ++i)
        {
            starts[i] = region.start[i];
            stops[i] = region.stop[i] / region.step[i];
        }
        Kokkos::MDRangePolicy<Kokkos::Rank<N>> policy(starts, stops);
        Kokkos::parallel_for(policy, BinopFunctor<N, SubtractOperator<T>, T>{a, b, *this});
    }

    void mul(array_view<N, T> const& a, array_view<N, T> const& b)
    {
        std::vector<int> starts(N);
        std::vector<int> stops(N);
        for (int i = 0; i < N; ++i)
        {
            starts[i] = region.start[i];
            stops[i] = region.stop[i] / region.step[i];
        }
        Kokkos::MDRangePolicy<Kokkos::Rank<N>> policy(starts, stops);
        Kokkos::parallel_for(policy, BinopFunctor<N, MultiplyOperator<T>, T>{a, b, *this});
    }

    void div(array_view<N, T> const& a, array_view<N, T> const& b)
    {
        std::vector<int> starts(N);
        std::vector<int> stops(N);
        for (int i = 0; i < N; ++i)
        {
            starts[i] = region.start[i];
            stops[i] = region.stop[i] / region.step[i];
        }
        Kokkos::MDRangePolicy<Kokkos::Rank<N>> policy(starts, stops);
        Kokkos::parallel_for(policy, BinopFunctor<N, DivideOperator<T>, T>{a, b, *this});
    }

    void copy(array_view<N, T> const& a)
    {
        std::vector<int> starts(N);
        std::vector<int> stops(N);
        for (int i = 0; i < N; ++i)
        {
            starts[i] = region.start[i];
            stops[i] = region.stop[i] / region.step[i];
        }
        Kokkos::MDRangePolicy<Kokkos::Rank<N>> policy(starts, stops);
        Kokkos::parallel_for(policy, UnopFunctor<N, T, CopyOperator<T>>{a, *this});
    }

    subregion<N> region;
    int offset[N];
    int stride[N];
    kokkos_view_type data;
};

template <int N, typename T>
class logical_view
{
public:

    array_view<N, T> get_subview(subregion<N> const& subreg)
    {
        // Assume subreg is fully contained in one of the subregions
        for (const auto& pair : data)
        {
            subregion<N> current_subreg = pair.first;
            // Check if current_subreg overlaps with subreg
            bool overlaps = true;
            for (int i = 0; i < N; ++i)
            {
                if (current_subreg.start[i] > subreg.start[i] || current_subreg.stop[i] < subreg.stop[i])
                {
                    overlaps = false;
                    break;
                }
            }
            if (overlaps)
            {
                // Get the corresponding array_view
                array_view<N, T> current_view = pair.second;

                // Create a new array_view for the overlapping region
                array_view<N, T> overlap_view;
                overlap_view.region = subreg;
                for (int i = 0; i < N; ++i)
                    overlap_view.offset[i] = current_view.offset[i];
                overlap_view.data = current_view.data;

                return overlap_view;
            }
        }
    }

    void add(logical_view<N, T> const& a, logical_view<N, T> const& b)
    {
        for (int i = 0; i < views.size(); ++i)
            views[i].add(a.views[i], b.views[i]);
    }

    void sub(logical_view<N, T> const& a, logical_view<N, T> const& b)
    {
        for (int i = 0; i < views.size(); ++i)
            views[i].sub(a.views[i], b.views[i]);
    }

    void mul(logical_view<N, T> const& a, logical_view<N, T> const& b)
    {
        for (int i = 0; i < views.size(); ++i)
            views[i].mul(a.views[i], b.views[i]);
    }

    void div(logical_view<N, T> const& a, logical_view<N, T> const& b)
    {
        for (int i = 0; i < views.size(); ++i)
            views[i].div(a.views[i], b.views[i]);
    }

    void copy(logical_view<N, T> const& a)
    {
        for (int i = 0; i < views.size(); ++i)
            views[i].copy(a.views[i]);
    }

    std::vector<array_view<N, T>> views;
};

template <typename T>
std::vector<logical_view<1, T>> align_views(std::vector<logical_view<1, T>>& views_vector)
{
    std::vector<int> starts;
    int max_stop = -1;
    for (const auto& view : views_vector)
    {
        for (const auto& pair : view.views)
        {
            starts.push_back(pair.first.start[0]);
            //starts.push_back(pair.first.stop[0]);
            if (pair.first.stop[0] > max_stop)
                max_stop = pair.first.stop[0];
        }
    }

    std::sort(starts.begin(), starts.end());
    starts.erase(std::unique(starts.begin(), starts.end()), starts.end());
    std::vector<logical_view<1, T>> new_views_vector;
    for (auto& view : views_vector)
    {
        logical_view<1, T> new_logical_view;
        for (size_t i = 0; i < starts.size() - 1; ++i)
        {
            subregion<1> subreg;
            subreg.start[0] = starts[i];
            subreg.stop[0] = i == starts.size() - 1 ? max_stop : starts[i + 1];
            subreg.step[0] = 1;
            array_view<1, T> subview = view.get_subview(subreg);
            new_logical_view.views.push_back(subview);
        }
        new_views_vector.push_back(new_logical_view);
    }
    return new_views_vector;
}