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
		FloatingPointMircroseconds End;
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
			json << R"("cat":")" << result.Category << "\",";
			json << R"("name":")" << result.Name << "\",";
			json << R"("dur":)" << result.ElapsedTime.count() << ',';
			json << R"("ph":"X",)";
			json << R"("pid":0,)";
			json << R"("tid":)" << result.ThreadID << ",";
			json << R"("ts":)" << result.Start.count();
			json << "}";

			WriteToStream(json);
		}

		void WriteProfileSnapshotStart(const ProfileResult& result)
		{
			std::stringstream json;

			auto hash = std::hash<std::string>{}(result.Name); // To get a unique ID

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << R"("args":{"snapshot":{}},)";
			json << R"("cat":")" << result.Category << "\",";
			json << R"("id":")" << hash << "\",";
			json << R"("name":")" << result.Name << "\",";
			json << R"("ph":"O",)";
			json << R"("pid":0,)";
			json << R"("tid":)" << result.ThreadID << ",";
			json << R"("ts":)" << result.Start.count();
			json << "}";

			WriteToStream(json);
		}

		void WriteProfileSnapshotEnd(const ProfileResult& result)
		{
			std::stringstream json;

			auto hash = std::hash<std::string>{}(result.Name); // To get a unique ID

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << R"("args":{"snapshot":{}},)";
			json << R"("cat":")" << result.Category << "\",";
			json << R"("id":")" << hash << "\",";
			json << R"("name":")" << result.Name << "\",";
			json << R"("ph":"D",)";
			json << R"("pid":0,)";
			json << R"("tid":)" << result.ThreadID << ",";
			json << R"("ts":)" << result.End.count();
			json << "}";

			WriteToStream(json);
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
			_outputStream << R"({"otherData": {},"traceEvents":[{})";
			_outputStream.flush();
		}

		void WriteFooter()
		{
			_outputStream << "]}";
			_outputStream.flush();
		}

		void WriteToStream(std::stringstream& json)
		{
			if (_currentSession != nullptr)
			{
				std::lock_guard lock(_mutex);
				_outputStream << json.str();
				_outputStream.flush();
			}
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
		InstrumentationTimer(const char* name, const char* category, bool isSnapshot = false)
			:_name(name), _category(category), _isSnapshot(isSnapshot), _isStopped(false), _shouldWriteResult(true)
		{
			_profileResult.Name = name;
			_profileResult.Category = category;
			_profileResult.ThreadID = std::this_thread::get_id();
			Start();
		}

		InstrumentationTimer()
			:_name(""), _category(""), _isSnapshot(false), _isStopped(true), _shouldWriteResult(false)
		{

		}

		~InstrumentationTimer()
		{
			if (!_isStopped)
			{
				Stop();
			}
		}

		void Start()
		{
			_startTimepoint = std::chrono::steady_clock::now();
			_profileResult.Start = FloatingPointMircroseconds{ _startTimepoint.time_since_epoch() };

			if (_isSnapshot && _shouldWriteResult)
			{
				Instrumentor::Get().WriteProfileSnapshotStart(_profileResult);
			}
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::steady_clock::now();
			_profileResult.End = FloatingPointMircroseconds{ endTimepoint.time_since_epoch() };

			if (_isSnapshot && _shouldWriteResult)
			{
				Instrumentor::Get().WriteProfileSnapshotEnd(_profileResult);
			}
			else
			{
				_profileResult.ElapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() -
					std::chrono::time_point_cast<std::chrono::microseconds>(_startTimepoint).time_since_epoch();

				if (_shouldWriteResult)
				{
					Instrumentor::Get().WriteProfile(_profileResult);
				}
			}

			_isStopped = true;
		}

		ProfileResult GetProfileResult()
		{
			return _profileResult;
		}

	private:
		const char* _name;
		const char* _category;
		std::chrono::time_point<std::chrono::steady_clock> _startTimepoint;
		bool _isSnapshot;
		bool _isStopped;
		bool _shouldWriteResult;
		struct ProfileResult _profileResult;
	};
}

#define HZ_PROFILE 0
#if HZ_PROFILE
#	define HZ_PROFILE_BEGIN_SESSION(name, filepath) ::Hazel::Instrumentor::Get().BeginSession(name, filepath)
#	define HZ_PROFILE_END_SESSION()  ::Hazel::Instrumentor::Get().EndSession()
#	define HZ_PROFILE_CATEGORY(name, category) ::Hazel::InstrumentationTimer HZ_GET_LINE(timer, __LINE__)(name, category)
#	define HZ_PROFILE_SCOPE(name) HZ_PROFILE_CATEGORY(name, "Scope")
#	define HZ_PROFILE_FUNCTION() HZ_PROFILE_CATEGORY(__FUNCSIG__, "Function")
#	define HZ_PROFILE_SNAPSHOT(name) ::Hazel::InstrumentationTimer HZ_GET_LINE(timer, __LINE__)(name, "snapshot", true)
#else
#	define HZ_PROFILE_BEGIN_SESSION(name, filepath)
#	define HZ_PROFILE_END_SESSION()
#	define HZ_PROFILE_CATEGORY(name, category)
#	define HZ_PROFILE_SCOPE(name)
#	define HZ_PROFILE_FUNCTION()
#	define HZ_PROFILE_SNAPSHOT(name)
#endif // HZ_PROFILE