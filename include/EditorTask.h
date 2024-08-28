#include "rio.h"

class EditorTask : public rio::ITask
{
public:
	EditorTask();

private:
	void prepare_() override;
	void calc_() override;
	void exit_() override;

#if RIO_IS_WIN
	void resize_(s32 width, s32 height);
	static void onResizeCallback_(s32 width, s32 height);
#endif // RIO_IS_WIN

private:
	bool mInitialized = false;
};