#include "FPSRoom.h"


void FPSRoom::Frame()
{
	frame.Update();
	acc += frame.GetDelta();
}

// 외부 manager while에서 돌아가야 하는 코드
void FPSRoom::Update()
{
	while (acc >= frame.targetDelta)
	{
		acc -= frame.targetDelta;
	}
}