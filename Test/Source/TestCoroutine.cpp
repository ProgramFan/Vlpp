#include "../../Source/UnitTest/UnitTest.h"
#include "../../Source/Reflection/GuiTypeDescriptorReflection.h"
#include "../../Source/Reflection/GuiTypeDescriptorCppHelper.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

/***********************************************************************
Coroutine (Enumerable)
***********************************************************************/

using EC = EnumerableCoroutine;

namespace test_coroutine
{
	class YieldAndPauseEnum : public Object, public ICoroutine
	{
	public:
		EC::IImpl*				impl;
		CoroutineStatus			status = CoroutineStatus::Waiting;
		vint					current = 0;

		YieldAndPauseEnum(EC::IImpl* _impl)
			:impl(_impl)
		{
		}

		void Resume(bool raiseException, Ptr<CoroutineResult> output)override
		{
			if (current < 5)
			{
				status = CoroutineStatus::Executing;
				EC::YieldAndPause(impl, BoxParameter<vint>(current));
				current++;
				status = CoroutineStatus::Waiting;
			}
			else
			{
				status = CoroutineStatus::Stopped;
			}
		}

		Ptr<IValueException> GetFailure()override
		{
			return nullptr;
		}

		CoroutineStatus GetStatus()override
		{
			return status;
		}
	};
}
using namespace test_coroutine;

TEST_CASE(TestYieldAndPause)
{
	auto ec = EC::Create([](auto* impl) {return MakePtr<YieldAndPauseEnum>(impl); });
	List<vint> xs;
	CopyFrom(xs, GetLazyList<vint>(ec));
	int ys[] = { 0,1,2,3,4 };
	TEST_ASSERT(CompareEnumerable(xs, From(ys)) == 0);
}

namespace test_coroutine
{
	class JoinAndPauseEnum : public Object, public ICoroutine
	{
	public:
		EC::IImpl*				impl;
		CoroutineStatus			status = CoroutineStatus::Waiting;
		vint					current = 0;

		JoinAndPauseEnum(EC::IImpl* _impl)
			:impl(_impl)
		{
		}

		void Resume(bool raiseException, Ptr<CoroutineResult> output)override
		{
			if (current < 5)
			{
				status = CoroutineStatus::Executing;
				EC::JoinAndPause(impl, __vwsn::UnboxCollection<IValueEnumerable>(Range<vint>(current, 3)));
				current++;
				status = CoroutineStatus::Waiting;
			}
			else
			{
				status = CoroutineStatus::Stopped;
			}
		}

		Ptr<IValueException> GetFailure()override
		{
			return nullptr;
		}

		CoroutineStatus GetStatus()override
		{
			return status;
		}
	};
}
using namespace test_coroutine;

TEST_CASE(TestJoinAndPause)
{
	auto ec = EC::Create([](auto* impl) {return MakePtr<JoinAndPauseEnum>(impl); });
	List<vint> xs;
	CopyFrom(xs, GetLazyList<vint>(ec));
	int ys[] = { 0,1,2,1,2,3,2,3,4,3,4,5,4,5,6 };
	TEST_ASSERT(CompareEnumerable(xs, From(ys)) == 0);
}

namespace test_coroutine
{
	class MixEnum : public Object, public ICoroutine
	{
	public:
		EC::IImpl*				impl;
		CoroutineStatus			status = CoroutineStatus::Waiting;
		vint					current = 0;

		MixEnum(EC::IImpl* _impl)
			:impl(_impl)
		{
		}

		void Resume(bool raiseException, Ptr<CoroutineResult> output)override
		{
			if (current < 5)
			{
				status = CoroutineStatus::Executing;
				if (current % 2 == 0)
				{
					EC::YieldAndPause(impl, BoxParameter<vint>(current));
				}
				else
				{
					EC::JoinAndPause(impl, __vwsn::UnboxCollection<IValueEnumerable>(Range<vint>(current, 3)));
				}
				current++;
				status = CoroutineStatus::Waiting;
			}
			else
			{
				status = CoroutineStatus::Stopped;
			}
		}

