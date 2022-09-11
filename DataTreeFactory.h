#pragma once

#include <node.h>
#include <node_object_wrap.h>

namespace KEngineCore
{
	class DataTree;
	class DataSapling;
	class DataTreeHeader;
}

namespace KEngineCoreNode
{
	class DataSaplingWrapper : public node::ObjectWrap 
	{
	public:
		static void Init(v8::Local<v8::Object> exports);
	private:
		explicit DataSaplingWrapper();
		explicit DataSaplingWrapper(KEngineCore::DataSapling * wrapped);
		~DataSaplingWrapper();

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreateBranchHeader(const v8::FunctionCallbackInfo<v8::Value>& args);



		static void SetHash(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetInt(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetFloat(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetBool(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetString(const v8::FunctionCallbackInfo<v8::Value>& args);


		static void AddKey(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void GrowBranch(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void BranchReady(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void HasBranch(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetBranch(const v8::FunctionCallbackInfo<v8::Value>& args);

		bool					  mOwned{ false };
		KEngineCore::DataSapling* mSapling{ nullptr };
		static v8::Persistent<v8::Function> constructor;
		friend class DataTreeHeaderWrapper;
	};


	class DataTreeHeaderWrapper : public node::ObjectWrap
	{
	public:
		explicit DataTreeHeaderWrapper(KEngineCore::DataTreeHeader * header);
		~DataTreeHeaderWrapper();
		static void Init(v8::Local<v8::Object> exports);

	private:

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void AddHash(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AddInt(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AddFloat(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AddBool(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AddString(const v8::FunctionCallbackInfo<v8::Value>& args);
		
		KEngineCore::DataTreeHeader* mHeader{ nullptr };
		static v8::Persistent<v8::Function> constructor;

		friend class DataSaplingWrapper;
	};

}