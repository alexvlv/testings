#include "CamCommands.h"
#include <iostream>

int main() {
	CamCommands::Command cmd = CamCommands::Command::CMD_NIGHT_ON;
	std::cout << CamCommands::toString(cmd).toStdString() << "\n"; // prints "CMD_NIGHT_ON"

	CamCommands::Command cmd2 = CamCommands::Command::URL_H;
	std::cout << CamCommands::toString(cmd2).toStdString() << "\n"; // prints "URL_H"
}
