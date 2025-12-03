#include <algorithm>
#include <unordered_map>
#include <functional>

template<int N>
class chare_index
{
public:
    int idx[N];
};

template<int N>
bool operator==(chare_index<N> const& a, chare_index<N> const& b)
{
    for (int i = 0; i < N; ++i)
        if (a.idx[i] != b.idx[i])
            return false;
    return true;
}

namespace std {
template<int N>
struct hash<chare_index<N>>
{
    size_t operator()(chare_index<N> const& k) const noexcept
    {
        size_t h = 0;
        std::hash<int> hasher;
        for (int i = 0; i < N; ++i)
        {
            h ^= hasher(k.idx[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

template<int N>
struct hash<region<N>>
{
    size_t operator()(region<N> const& r) const noexcept
    {
        size_t h = 0;
        std::hash<int> hasher;
        for (int i = 0; i < N; ++i)
        {
            h ^= hasher(r.start[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hasher(r.stop[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hasher(r.step[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

template<int N>
struct hash<subregion<N>>
{
    size_t operator()(subregion<N> const& sr) const noexcept
    {
        size_t h = 0;
        std::hash<int> hasher;
        std::hash<region<N>*> ptr_hasher;
        
        // Hash the root pointer
        h ^= ptr_hasher(sr.root) + 0x9e3779b9 + (h << 6) + (h >> 2);
        
        // Hash the region data
        for (int i = 0; i < N; ++i)
        {
            h ^= hasher(sr.start[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hasher(sr.stop[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hasher(sr.step[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};
} // namespace std

template<int N>
class region
{
public:
    int start[N];
    int stop[N];
    int step[N];

    region(region const& other) noexcept
    {
        for (int i = 0; i < N; ++i)
        {
            start[i] = other.start[i];
            stop[i] = other.stop[i];
            step[i] = other.step[i];
        }
    }
};

template<int N>
bool operator==(region<N> const& a, region<N> const& b)
{
    for (int i = 0; i < N; ++i)
    {
        if (a.start[i] != b.start[i] || a.stop[i] != b.stop[i] || a.step[i] != b.step[i])
            return false;
    }
    return true;
}

template<int N>
class subregion : public region<N>
{
public:
    region<N>* root;

    subregion(subregion const& other) noexcept
      : region<N>(other)
      , root(other.root)
    {
    }
};

template<int N>
bool operator==(subregion<N> const& a, subregion<N> const& b)
{
    return a.root == b.root && operator==(static_cast<region<N> const&>(a), static_cast<region<N> const&>(b));
}

template<int N>
subregion<N> intersect(subregion<N> const& a, subregion<N> const& b)
{
    subregion<N> result;
    result.root = a.root; // Assuming both share the same root

    for (int i = 0; i < N; ++i)
    {
        result.start[i] = std::max(a.start[i], b.start[i]);
        result.stop[i] = std::min(a.stop[i], b.stop[i]);
        result.step[i] = std::max(a.step[i], b.step[i]);

        // Check for no intersection
        if (result.start[i] >= result.stop[i])
        {
            // Return an empty subregion
            for (int j = 0; j < N; ++j)
            {
                result.start[j] = 0;
                result.stop[j] = 0;
                result.step[j] = 1;
            }
            return result;
        }
    }

    return result;
}

template<int N>
subregion<N> subtract(subregion<N> const& a, subregion<N> const& b)
{
    subregion<N> result = a; // Start with 'a'

    for (int i = 0; i < N; ++i)
    {
        if (b.start[i] <= a.start[i] && b.stop[i] >= a.stop[i])
        {
            // 'b' completely covers 'a' in this dimension
            result.start[i] = 0;
            result.stop[i] = 0;
            result.step[i] = 1;
        }
        else if (b.start[i] > a.start[i] && b.stop[i] < a.stop[i])
        {
            // 'b' is inside 'a', split 'a' into two parts
            // Here we just adjust the stop of 'a' for simplicity
            result.stop[i] = b.start[i];
        }
        else if (b.start[i] <= a.start[i])
        {
            // Overlap at the start
            result.start[i] = b.stop[i];
        }
        else if (b.stop[i] >= a.stop[i])
        {
            // Overlap at the end
            result.stop[i] = b.start[i];
        }
    }

    return result;
}

std::unordered_map<chare_index<1>, subregion<1>> decompose(subregion<1> const& region, int* chare_sizes)
{
    std::unordered_map<chare_index<1>, subregion<1>> decomposition;

    int chare_size = chare_sizes[0];
    int start = region.start[0];
    int stop = region.stop[0];

    int chare_start = start / chare_size;
    int chare_stop = (stop + chare_size - 1) / chare_size;

    for (int cs = chare_start; cs < chare_stop; cs++)
    {
        subregion<1> subreg = region;
        subreg.start[0] = std::max(start, cs * chare_size);
        subreg.stop[0] = std::min(stop, (cs + 1) * chare_size);
        if (subreg.start[0] < subreg.stop[0])
        {
            chare_index<1> chare_id;
            chare_id.idx[0] = cs;
            decomposition[chare_id] = subreg;
        }
    }

    return decomposition;
}

std::unordered_map<chare_index<2>, subregion<2>> decompose(subregion<2> const& region, int* chare_sizes)
{
    std::unordered_map<chare_index<2>, subregion<2>> decomposition;

    int chare_size_x = chare_sizes[0];
    int chare_size_y = chare_sizes[1];
    int start_x = region.start[0];
    int stop_x = region.stop[0];
    int start_y = region.start[1];
    int stop_y = region.stop[1];

    int chare_start_x = start_x / chare_size_x;
    int chare_stop_x = (stop_x + chare_size_x - 1) / chare_size_x;
    int chare_start_y = start_y / chare_size_y;
    int chare_stop_y = (stop_y + chare_size_y - 1) / chare_size_y;

    for (int cx = chare_start_x; cx < chare_stop_x; cx++)
    {
        for (int cy = chare_start_y; cy < chare_stop_y; cy++)
        {
            subregion<2> subreg = region;
            subreg.start[0] = std::max(start_x, cx * chare_size_x);
            subreg.stop[0] = std::min(stop_x, (cx + 1) * chare_size_x);
            subreg.start[1] = std::max(start_y, cy * chare_size_y);
            subreg.stop[1] = std::min(stop_y, (cy + 1) * chare_size_y);
            if (subreg.start[0] < subreg.stop[0] && subreg.start[1] < subreg.stop[1])
            {
                chare_index<2> chare_id;
                chare_id.idx[0] = cx;
                chare_id.idx[1] = cy;
                decomposition[chare_id] = subreg;
            }
        }
    }

    return decomposition;
}

template<int N>
subregion<N> map(subregion<N> const& r1, subregion<N> const& r2)
{
    for (int i = 0; i < N; ++i)
        static_assert(r1.step[i] == r2.step[i], "Step sizes must match for mapping.");

    subregion<N> result;

    for (int d = 0; d < N; ++d)
    {
        int offset = r2.root->start[d] - r1.root->start[d];
        result.start[d] = r2.start[d] - offset;
        result.stop[d] = r2.stop[d] - offset;
        result.step[d] = r2.step[d];
    }
    return result;
}