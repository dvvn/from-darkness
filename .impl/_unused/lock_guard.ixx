module;

export module fd.lock_guard;

export namespace fd
{
    template <class T>
    class lock_guard
    {
        T* mtx_;

      public:
        ~lock_guard()
        {
            if (mtx_)
                mtx_->unlock();
        }

        lock_guard(const lock_guard&) = delete;

        lock_guard(T& mtx)
            : mtx_(&mtx)
        {
            mtx_->lock();
        }

        void release()
        {
            mtx_->unlock();
            mtx_ = nullptr;
        }
    };
} // namespace fd
