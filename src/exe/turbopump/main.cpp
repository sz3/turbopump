#include "main/TurboPumpApp.h"
#include "programmable/TurboApi.h"

int main()
{
	TurboApi api;
	TurboPumpApp app(api, "/tmp/turbopump", 9001);
	app.run();
	return 0;
}
