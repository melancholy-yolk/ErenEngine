#include "WinMainCommandParameters.h"

FWinMainCommandParameters::FWinMainCommandParameters(HINSTANCE InHInstance, HINSTANCE InPrevInstance, PSTR InCmdLine, int InShowCmd)
	: HInstance(InHInstance),
	PrevInstance(InHInstance),
	CmdLine(InCmdLine),
	ShowCmd(InShowCmd)
{

}