		Ptr<IValueException> GetFailure()override
		{
			return nullptr;
		}

		CoroutineStatus GetStatus()override
		{
			return status;
		}
	};
}
using namespace test_coroutine;

TEST_CASE(TestYieldJoinAndPause)
{
	auto ec = EC::Create([](auto* impl) {return MakePtr<MixEnum>(impl); });
	List<vint> xs;
	CopyFrom(xs, GetLazyList<vint>(ec));
	int ys[] = { 0,1,2,3,2,3,4,5,4 };
	TEST_ASSERT(CompareEnumerable(xs, From(ys)) == 0);
}

/***********************************************************************
Coroutine (Async)
***********************************************************************/

using AC = AsyncCoroutine;

namespace test_coroutine
{
	class SyncScheduler : public Object, public IAsyncScheduler, public Description<SyncScheduler>
	{
	public:
		List<Func<void()>>		tasks;

		void Run()
		{
			while (tasks.Count() > 0)
			{
				auto firstTask = tasks[0];
				tasks.RemoveAt(0);
				firstTask();
			}
		}

		void Execute(const Func<void()>& callback)override
		{
			tasks.Add(callback);
		}

		void DelayExecute(const Func<void()>& callback, vint milliseconds)override
		{
			tasks.Add(callback);
		}
	};
}
using namespace test_coroutine;

namespace test_coroutine
{
	class EmptyAsync : public Object, public ICoroutine
	{
	public:
		AC::IImpl*				impl;
		CoroutineStatus			status = CoroutineStatus::Waiting;

		EmptyAsync(AC::IImpl* _impl)
			:impl(_impl)
		{
		}

		void Resume(bool raiseException, Ptr<CoroutineResult> output)override
		{
			AC::ReturnAndExit(impl, BoxValue<WString>(L"Empty!"));
			status = CoroutineStatus::Stopped;
		}

		Ptr<IValueException> GetFailure()override
		{
			return nullptr;
		}

		CoroutineStatus GetStatus()override
		{
			return status;
		}
	};

	Ptr<IAsync> CreateEmptyAsync()
	{
		return AC::Create([](auto impl) { return MakePtr<EmptyAsync>(impl); });
	}

	class FailAsync : public Object, public ICoroutine
	{
	public:
		AC::IImpl*				impl;
		CoroutineStatus			status = CoroutineStatus::Waiting;

		FailAsync(AC::IImpl* _impl)
			:impl(_impl)
		{
		}

		void Resume(bool raiseException, Ptr<CoroutineResult> output)override
		{
			status = CoroutineStatus::Stopped;
		}

		Ptr<IValueException> GetFailure()override
		{
			return IValueException::Create(L"Fail!");
		}

		CoroutineStatus GetStatus()override
		{
			return status;
		}
	};

	Ptr<IAsync> CreateFailAsync()
	{
		return AC::Create([](auto impl) { return MakePtr<FailAsync>(impl); });
	}
}
using namespace test_coroutine;

TEST_CASE(TestEmptyAsync)
{
	auto scheduler = MakePtr<SyncScheduler>();
	IAsyncScheduler::RegisterDefaultScheduler(scheduler);
	{
		CreateEmptyAsync()->Execute([](auto output)
		{
			TEST_ASSERT(UnboxValue<WString>(output->GetResult()) == L"Empty!");
		});
	}
	scheduler->Run();
	IAsyncScheduler::UnregisterDefaultScheduler();
}

TEST_CASE(TestFailAsync)
{
	auto scheduler = MakePtr<SyncScheduler>();
	IAsyncScheduler::RegisterDefaultScheduler(scheduler);
	{
		CreateFailAsync()->Execute([](auto output)
		{
			TEST_ASSERT(output->GetFailure()->GetMessage() == L"Fail!");
		});
	}
	scheduler->Run();
	IAsyncScheduler::UnregisterDefaultScheduler();
}