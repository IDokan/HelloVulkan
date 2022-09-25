/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Timer.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.08.2021
	header file for timer.
******************************************************************************/
#pragma once
#include <chrono>

class Timer
{
public:
	using clock_t = std::chrono::high_resolution_clock;
	using second_t = std::chrono::duration<double, std::ratio<1>>;
	using typeTimeStamp = std::chrono::time_point <clock_t>;
public:
	static Timer* GetTimer()
	{
		static Timer* timer = new Timer();
		return timer;
	}

	void Reset() noexcept
	{
		timeStamp = clock_t::now();
	}

	double GetEllapsedSeconds() const noexcept
	{
		return std::chrono::duration_cast<second_t>(clock_t::now() - timeStamp).count();
	}

	double GetCurrentTime() const noexcept
	{
		return std::chrono::duration_cast<second_t>(clock_t::now() - startTimeStamp).count();
	}

	double GetDeltaTime()
	{
		double dt = GetEllapsedSeconds();
		fpsEllapsedTime += dt;
		++fpsFrame;

		return dt;
	}

	int GetFPSFrame()
	{
		int frame = -1;
		if (fpsEllapsedTime >= 1.f)
		{
			fpsEllapsedTime = 0.f;
			frame = fpsFrame;
			fpsFrame = 0;

		}

		return frame;
	}

private:
	Timer() : timeStamp(clock_t::now()), startTimeStamp(timeStamp), fpsEllapsedTime(0.f), fpsFrame(0)
	{}

	typeTimeStamp timeStamp;
	typeTimeStamp startTimeStamp;

	double fpsEllapsedTime;
	int fpsFrame;
};

