#ifndef _TIME_MANAGER_H_
#define _TIME_MANAGER_H_

#include <omp.h>
#include <chrono>
#include <unordered_map>
#include <iostream>

#include "Common/numeric_types.h"
#include "Utils/Logger.h"

class TimeHelper
{
    private:

        struct Interval
        {
            #ifdef _OPENMP
            double start;
            double stop;
            #else
            std::chrono::steady_clock::time_point start;
            std::chrono::steady_clock::time_point stop;
            #endif
        };

        Real fps;
        Real lastTimeSave;

        uint frame;
        Real time;
        Real startTime;
        Real endTime;
        Real ts;

        std::unordered_map<std::string, Interval> intervals;

        Real minTimeStep;
        Real maxTimeStep;

    public:

        TimeHelper()
        {
            time = 0.0;
            ts = 0.0001;
            frame = 1;
            lastTimeSave = 0.0;
        }

        void setFrame(const uint frame) { this->frame = frame; }
        void setTime(const Real time) { this->time = time; }
        void setStartTime(const Real startTime) { this->startTime = startTime; }
        void setEndTime(const Real endTime) { this->endTime = endTime; }
        void setTimeStep(const Real ts) { this->ts = ts; }
        void setFPS(const Real fps) { this->fps = fps; } // Se debe llamar antes que setMinTimeStep()
        void setMinTimeStep(const Real minTs) { Real minFPSTimeStep = static_cast<Real>(1.0) / fps; minFPSTimeStep < minTs? minTimeStep = minFPSTimeStep : minTimeStep = minTs; }
        void setMaxTimeStep(const Real maxTs) { maxTimeStep = maxTs; }

        uint getFrame() { return frame; }
        Real getTime() { return time; }
        Real getStartTime() { return startTime; }
        Real getEndTime() { return endTime; }
        Real getTimeStep() { return ts; }
        Real getFPS() { return fps; }
        Real getMinTimeStep() { return minTimeStep; }
        Real getMaxTimeStep() { return maxTimeStep; }


        void startCounting(const std::string& name)
        {
            #ifdef _OPENMP
            intervals[name].start = omp_get_wtime();
            #else
            intervals[name].start = std::chrono::high_resolution_clock::now();
            #endif
        }

        void stopCounting(const std::string& name)
        {
            #ifdef _OPENMP
            intervals[name].stop = omp_get_wtime();
            #else
            intervals[name].stop = std::chrono::high_resolution_clock::now();
            #endif

            DEBUG(name, " -> ", getInterval(name), " s");
        }

        double getInterval(const std::string& name)
        {
            #ifdef _OPENMP
            return intervals[name].stop - intervals[name].start;
            #else
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(intervals[name].stop - intervals[name].start);
            return duration.count() / 1000000.0;
            #endif
        }

        bool hasToSave()
        {
            if (time - lastTimeSave >= 1.0 / fps)
            {
                lastTimeSave = (Real) frame / fps;
                
                return true;
            }
            else
                return false;
        }

        void clean()
        {
            intervals.clear();
        }
};

#endif
 
