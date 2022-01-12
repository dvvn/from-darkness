module;

export module cheat.csgo.interfaces:AnimationLayer;

export namespace cheat::csgo
{
	struct CAnimationLayer
	{
		bool m_bClientBlend;          //0x00
		float m_flBlendIn;            //0x04
		void* m_pStudioHdr;           //0x08
		int m_nDispatchedSrc;         //0x0C
		int m_nDispatchedDst;         //0x10
		int m_iOrder;				  //0x14
		int m_nSequence;			  //0x18
		float m_flPrevCycle;          //0x1C
		float m_flWeight;             //0x20
		float m_flWeightDeltaRate;    //0x24
		float m_flPlaybackRate;       //0x28
		float m_flCycle;              //0x2C
		void* m_pOwner;               //0x30
		int m_nInvalidatePhysicsBits; //0x34
	};
}