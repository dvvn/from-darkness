export module fds.csgo.interfaces.RefCounted;

export namespace fds::csgo
{
    class IRefCounted
    {
      private:
        volatile long refCount;

      public:
        virtual void destructor(char bDelete) = 0;
        virtual bool OnFinalRelease()         = 0;

        /*void unreference()
        {
            if (InterlockedDecrement(&refCount) == 0 && OnFinalRelease())
            {
                destructor(1);
            }
        }*/
    };
} // namespace fds::csgo
