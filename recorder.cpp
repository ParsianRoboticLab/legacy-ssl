#include "profiler.h"
#include "worldmodel.h"
#include "agent.h"
#include "coach.h"
#include "soccer.h"
#include "skills.h"
#include "recorder.h"

class CRecorder {
public:
    CRecorder(CNewProfiler _Profiler);
private:
    CNewProfiler profiler;
};
