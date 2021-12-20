export module cheat.csgo.math.Quaternion;

export namespace cheat::csgo
{
	class Quaternion
	{
	public:
		float x, y, z, w;
	};

	class alignas(16) QuaternionAligned :public Quaternion
	{
	};
}

