module;

export module fd.semaphore;

struct semaphore
{
    using size_type =
#ifdef _WIN32
        long
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

    void release(const size_type count = 1);
    void acquire();
};

export namespace fd
{
    using ::semaphore;
} // namespace fd
