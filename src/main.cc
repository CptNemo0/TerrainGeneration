#include "../include/App.h"

int main()
{
	App app{1600, 900, "Cloth Simulation"};
	app.Init();
	app.Run();
	app.End();
	return 0;
}