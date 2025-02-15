// WsaContext.h

#pragma once

#include "SocketEmulation.h"
#include "ThreadEmulation.h"
using namespace ThreadEmulation;

#include <vector>
#include <deque>
#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <condition_variable>

#include <assert.h>

namespace SocketEmulation
{

    struct wsa_handle
        : public boost::enable_shared_from_this<wsa_handle>
    {
        typedef boost::shared_ptr<wsa_handle> pointer_t;

        size_t index;
        size_t cls; // 1 - socket, 2 - iocp
    };

    template <typename T, size_t C>
    struct wsa_handle_t
        : wsa_handle
    {
        static size_t const CLS = C;
    };

    struct wsa_context
    {
        wsa_context()
        {
            handles_.push_back(wsa_handle::pointer_t()); // 0 ����Ч���
        }

        ~wsa_context()
        {
        }

        template <typename handle_t>
        handle_t * alloc()
        {
            std::unique_lock<std::mutex> lc(mutex_);
            wsa_handle::pointer_t handle(new handle_t);
                handle->cls = handle_t::CLS;
            if (free_indexs_.empty()) {
                handle->index = handles_.size();
                handles_.push_back(handle);
            } else {
                handle->index = free_indexs_.front();
                free_indexs_.pop_front();
                assert(handles_[handle->index] == NULL);
                handles_[handle->index] = handle;
            }
            return (handle_t *)handle.get();
        }

        template <typename handle_t>
        void free(
            handle_t * handle)
        {
            std::unique_lock<std::mutex> lc(mutex_);
            assert(handles_[handle->index]);
            free_indexs_.push_back(handle->index);
            handles_[handle->index].reset();
        }

        template <typename handle_t>
        handle_t * get(
            size_t index)
        {
            std::unique_lock<std::mutex> lc(mutex_);
            assert(handles_[index]);
            handle_t * handle = (handle_t *)handles_[index].get();
            assert(handle->cls == handle_t::CLS);
            return handle;
        }

    private:
        std::mutex mutex_;
        std::vector<wsa_handle::pointer_t> handles_;
        std::deque<size_t> free_indexs_;
    };

    inline wsa_context & g_wsa_context()
    {
        static wsa_context context;
        return context;
    }

}
