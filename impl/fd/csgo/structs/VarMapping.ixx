module;

export module fd.csgo.structs.VarMapping;
export import fd.csgo.tools.UtlVector;

export namespace fd::csgo
{
    class IInterpolatedVar;

    class VarMapEntry_t
    {
      public:
        unsigned short type;
        unsigned short m_bNeedsToInterpolate; // Set to false when this var doesn't
        // need Interpolate() called on it anymore.
        void* data;
        IInterpolatedVar* watcher;
    };

    struct VarMapping_t
    {
        CUtlVector<VarMapEntry_t> m_Entries;
        int m_nInterpolatedEntries    = 0;
        float m_lastInterpolationTime = 0;
    };
} // namespace fd::csgo
