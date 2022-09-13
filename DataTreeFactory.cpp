#include "DataTreeFactory.h"
#include "DataTree.h"
#include <nan.h>
#include <node.h>
#include <iostream>
#include <fstream>

using namespace v8;

namespace KEngineCoreNode
{

    Persistent<Function> DataSaplingWrapper::constructor;
    Persistent<Function> DataTreeHeaderWrapper::constructor;

	DataSaplingWrapper::DataSaplingWrapper() {
		mSapling = new KEngineCore::DataSapling();
        mSapling->Init(nullptr, nullptr, nullptr);
        mOwned = true;
	}

    DataSaplingWrapper::DataSaplingWrapper(KEngineCore::DataSapling* wrapped)
    {
        mSapling = wrapped;
        mOwned = false;
    }

	DataSaplingWrapper::~DataSaplingWrapper()
	{
        if (mOwned)
		    delete mSapling;
	}

    void DataSaplingWrapper::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        Local<ObjectTemplate> addon_data_tpl = ObjectTemplate::New(isolate);
        addon_data_tpl->SetInternalFieldCount(1);  // 1 field for the DataTreeFactory::New()
        Local<Object> addon_data =
            addon_data_tpl->NewInstance(context).ToLocalChecked();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New, addon_data);
        tpl->SetClassName(String::NewFromUtf8(isolate, "DataSapling").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        // Prototype
        NODE_SET_PROTOTYPE_METHOD(tpl, "setHash", SetHash);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setInt", SetInt);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setFloat", SetFloat);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setBool", SetBool);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setString", SetString);

        NODE_SET_PROTOTYPE_METHOD(tpl, "createBranchHeader", CreateBranchHeader);
        NODE_SET_PROTOTYPE_METHOD(tpl, "addKey", AddKey);

        NODE_SET_PROTOTYPE_METHOD(tpl, "growBranch", GrowBranch);
        NODE_SET_PROTOTYPE_METHOD(tpl, "branchReady", BranchReady);
        
        NODE_SET_PROTOTYPE_METHOD(tpl, "hasBranch", HasBranch);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getBranch", GetBranch);

        NODE_SET_PROTOTYPE_METHOD(tpl, "writeToFile", WriteToFile);

