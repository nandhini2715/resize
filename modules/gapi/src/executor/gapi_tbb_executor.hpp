#ifndef OPENCV_GAPI_TBB_EXECUTOR_HPP
#define OPENCV_GAPI_TBB_EXECUTOR_HPP

#if defined(USE_GAPI_TBB_EXECUTOR)

#include <atomic>
#include <vector>
#include <functional>
#include <iostream>

#include <tbb/concurrent_priority_queue.h>

namespace cv { namespace gimpl { namespace parallel {

//simple wrapper to allow copies of std::atomic
template<typename  count_t>
struct atomic_copyable_wrapper {
    std::atomic<count_t> value;

    atomic_copyable_wrapper(count_t val) : value(val) {}
    atomic_copyable_wrapper(atomic_copyable_wrapper const& lhs) : value (lhs.value.load(std::memory_order_relaxed)) {}

    atomic_copyable_wrapper& operator=(count_t val){
        value.store(val, std::memory_order_relaxed);
        return *this;
    }

    count_t fetch_sub(count_t val){
        return value.fetch_sub(val);
    }

    count_t fetch_add(count_t val){
        return value.fetch_add(val);
    }
};

struct async_tag {};
constexpr async_tag async;

struct tile_node {
    //place in totally ordered queue of tasks to execute. Inverse to priority, i.e. lower index means higher priority
    size_t                                                                              total_order_index = 0;
    std::function<void()>                                                               task_body;
    std::function<void(std::function<void()> callback, size_t total_order_index)>       async_task_body;
    //number of dependencies according to a dependency graph (i.e. number of "input" edges).
    //Set only once during graph compilation. (Can not make it const due two two phase initialization of the tile_node objects)
    size_t                                                                              dependencies     = 0;
    //number of unsatisfied dependencies. When drops to zero task is ready for execution.
    //Initially equal to "dependencies"
    atomic_copyable_wrapper<size_t>                                                     dependency_count = 0;
    std::vector<tile_node*>                                                             dependees;

    //FIXME: do not use bool flag - instead generate different task graph for async tasks
    bool async = false;

    tile_node(decltype(task_body) && f) : task_body(std::move(f)) {};
    tile_node(async_tag, decltype(async_task_body) && f) : async_task_body(std::move(f)), async(true) {};
};

struct tile_node_indirect_priority_comparator {
    bool operator()(tile_node const * lhs, tile_node const * rhs) const{
        return lhs->total_order_index > rhs->total_order_index;
    };
};

inline std::ostream& operator<<(std::ostream& o, tile_node const& n){
    o<<"( at:"<<&n <<"," <<"indx: "<< n.total_order_index  <<","<<"deps #:"<< n.dependency_count.value << ", prods:"<< n.dependees.size() <<"[";
    for (auto* d: n.dependees){
        o<<d<<",";
    }
    o<<"]";
    return o;
}

void execute(tbb::concurrent_priority_queue<tile_node* , tile_node_indirect_priority_comparator> & q);

}}} //cv::gimpl::parallel

#endif //USE_GAPI_TBB_EXECUTOR

#endif /* OPENCV_GAPI_TBB_EXECUTOR_HPP */
