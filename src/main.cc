//#include "../include/App.h"

#include "../include/Structs.h"
#include <vector>
#include<DirectXMath.h>
#include <iostream>
#include "../include/TerrainChunk.h"
#include "../include/App.h"

int main()
{
	srand(20000809);
	App app{1600, 900, "Terrain Generation"};
	app.Init();
	app.Run();
	app.End();

	TerrainChunk chunk{ 0.0f, 0.0f, 128 };
	chunk.BuildChunk();
	return 0;
}