//        Local<Function> constructor = tpl->GetFunction(context).ToLocalChecked();

        auto localConstructor = tpl->GetFunction(context).ToLocalChecked();
        constructor.Reset(isolate, localConstructor);


        addon_data->SetInternalField(0, localConstructor);
        exports->Set(context, String::NewFromUtf8(
            isolate, "DataSapling").ToLocalChecked(),
            localConstructor).FromJust();
    }

    void DataSaplingWrapper::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        if (args.IsConstructCall()) {
            // Invoked as constructor: `new DataSapling(...)`
            DataSaplingWrapper* obj;
            if (args[0]->IsUndefined()) //Raw new
            {
                obj = new DataSaplingWrapper();
            }
            else if (args[1]->IsUndefined()) // CreateBranch
            {
                DataSaplingWrapper* parent = node::ObjectWrap::Unwrap<DataSaplingWrapper>(args[0]->ToObject(context).ToLocalChecked());
                obj = new DataSaplingWrapper(parent->mSapling->GrowBranch());

            }
            else //GetBranch
            {
                DataSaplingWrapper* parent = node::ObjectWrap::Unwrap<DataSaplingWrapper>(args[0]->ToObject(context).ToLocalChecked());
                Nan::Utf8String key(args[1]);
                Nan::Utf8String value(args[2]);
                KEngineCore::DataSapling* retVal = static_cast<KEngineCore::DataSapling*>(parent->mSapling->GetBranch((const char*)(*key), (const char*)(*value)));
                obj = new DataSaplingWrapper(retVal);
            }
            
            obj->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        }
        else {
            // Invoked as plain function `DataSapling(...)`, turn into construct call.
            Local<Function> cons =
                args.Data().As<Object>()->GetInternalField(0).As<Function>();
            Local<Object> result =
                cons->NewInstance(context).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void DataSaplingWrapper::CreateBranchHeader(const FunctionCallbackInfo<Value>& args) {

        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();
        HandleScope scope(isolate);

        const unsigned argc = 1;
        Handle<Value> argv[argc] = { args.Holder() };
        Local<Function> cons = Local<Function>::New(isolate, DataTreeHeaderWrapper::constructor);
        Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

        args.GetReturnValue().Set(instance);

    }


    void DataSaplingWrapper::SetHash(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();

        if (args[0]->IsString() && args[1]->IsString()) {
            Nan::Utf8String key(args[0]);
            Nan::Utf8String value(args[1]);

            DataSaplingWrapper* obj = ObjectWrap::Unwrap<DataSaplingWrapper>(args.Holder());
            obj->mSapling->SetHash((const char*)(*key), (const char*)(*value));
        }
    }    
    
    void DataSaplingWrapper::SetInt(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        if (args[0]->IsString() && args[1]->IsNumber()) {
            Nan::Utf8String key(args[0]);
            double value = args[1]->IsUndefined() ?
                0 : args[1]->NumberValue(context).FromMaybe(0);

            DataSaplingWrapper* obj = ObjectWrap::Unwrap<DataSaplingWrapper>(args.Holder());
            obj->mSapling->SetInt((const char*)(*key), (int)(value));
        }
    }

    void DataSaplingWrapper::SetFloat(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        if (args[0]->IsString() && args[1]->IsNumber()) {
            Nan::Utf8String key(args[1]);
            double value = args[1]->IsUndefined() ?
                0 : args[1]->NumberValue(context).FromMaybe(0);

            DataSaplingWrapper* obj = ObjectWrap::Unwrap<DataSaplingWrapper>(args.Holder());
            obj->mSapling->SetFloat((const char*)(*key), (float)(value));
        }
    }

    void DataSaplingWrapper::SetBool(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        if (args[0]->IsString() && args[1]->IsBoolean()) {
            Nan::Utf8String key(args[0]);
            bool value = args[1]->IsUndefined() ?
                false : args[1]->BooleanValue(isolate);

            DataSaplingWrapper* obj = ObjectWrap::Unwrap<DataSaplingWrapper>(args.Holder());
            obj->mSapling->SetFloat((const char*)(*key), value);
        }
    }

    void DataSaplingWrapper::SetString(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();

        if (args[0]->IsString() && args[1]->IsString()) {
            Nan::Utf8String key(args[0]);
            Nan::Utf8String value(args[1]);

            DataSaplingWrapper* obj = ObjectWrap::Unwrap<DataSaplingWrapper>(args.Holder());
            obj->mSapling->SetString((const char*)(*key), (const char*)(*value));
        }
    }

    void DataSaplingWrapper::AddKey(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();

        if (args[0]->IsString() ) {
            Nan::Utf8String key(args[0]);

            DataSaplingWrapper* obj = ObjectWrap::Unwrap<DataSaplingWrapper>(args.Holder());
            obj->mSapling->AddKey((const char*)(*key));
        }
    }

    void DataSaplingWrapper::GrowBranch(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();
        const unsigned argc = 1;
        Handle<Value> argv[argc] = { args.Holder() };
        Local<Function> cons = Local<Function>::New(isolate, DataSaplingWrapper::constructor);
        Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
        args.GetReturnValue().Set(instance);
    }

    void DataSaplingWrapper::BranchReady(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();

        Local<Context> context = isolate->GetCurrentContext();
        DataSaplingWrapper* obj = ObjectWrap::Unwrap<DataSaplingWrapper>(args.Holder());
        DataSaplingWrapper* branch = ObjectWrap::Unwrap<DataSaplingWrapper>(args[0]->ToObject(context).ToLocalChecked());
        obj->mSapling->BranchReady(branch->mSapling);
    }

    void DataSaplingWrapper::HasBranch(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();

        if (args[0]->IsString() && args[1]->IsString()) {
            Nan::Utf8String key(args[0]);
            Nan::Utf8String value(args[1]);

            DataSaplingWrapper* obj = ObjectWrap::Unwrap<DataSaplingWrapper>(args.Holder());
            bool retVal = obj->mSapling->HasBranch((const char*)(*key), (const char*)(*value));

            args.GetReturnValue().Set(Boolean::New(isolate, retVal));
        }
    }

    void DataSaplingWrapper::GetBranch(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        if (args[0]->IsString() && args[1]->IsString()) {
            Nan::Utf8String key(args[0]);
            Nan::Utf8String value(args[1]);


            const unsigned argc = 3;
            Handle<Value> argv[argc] = { args.Holder(), args[0], args[1] };
            Local<Function> cons = Local<Function>::New(isolate, DataSaplingWrapper::constructor);
            Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
            args.GetReturnValue().Set(instance);
        }
    }

    void DataSaplingWrapper::WriteToFile(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        if (args[0]->IsString()) {
            Nan::Utf8String filename(args[0]);

            std::ofstream stream((const char*)(*filename), std::ios::binary);

            DataSaplingWrapper* obj = ObjectWrap::Unwrap<DataSaplingWrapper>(args.Holder());
            obj->mSapling->WriteToStream(stream);
            stream.close();
        }
    }


    DataTreeHeaderWrapper::DataTreeHeaderWrapper(KEngineCore::DataTreeHeader* header)
    {
        mHeader = header;
    }

    DataTreeHeaderWrapper::~DataTreeHeaderWrapper()
    {
        //DOES NOT OWN WRAPPED OBJECT
        mHeader = nullptr;
    }

    void DataTreeHeaderWrapper::Init(Local<Object> exports) {

        Isolate* isolate = Isolate::GetCurrent();
        Local<Context> context = isolate->GetCurrentContext();
        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(String::NewFromUtf8(isolate, "DataTreeHeader").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        // Prototype
        NODE_SET_PROTOTYPE_METHOD(tpl, "addHash", AddHash);
        NODE_SET_PROTOTYPE_METHOD(tpl, "addInt", AddInt);
        NODE_SET_PROTOTYPE_METHOD(tpl, "addFloat", AddFloat);
        NODE_SET_PROTOTYPE_METHOD(tpl, "addBool", AddBool);
        NODE_SET_PROTOTYPE_METHOD(tpl, "addString", AddString);

        constructor.Reset(isolate, tpl->GetFunction(context).ToLocalChecked());
    }

    void DataTreeHeaderWrapper::New(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = Isolate::GetCurrent();
        Local<Context> context = isolate->GetCurrentContext();
        HandleScope scope(isolate);

        if (args.IsConstructCall()) {
            // Invoked as constructor: `new MyObject(...)`
            
            DataSaplingWrapper* owner = ObjectWrap::Unwrap<DataSaplingWrapper>(args[0]->ToObject(context).ToLocalChecked());

            
            DataTreeHeaderWrapper* obj = new DataTreeHeaderWrapper(owner->mSapling->CreateBranchHeader());
            obj->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        }
        else {
            // Invoked as plain function `MyObject(...)`, turn into construct call.
            const int argc = 1;
            Local<Value> argv[argc] = { args[0] };
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            args.GetReturnValue().Set(cons->NewInstance(context, argc, argv).ToLocalChecked());
        }
    }

    void DataTreeHeaderWrapper::AddHash(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();

        if (args[0]->IsString()) {
            Nan::Utf8String key(args[0]);

            DataTreeHeaderWrapper* obj = ObjectWrap::Unwrap<DataTreeHeaderWrapper>(args.Holder());
            obj->mHeader->AddHash((const char*)(*key));
        }
    }

    void DataTreeHeaderWrapper::AddInt(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();

        if (args[0]->IsString()) {
            Nan::Utf8String key(args[0]);

            DataTreeHeaderWrapper* obj = ObjectWrap::Unwrap<DataTreeHeaderWrapper>(args.Holder());
            obj->mHeader->AddInt((const char*)(*key));
        }
    }

    void DataTreeHeaderWrapper::AddFloat(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();

        if (args[0]->IsString()) {
            Nan::Utf8String key(args[0]);

            DataTreeHeaderWrapper* obj = ObjectWrap::Unwrap<DataTreeHeaderWrapper>(args.Holder());
            obj->mHeader->AddFloat((const char*)(*key));
        }
    }

    void DataTreeHeaderWrapper::AddBool(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();

        if (args[0]->IsString()) {
            Nan::Utf8String key(args[0]);

            DataTreeHeaderWrapper* obj = ObjectWrap::Unwrap<DataTreeHeaderWrapper>(args.Holder());
            obj->mHeader->AddBool((const char*)(*key));
        }
    }

    void DataTreeHeaderWrapper::AddString(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        Isolate* isolate = args.GetIsolate();

        if (args[0]->IsString()) {
            Nan::Utf8String key(args[0]);

            DataTreeHeaderWrapper* obj = ObjectWrap::Unwrap<DataTreeHeaderWrapper>(args.Holder());
            obj->mHeader->AddString((const char*)(*key));
        }
    }


}