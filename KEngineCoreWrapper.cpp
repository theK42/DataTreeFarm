#include <node.h>
#include "DataTreeFactory.h"

namespace KEngineCoreNode {

	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		DataSaplingWrapper::Init(exports);
		DataTreeHeaderWrapper::Init(exports);
	}

	NODE_MODULE(KEngineCoreNode, InitAll)

}