#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>

namespace Hazel
{
	using FloatingPointMircroseconds = std::chrono::duration<double, std::micro>;

	struct ProfileResult
	{
		std::string Name;
		std::string Category;
		FloatingPointMircroseconds Start;
		std::chrono::microseconds ElapsedTime;
		std::thread::id ThreadID;
	};

	struct InstrumentationSession
	{
		std::string Name;
	};

	class Instrumentor
	{
	public:
		Instrumentor(const Instrumentor&) = delete;
		Instrumentor(Instrumentor&&) = delete;

		void BeginSession(std::string name, std::string filePath = "results.json")
		{
			std::lock_guard lock(_mutex);
			if (_currentSession != nullptr)
			{
				// If there is already a session running, close it before opening a new one.
				// Profiling ment for original session will end up in this new one,
				// instead of having a badly formated json file.
				if (Log::GetCoreLogger() != nullptr) // Edge case: BeginSession() Might be before Log::Init();
				{
					HZ_CORE_LERROR("Closing Session '{0}' to Begin Session '{1}'.", _currentSession->Name, name);
				}
				InternalEndSession();
			}

			_outputStream.open(std::move(filePath));
			if (_outputStream.is_open())
			{
				_currentSession = new InstrumentationSession{ std::move(name) };
				WriteHeader();
			}
			else
			{
				if (Log::GetCoreLogger() != nullptr) // Edge case: BeginSession() Might be before Log::Init();
				{
					HZ_CORE_LERROR("Instrumentor could not open results file '{0}'.", filePath);
				}
			}
		}

		void EndSession()
		{
			std::lock_guard lock(_mutex);
			InternalEndSession();
		}

		void WriteProfile(const ProfileResult& result)
		{
			std::stringstream json;

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << "\"cat\":\"" << result.Category << "\",";
			json << "\"dur\":" << result.ElapsedTime.count() << ',';
			json << "\"name\":\"" << result.Name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.ThreadID << ",";
			json << "\"ts\":" << result.Start.count();
			json << "}";

			if (_currentSession != nullptr)
			{
				std::lock_guard lock(_mutex);
				_outputStream << json.str();
				_outputStream.flush();
			}
		}



		static Instrumentor& Get()
		{
			static Instrumentor sInstance;
			return sInstance;
		}
	private:
		Instrumentor()
			:_currentSession(nullptr)
		{
		}

		~Instrumentor()
		{
			EndSession();
		}

		void WriteHeader()
		{
			_outputStream << "{\"otherData\": {},\"traceEvents\":[{}";
			_outputStream.flush();
		}

		void WriteFooter()
		{
			_outputStream << "]}";
			_outputStream.flush();
		}

		// You already must own the mutex before calling this.
		void InternalEndSession()
		{
			WriteFooter();
			_outputStream.close();
			delete _currentSession;
			_currentSession = nullptr;
		}

	private:
		InstrumentationSession* _currentSession;
		std::ofstream _outputStream;
		std::mutex _mutex;
	};

	class InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name, const char* category)
			:_name(name),_category(category), _stopped(false)
		{
			_startTimepoint = std::chrono::steady_clock::now();
		}

		~InstrumentationTimer()
		{
			if (!_stopped)
			{
				Stop();
			}
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::steady_clock::now();

			auto higResStart = FloatingPointMircroseconds{ _startTimepoint.time_since_epoch() };
			auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() -
				std::chrono::time_point_cast<std::chrono::microseconds>(_startTimepoint).time_since_epoch();

			Instrumentor::Get().WriteProfile({ _name, _category, higResStart, elapsedTime, std::this_thread::get_id() });

			_stopped = true;
		}

	private:
		const char* _name;
		const char* _category;
		std::chrono::time_point<std::chrono::steady_clock> _startTimepoint;
		bool _stopped;
	};
}

#define HZ_PROFILE 1
#if HZ_PROFILE
#	define HZ_PROFILE_BEGIN_SESSION(name, filepath) ::Hazel::Instrumentor::Get().BeginSession(name, filepath)
#	define HZ_PROFILE_END_SESSION()  ::Hazel::Instrumentor::Get().EndSession()
#	define HZ_PROFILE_CATEGORY(name, category) ::Hazel::InstrumentationTimer timer##__LINE__(name, category)
#	define HZ_PROFILE_SCOPE(name) HZ_PROFILE_CATEGORY(name, "Scope")
#	define HZ_PROFILE_FUNCTION() HZ_PROFILE_CATEGORY(__FUNCSIG__, "Function")
#else
#	define HZ_PROFILE_BEGIN_SESSION(name, filepath)
#	define HZ_PROFILE_END_SESSION()
#	define HZ_PROFILE_CATEGORY(name, category)
#	define HZ_PROFILE_SCOPE(name)
#	define HZ_PROFILE_FUNCTION()
#endif // HZ_PROFILE