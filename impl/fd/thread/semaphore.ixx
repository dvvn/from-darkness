module;

export module fd.semaphore;
import fd.lock_guard;

struct semaphore
{
    using size_type =
#ifdef _WIN32
        unsigned long
#else

#endif
        ;

  private:
    void* handle_;

  public:
    ~semaphore();
    semaphore();
    semaphore(const size_type init_count, const size_type max_count);

    semaphore(const semaphore&)            = delete;
    semaphore& operator=(const semaphore&) = delete;

    semaphore(semaphore&& other);
    semaphore& operator=(semaphore&& other);

    // ++count
    void release(const size_type count = 1);
    // --1
    bool acquire();
};

export namespace fd
{
    template <>
    class lock_guard<semaphore>
    {
        semaphore* sem_;

      public:
        ~lock_guard();

        lock_guard(const lock_guard&) = delete;
        lock_guard(semaphore& sem);

        void release();
    };

    using ::semaphore;
} // namespace fd
