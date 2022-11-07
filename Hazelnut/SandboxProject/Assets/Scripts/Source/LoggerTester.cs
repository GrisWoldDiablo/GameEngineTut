using Hazel;

namespace Sandbox
{
	public class LoggerTester : Entity
	{
		void OnCreate()
		{
			Logger.Trace($"{nameof(LoggerTester)} {nameof(Logger.Trace)}");
			Logger.Debug($"{nameof(LoggerTester)} {nameof(Logger.Debug)}");
			Logger.Info($"{nameof(LoggerTester)} {nameof(Logger.Info)}");
			Logger.Warning($"{nameof(LoggerTester)} {nameof(Logger.Warning)}");
			Logger.Error($"{nameof(LoggerTester)} {nameof(Logger.Error)}");
			Logger.Critical($"{nameof(LoggerTester)} {nameof(Logger.Critical)}");
		}
	}
}
