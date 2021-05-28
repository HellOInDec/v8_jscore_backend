#include "v8.h"
#include<cstring>
#include <algorithm>


namespace v8 {
namespace platform {

std::unique_ptr<v8::Platform> NewDefaultPlatform() {
    return std::unique_ptr<v8::Platform>{};
}

}  // namespace platform
}  // namespace v8

namespace v8 {

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
    JSValueRef exception = nullptr;
    double d = JSValueToNumber(Isolate::current_->current_context_->context_, value_, &exception);
    if (exception) {
        return Maybe<uint32_t>();
    }
    else {
        return Maybe<uint32_t>(d);
    }
}
    
Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
    JSValueRef exception = nullptr;
    double d = JSValueToNumber(Isolate::current_->current_context_->context_, value_, &exception);
    if (exception) {
        return Maybe<int32_t>();
    }
    else {
        return Maybe<int32_t>(d);
    }
}

ValueKind Value::getKind() const {
    if (value_ == nullptr) {
        return ValueKind::kNull;
    }

    JSContextRef ctx = Isolate::current_->current_context_->context_;
    JSType type = JSValueGetType(ctx, value_);

    if (type == JSType::kJSTypeUndefined || type == JSType::kJSTypeNull) {
        return ValueKind::kNull;
    } else if (type == JSType::kJSTypeString) {
        return ValueKind::kString;
    } else if (type == JSType::kJSTypeNumber) {
        return ValueKind::kNumber;
    } else if (type == JSType::kJSTypeBoolean) {
        return ValueKind::kBoolean;
    } else if (type == JSType::kJSTypeSymbol) {
        return ValueKind::kSymbol;
    } else if (JSValueIsDate(ctx, value_)) {
        return ValueKind::kDate;
    } else if (JSValueIsArray(ctx, value_)) {
        return ValueKind::kArray;
    } else if (IsArrayBuffer()) {
        return ValueKind::kByteBuffer;
    } else if (JSValueIsObject(ctx, value_)) {
        auto obj = JSValueToObject( ctx, value_, NULL);
    if (JSObjectIsFunction(ctx, obj)) {
        return ValueKind::kFunction;
    }
        return ValueKind::kObject;
    } else {
        return ValueKind::kUnsupported;
    }
}

bool Value::IsArrayBuffer() const {
    if (value_ == nullptr) return false;

    JSValueRef exception = nullptr;
    JSContextRef context = Isolate::current_->current_context_->context_;

    bool ret = JSValueGetTypedArrayType(context, value_, &exception) != kJSTypedArrayTypeNone;
    if (ret || exception) return false;

    // check is DataView
    if (JSValueIsObject(context, value_)) {
        auto ptr = JSObjectGetTypedArrayBytesPtr(context, JSValueToObject( context, value_, &exception),
                                             &exception);
        return ptr != nullptr && !exception;
    }
    return false;
}
    
bool Value::IsUndefined() const {
    return getKind() == ValueKind::kUndefined;
}

bool Value::IsNull() const {
    return getKind() == ValueKind::kNull;
}

bool Value::IsNullOrUndefined() const {
    ValueKind kind = getKind();
    return kind == ValueKind::kNull || kind == ValueKind::kUndefined;
}

bool Value::IsString() const {
    return getKind() == ValueKind::kString;
}

bool Value::IsSymbol() const {
    return  getKind() == ValueKind::kSymbol;
}

Isolate* Promise::GetIsolate() {
    return Isolate::current_;
}

Isolate::Isolate() : Isolate(nullptr) {
}

Isolate::Isolate(void* external_runtime) : current_context_(nullptr) {
    is_external_runtime_ = external_runtime != nullptr;
    //todo rhythm
//    literal_values_[kUndefinedValueIndex] = JSValueMakeUndefined(Isolate::current_->current_context_->context_);
//    literal_values_[kNullValueIndex] = JSValueMakeNull(Isolate::current_->current_context_->context_);
//    literal_values_[kTrueValueIndex] = JSValueMakeBoolean(Isolate::current_->current_context_->context_, true);
//    literal_values_[kFalseValueIndex] = JSValueMakeBoolean(Isolate::current_->current_context_->context_, false);
//    literal_values_[kEmptyStringIndex] = JSValueMakeUndefined(Isolate::current_->current_context_->context_);
    
    virtualMachine_ = JSContextGroupCreate();
    
    //todo rhythm
//    exception_ = JSValueMakeUndefined(Isolate::current_->current_context_->context_);
};

Isolate::~Isolate() {
    for (size_t i = 0; i < values_.size(); i++) {
        delete values_[i];
    }
    values_.clear();
    //todo rhythm
//    JS_FreeValueRT(runtime_, literal_values_[kEmptyStringIndex]);
//    if (!is_external_runtime_) {
//        JS_FreeRuntime(runtime_);
//    }
};

Value* Isolate::Alloc_() {
    if (value_alloc_pos_ == (int)values_.size()) {
        JSValueRef* node = new JSValueRef();
        values_.push_back(node);
    }
    auto ret = reinterpret_cast<Value*>(values_[value_alloc_pos_++]);
    return ret;
}

void Isolate::ForeachAllocValue(int start, int end, std::function<void(JSValueRef*, int)> callback) {
    for (int i = std::min(end, value_alloc_pos_) ; i > std::max(0, start); i--) {
        int idx = i - 1;
        JSValueRef * to_free = values_[idx];
        callback(to_free, idx);
    }
}

void Isolate::Escape(JSValueRef* val) {
    V8::Check(currentHandleScope, "try to escape a scope, but no scope register!");
    currentHandleScope->Escape_(val);
}

Isolate* Isolate::current_ = nullptr;

void Isolate::handleException() {
    if (currentTryCatch_) {
        currentTryCatch_->handleException();
        return;
    }
    
    //todo rhythm
//    JSValue ex = JS_GetException(current_context_->context_);
//
//    if (!JS_IsUndefined(ex) && !JS_IsNull(ex)) {
//        JSValue fileNameVal = JS_GetProperty(current_context_->context_, ex, JS_ATOM_fileName);
//        JSValue lineNumVal = JS_GetProperty(current_context_->context_, ex, JS_ATOM_lineNumber);
//
//        auto msg = JS_ToCString(current_context_->context_, ex);
//        auto fileName = JS_ToCString(current_context_->context_, fileNameVal);
//        auto lineNum = JS_ToCString(current_context_->context_, lineNumVal);
//        if (JS_IsUndefined(fileNameVal)) {
//            std::cerr << "Uncaught " << msg << std::endl;
//        }
//        else {
//            std::cerr << fileName << ":" << lineNum << ": Uncaught " << msg << std::endl;
//        }
//
//        JS_FreeCString(current_context_->context_, lineNum);
//        JS_FreeCString(current_context_->context_, fileName);
//        JS_FreeCString(current_context_->context_, msg);
//
//        JS_FreeValue(current_context_->context_, lineNumVal);
//        JS_FreeValue(current_context_->context_, fileNameVal);
//
//        JS_FreeValue(current_context_->context_, ex);
//    }
}

void Isolate::LowMemoryNotification() {
    Scope isolate_scope(this);
    JSGarbageCollect(this->current_context_->context_);
}

Local<Value> Isolate::ThrowException(Local<Value> exception) {
    exception_ = exception->value_;
    this->Escape(*exception);
    return Local<Value>(exception);
}

void Isolate::SetPromiseRejectCallback(PromiseRejectCallback cb) {
    //todo rhythm
//    JS_SetHostPromiseRejectionTracker(runtime_, [](JSContext *ctx, JSValueConst promise,
//                                                   JSValueConst reason,
//                                                   JS_BOOL is_handled, void *opaque) {
//        PromiseRejectCallback callback = (PromiseRejectCallback)opaque;
//        callback(PromiseRejectMessage(promise, is_handled ? kPromiseHandlerAddedAfterReject : kPromiseRejectWithNoHandler,  reason));
//    }, (void*)cb);
}

Local<Value> Exception::Error(Local<String> message) {
    Isolate *isolate = Isolate::current_;
    Value* val = isolate->Alloc<Value>();
    //todo rhythm
//    JSContext* ctx = isolate->current_context_->context_;
//    val->value_ = JS_NewError(ctx);
//    JS_DefinePropertyValue(ctx, val->value_, JS_ATOM_message, JS_NewString(ctx, *String::Utf8Value(isolate, message)),
//                           JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    return Local<Value>(val);
}

void HandleScope::Escape_(JSValueRef* val) {
    //todo rhythm
//    if (JS_VALUE_HAS_REF_COUNT(*val)) {
//        if (escapes_.find(val) == escapes_.end()) {
//            escapes_.insert(val);
//        }
//        else {
//            JS_DupValueRT(isolate_->runtime_, *val);
//        }
//    }
}

void HandleScope::Exit() {
    //todo rhythm
//    if(prev_pos_ < isolate_->value_alloc_pos_) {
//        //std::cout << "---------------- start HandleScope::Exit -------------------" << std::endl;
//        //std::cout << prev_pos_ << "," << isolate_->value_alloc_pos_ << std::endl;
//        isolate_->ForeachAllocValue(prev_pos_, isolate_->value_alloc_pos_, [this](JSValue* val, int idx){
//            if (JS_VALUE_HAS_REF_COUNT(*val)) {
//                if (this->escapes_.find(val) == this->escapes_.end()) { //not excaped
//                    //std::cout << "free val type:" << JS_VALUE_GET_TAG(*val) << "," << val << ", idx:" << idx << std::endl;
//                    JS_FreeValueRT(isolate_->runtime_, *val);
//                //} else {
//                    //std::cout << "escaped val type:" << JS_VALUE_GET_TAG(*val) << "," << val << std::endl;
//                }
//            }
//        });
//        isolate_->value_alloc_pos_ = prev_pos_;
//        //std::cout << "---------------- end HandleScope::Exit -------------------" << std::endl;
//    }
//
//    if (JS_VALUE_HAS_REF_COUNT(scope_value_)) {
//        if (this->escapes_.find(&scope_value_) == this->escapes_.end()) { //not excaped
//            JS_FreeValueRT(isolate_->runtime_, scope_value_);
//        }
//    }
}

bool Value::IsFunction() const {
    return getKind() == ValueKind::kFunction;
}

bool Value::IsDate() const {
    return getKind() == ValueKind::kDate;
}

bool Value::IsArrayBufferView() const {
    //todo rhythm
    return false;
}

bool Value::IsObject() const {
    return JSValueIsObject(Isolate::current_->current_context_->context_, value_);
}

bool Value::IsBigInt() const {
    //todo rhythm
    return false;
}

bool Value::IsBoolean() const {
    return getKind() == ValueKind::kBoolean;
}

bool Value::IsNumber() const {
    return getKind() == ValueKind::kNumber;
}

bool Value::IsExternal() const {
    //todo rhythm
    return false;
}

bool Value::IsInt32() const {
    return getKind() == ValueKind::kNumber;
}

MaybeLocal<BigInt> Value::ToBigInt(Local<Context> context) const {
    if (IsBigInt()) {
        return MaybeLocal<BigInt>(Local<BigInt>(static_cast<BigInt*>(const_cast<Value*>(this))));
    } else {
        return MaybeLocal<BigInt>();
    }
}

MaybeLocal<Number> Value::ToNumber(Local<Context> context) const {
    if (IsNumber()) {
        return MaybeLocal<Number>(Local<Number>(static_cast<Number*>(const_cast<Value*>(this))));
    }
    else {
        JSValueRef exception = nullptr;
        double d = JSValueToNumber(Isolate::current_->current_context_->context_, value_, &exception);
        if (exception) {
            return MaybeLocal<Number>();
        }
        else {
            return Number::New(context->GetIsolate(), d);
        }
    }
}

Local<Boolean> Value::ToBoolean(Isolate* isolate) const {
    return Local<Boolean>(static_cast<Boolean*>(const_cast<Value*>(this)));
}

MaybeLocal<Int32> Value::ToInt32(Local<Context> context) const {
    return MaybeLocal<Int32>(Local<Int32>(static_cast<Int32*>(const_cast<Value*>(this))));
}

MaybeLocal<Integer> Value::ToInteger(Local<Context> context) const {
    return MaybeLocal<Integer>(Local<Integer>(static_cast<Integer*>(const_cast<Value*>(this))));
}

bool Value::BooleanValue(Isolate* isolate) const {
    return JSValueToBoolean(isolate->current_context_->context_, value_);
}

bool Value::IsRegExp() const {
    //todo rhythm
    return false;
}


MaybeLocal<String> Value::ToString(Local<Context> context) const {
    if (IsString()) {
        return MaybeLocal<String>(Local<String>(static_cast<String*>(const_cast<Value*>(this))));
    } else {
        //由HandleScope跟踪回收
        String * str = context->GetIsolate()->Alloc<String>();
        JSStringRef stringRef = JSStringCreateWithUTF8CString("");
        str->value_ = JSValueMakeString(context->context_, stringRef);
        return MaybeLocal<String>(Local<String>(str));
    }
    
}

V8_WARN_UNUSED_RESULT MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
    if (IsObject()) {
        return MaybeLocal<Object>(Local<Object>(Object::Cast(const_cast<Value*>(this))));
    } else {
        return MaybeLocal<Object>();
    }
}

V8_WARN_UNUSED_RESULT Maybe<double> Value::NumberValue(Local<Context> context) const {
    return Maybe<double>(Number::Cast(const_cast<Value*>(this))->Value());
}

MaybeLocal<String> String::NewFromUtf8(
    Isolate* isolate, const char* data,
    NewStringType type, int length) {
    String *str = isolate->Alloc<String>();
    //printf("NewFromUtf8:%p\n", str);
    size_t len = length > 0 ? length : strlen(data);
    JSStringRef stringRef = JSStringCreateWithUTF8CString(data);
    str->value_ = JSValueMakeString(isolate->current_context_->context_, stringRef);
    return Local<String>(str);
}

Local<String> String::Empty(Isolate* isolate) {
    JSValueRef value = isolate->literal_values_[kEmptyStringIndex];
    JSType type = JSValueGetType(isolate->current_context_->context_, value);
    if ( type == kJSTypeUndefined || type == kJSTypeNull) {
        isolate->literal_values_[kEmptyStringIndex] = JSValueMakeString(isolate->current_context_->context_, JSStringCreateWithUTF8CString(""));
    }
    return Local<String>(reinterpret_cast<String*>(&isolate->literal_values_[kEmptyStringIndex]));
}

int String::Utf8Length(Isolate* isolate) const {
    JSValueRef exception;
    JSStringRef stringRef = JSValueToStringCopy(isolate->current_context_->context_, value_, &exception);
    size_t len = JSStringGetLength(stringRef);
    
    return (int)len;
}

int String::WriteUtf8(Isolate* isolate, char* buffer) const {
    //todo rhythm
//    size_t len;
//    const char* p = JS_ToCStringLen(isolate->current_context_->context_, &len, value_);
//
//    memcpy(buffer, p, len);
//
//    JS_FreeCString(isolate->current_context_->context_, p);
//    return (int)len;
    return 0;
}

//！！如果一个Local<String>用到这个接口了，就不能再传入JS
MaybeLocal<Script> Script::Compile(
    Local<Context> context, Local<String> source,
    ScriptOrigin* origin) {
    Script* script = new Script();
    script->source_ = source;
    if (origin) {
        script->resource_name_ = MaybeLocal<String>(Local<String>::Cast(origin->resource_name_));
    }
    return MaybeLocal<Script>(Local<Script>(script));
}

static V8_INLINE MaybeLocal<Value> ProcessResult(Isolate *isolate, JSValueRef ret) {
    Value* val = nullptr;
    //脚本执行的返回值由HandleScope接管，这可能有需要GC的对象
    val = isolate->Alloc<Value>();
    val->value_ = ret;
    return MaybeLocal<Value>(Local<Value>(val));
}

MaybeLocal<Value> Script::Run(Local<Context> context) {
    auto isolate = context->GetIsolate();

    JSValueRef jscException = nullptr;
    String::Utf8Value source(isolate, source_);
    const char *filename = resource_name_.IsEmpty() ? "eval" : *String::Utf8Value(isolate, resource_name_.ToLocalChecked());
    auto ret = JSEvaluateScript(context->context_, JSStringCreateWithUTF8CString(*source), JSContextGetGlobalObject(context->context_), JSStringCreateWithUTF8CString(filename), 0, &jscException);

    return ProcessResult(isolate, ret);
}

Script::~Script() {
    //JS_FreeValue(context_->context_, source_->value_);
    //if (!resource_name_.IsEmpty()) {
    //    JS_FreeValue(context_->context_, resource_name_.ToLocalChecked()->value_);
    //}
}

Local<External> External::New(Isolate* isolate, void* value) {
    External* external = isolate->Alloc<External>();
    //todo rhythm
//    JS_INITPTR(external->value_, JS_TAG_EXTERNAL, value);
    return Local<External>(external);
}

void* External::Value() const {
    //todo rhythm
//    return JS_VALUE_GET_PTR(value_);
    return nullptr;
}

double Number::Value() const {
    JSValueRef jscException = nullptr;
    double ret = JSValueToNumber(Isolate::current_->current_context_->context_, value_, &jscException);
    return ret;
}

Local<Number> Number::New(Isolate* isolate, double value) {
    Number* ret = isolate->Alloc<Number>();
    ret->value_ = JSValueMakeNumber(isolate->GetCurrentContext()->context_, value);
    return Local<Number>(ret);
}

Local<Integer> Integer::New(Isolate* isolate, int32_t value) {
    Integer* ret = isolate->Alloc<Integer>();
    ret->value_ = JSValueMakeNumber(isolate->GetCurrentContext()->context_, value);
    return Local<Integer>(ret);
}

Local<Integer> Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
    Integer* ret = isolate->Alloc<Integer>();
    ret->value_ = JSValueMakeNumber(isolate->GetCurrentContext()->context_, value);
    return Local<Integer>(ret);
}

Local<BigInt> BigInt::New(Isolate* isolate, int64_t value) {
    BigInt* ret = isolate->Alloc<BigInt>();
    ret->value_ = JSValueMakeNumber(isolate->GetCurrentContext()->context_, value);
    return Local<BigInt>(ret);
}

Local<BigInt> BigInt::NewFromUnsigned(Isolate* isolate, uint64_t value) {
    BigInt* ret = isolate->Alloc<BigInt>();
    ret->value_ = JSValueMakeNumber(isolate->GetCurrentContext()->context_, value);
    return Local<BigInt>(ret);
}

uint64_t BigInt::Uint64Value(bool* lossless) const {
    return static_cast<uint64_t>(Int64Value(lossless));
}

int64_t BigInt::Int64Value(bool* lossless) const {
    JSValueRef jscException = nullptr;
    double ret = JSValueToNumber(Isolate::current_->current_context_->context_, value_, &jscException);
    return static_cast<int64_t>(ret);
}

bool Boolean::Value() const {
    bool ret = JSValueToBoolean(Isolate::current_->current_context_->context_, value_);
    return ret;
}

Local<Boolean> Boolean::New(Isolate* isolate, bool value) {
    Boolean* ret = isolate->Alloc<Boolean>();
    ret->value_ = JSValueMakeBoolean(isolate->GetCurrentContext()->context_, value != 0);
    return Local<Boolean>(ret);
}

int64_t Integer::Value() const {
    JSValueRef jscException = nullptr;
    bool ret = JSValueToNumber(Isolate::current_->current_context_->context_, value_, &jscException);
    return ret;
}

int32_t Int32::Value() const {
    JSValueRef jscException = nullptr;
    double ret = JSValueToNumber(Isolate::current_->current_context_->context_, value_, &jscException);
    return static_cast<int32_t>(ret);
}

String::Utf8Value::Utf8Value(Isolate* isolate, Local<v8::Value> obj) {
    //todo rhythm
    JSValueRef jscException = nullptr;
    auto context = isolate->GetCurrentContext()->context_;
    JSStringRef stringRef = JSValueToStringCopy(context, obj->value_, &jscException);
    len_ = JSStringGetMaximumUTF8CStringSize(stringRef);
    
    char * c_data = new char [len_];
    len_ = JSStringGetUTF8CString(stringRef, c_data, len_);
    data_ = c_data;
}

String::Utf8Value::~Utf8Value() {
}

MaybeLocal<Value> Date::New(Local<Context> context, double time) {
    //todo rhythm
    Date *date = context->GetIsolate()->Alloc<Date>();
//    date->value_ = JS_NewDate(context->context_, time);
    return MaybeLocal<Value>(Local<Date>(date));
}
    
double Date::ValueOf() const {
    //todo rhythm
//    return JS_GetDate(Isolate::current_->current_context_->context_, value_);
    return 0;
}

void Map::Clear() {
    //todo rhythm
//    JS_MapClear(Isolate::current_->GetCurrentContext()->context_, value_);
}

MaybeLocal<Value> Map::Get(Local<Context> context,
                           Local<Value> key) {
    
    //todo rhythm
//    JSValue v = JS_MapGet(context->context_, value_, key->value_);
//    if (JS_IsException(v)) {
//        return MaybeLocal<Value>();
//    }
    Value *val = context->GetIsolate()->Alloc<Value>();
//    val->value_ = v;
    return MaybeLocal<Value>(Local<Value>(val));
}

MaybeLocal<Map> Map::Set(Local<Context> context,
                         Local<Value> key,
                         Local<Value> value) {
    //todo rhythm
//    JSValue m = JS_MapSet(context->context_, value_, key->value_, value->value_);
//    if (JS_IsException(m)) {
//        return MaybeLocal<Map>();
//    }
    Map *map = context->GetIsolate()->Alloc<Map>();
//    map->value_ = m;
    return MaybeLocal<Map>(Local<Map>(map));
}

Local<Map> Map::New(Isolate* isolate) {
    //todo rhythm
    Map *map = isolate->Alloc<Map>();
//    map->value_ = JS_NewMap(isolate->GetCurrentContext()->context_);
    return Local<Map>(map);
}

static std::vector<uint8_t> dummybuffer;

Local<ArrayBuffer> ArrayBuffer::New(Isolate* isolate, size_t byte_length) {
    //todo rhythm
    ArrayBuffer *ab = isolate->Alloc<ArrayBuffer>();
//    if (dummybuffer.size() < byte_length) dummybuffer.resize(byte_length, 0);
//    ab->value_ = JS_NewArrayBufferCopy(isolate->current_context_->context_, dummybuffer.data(), byte_length);
    return Local<ArrayBuffer>(ab);
}

Local<ArrayBuffer> ArrayBuffer::New(Isolate* isolate, void* data, size_t byte_length,
                                           ArrayBufferCreationMode mode) {
    //todo rhythm
    V8::Check(mode == ArrayBufferCreationMode::kExternalized, "only ArrayBufferCreationMode::kExternalized support!");
    ArrayBuffer *ab = isolate->Alloc<ArrayBuffer>();
//    ab->value_ = JS_NewArrayBuffer(isolate->current_context_->context_, (uint8_t*)data, byte_length, nullptr, nullptr, false);
    return Local<ArrayBuffer>(ab);
}

ArrayBuffer::Contents ArrayBuffer::GetContents() {
    //todo rhythm
    ArrayBuffer::Contents ret;
//    ret.data_ = JS_GetArrayBuffer(Isolate::current_->current_context_->context_, &ret.byte_length_, value_);
    return ret;
}

Local<ArrayBuffer> ArrayBufferView::Buffer() {
    //todo rhythm
    Isolate* isolate = Isolate::current_;
    ArrayBuffer* ab = isolate->Alloc<ArrayBuffer>();
//    ab->value_ = JS_GetArrayBufferView(isolate->current_context_->context_, value_);
    return Local<ArrayBuffer>(ab);
}
    
size_t ArrayBufferView::ByteOffset() {
    //todo rhythm
    size_t byte_offset;
    size_t byte_length;
    size_t bytes_per_element;
//    JS_GetArrayBufferViewInfo(Isolate::current_->current_context_->context_, value_, &byte_offset, &byte_length, &bytes_per_element);
    return byte_offset;
}
    
size_t ArrayBufferView::ByteLength() {
    //todo rhythm
    size_t byte_offset;
    size_t byte_length;
    size_t bytes_per_element;
//    JS_GetArrayBufferViewInfo(Isolate::current_->current_context_->context_, value_, &byte_offset, &byte_length, &bytes_per_element);
    return byte_length;
}

Local<Object> Context::Global() {
    Isolate* isolate = Isolate::current_;
    Object *object = isolate->Alloc<Object>();
    object->value_ = global_;
    return Local<Object>(object);
}

Context::Context(Isolate* isolate) : Context(isolate, nullptr) {
}

Context::Context(Isolate* isolate, void* external_context) :isolate_(isolate) {
    //todo rhythm
    is_external_context_ = external_context != nullptr;
    
    
    JSClassDefinition global = kJSClassDefinitionEmpty;
    global.className = "JscEngine";
    JSClassRef globalClass_ = JSClassCreate(&global);
    
    context_ = JSGlobalContextCreateInGroup(isolate->virtualMachine_, globalClass_);
    JSObjectSetPrivate(JSContextGetGlobalObject(context_), this);
    global_ = JSContextGetGlobalObject(context_);
}

Context::~Context() {
    //todo rhythm
//    JS_FreeValue(context_, global_);
//    if (!is_external_context_) {
//        JS_FreeContext(context_);
//    }
}

MaybeLocal<Value> Function::Call(Local<Context> context,
                             Local<Value> recv, int argc,
                             Local<Value> argv[]) {
    //rhythm todo
//    Isolate *isolate = context->GetIsolate();
//    JSValueRef js_this = recv.IsEmpty() ? isolate->Undefined() : (*recv);
//    JSValueRef js_argv = (JSValueRef)alloca(argc * sizeof(JSValueRef));
//    for(int i = 0 ; i < argc; i++) {
//        //isolate->Escape(*argv[i]);
//        js_argv[i] = argv[i]->value_;
//    }
//    JSValue ret = JS_Call(context->context_, value_, *js_this, argc, js_argv);
//
//    return ProcessResult(isolate, ret);
//}
//
//MaybeLocal<Object> Function::NewInstance(Local<Context> context, int argc, Local<Value> argv[]) const {
//    Isolate *isolate = context->GetIsolate();
//    JSValue *js_argv = (JSValue*)alloca(argc * sizeof(JSValue));
//    for(int i = 0 ; i < argc; i++) {
//        //isolate->Escape(*argv[i]);
//        js_argv[i] = argv[i]->value_;
//    }
//
//    JSValue ret = JS_CallConstructor(context->context_, value_, argc, js_argv);
//
//    auto maybe_value = ProcessResult(isolate, ret);
//    if (maybe_value.IsEmpty()) {
//        return MaybeLocal<Object>();
//    } else {
//        return MaybeLocal<Object>(maybe_value.ToLocalChecked().As<Object>());
//    }
}

void Template::Set(Isolate* isolate, const char* name, Local<Data> value) {
    fields_[name] = value;
}

void Template::Set(Local<Name> name, Local<Data> value,
                   PropertyAttribute attributes) {
    Isolate* isolate = Isolate::current_;
    Set(isolate, *String::Utf8Value(Isolate::current_, name), value);
}
    
void Template::SetAccessorProperty(Local<Name> name,
                                         Local<FunctionTemplate> getter,
                                         Local<FunctionTemplate> setter,
                                         PropertyAttribute attribute) {
    
    accessor_property_infos_[*String::Utf8Value(Isolate::current_, name)] = {getter, setter, attribute};
}

void Template::InitPropertys(Local<Context> context, JSValueRef obj) {
    //rhythm todo
//    for(auto it : fields_) {
//        JSAtom atom = JS_NewAtom(context->context_, it.first.data());
//        Local<FunctionTemplate> funcTpl = Local<FunctionTemplate>::Cast(it.second);
//        Local<Function> lfunc = funcTpl->GetFunction(context).ToLocalChecked();
//        context->GetIsolate()->Escape(*lfunc);
//        JS_DefinePropertyValue(context->context_, obj, atom, lfunc->value_, JS_PROP_CONFIGURABLE | JS_PROP_ENUMERABLE | JS_PROP_WRITABLE);
//        JS_FreeAtom(context->context_, atom);
//    }
//
//    for (auto it : accessor_property_infos_) {
//        JSValue getter = JS_Undefined();
//        JSValue setter = JS_Undefined();
//        int flag = 0;
//        if (!(it.second.attribute_ & DontDelete)) {
//            flag |= JS_PROP_CONFIGURABLE;
//        }
//        if (!(it.second.attribute_ & DontEnum)) {
//            flag |= JS_PROP_ENUMERABLE;
//        }
//
//        std::string name = it.first;
//        if (!it.second.getter_.IsEmpty()) {
//            flag |= JS_PROP_HAS_GET;
//            Local<Function> gfunc = it.second.getter_->GetFunction(context).ToLocalChecked();
//            context->GetIsolate()->Escape(*gfunc);
//            getter = gfunc->value_;
//        }
//
//        if (!(it.second.attribute_ & ReadOnly) && !it.second.setter_.IsEmpty()) {
//            flag |= JS_PROP_HAS_SET;
//            flag |= JS_PROP_WRITABLE;
//            Local<Function> sfunc = it.second.setter_->GetFunction(context).ToLocalChecked();
//            context->GetIsolate()->Escape(*sfunc);
//            setter = sfunc->value_;
//        }
//        JSAtom atom = JS_NewAtom(context->context_, name.c_str());
//        JS_DefineProperty(context->context_, obj, atom, JS_Undefined(), getter, setter, flag);
//        JS_FreeAtom(context->context_, atom);
//        JS_FreeValue(context->context_, getter);
//        JS_FreeValue(context->context_, setter);
//    }
}

void ObjectTemplate::SetAccessor(Local<Name> name, AccessorNameGetterCallback getter,
                                 AccessorNameSetterCallback setter,
                                 Local<Value> data, AccessControl settings,
                                 PropertyAttribute attribute) {
    //rhythm todo
//    JSValue js_data = data.IsEmpty() ? JS_Undefined() : data->value_;
//    accessor_infos_[*String::Utf8Value(Isolate::current_, name)] = {getter, setter, js_data, settings, attribute};
}

void ObjectTemplate::InitAccessors(Local<Context> context, JSValueRef obj) {
    //rhythm todo
//    for (auto it : accessor_infos_) {
//        JSValue getter = JS_Undefined();
//        JSValue setter = JS_Undefined();
//        int flag = 0;
//        if (!(it.second.attribute_ & DontDelete)) {
//            flag |= JS_PROP_CONFIGURABLE;
//        }
//        if (!(it.second.attribute_ & DontEnum)) {
//            flag |= JS_PROP_ENUMERABLE;
//        }
//
//        std::string name = it.first;
//
//        auto name_val = String::NewFromUtf8(context->GetIsolate(), name.c_str()).ToLocalChecked();
//
//        JSValue func_data[] = {JS_Undefined(), name_val->value_, it.second.data_};
//
//        if (it.second.getter_) {
//            flag |= JS_PROP_HAS_GET;
//            JS_INITPTR(func_data[0], JS_TAG_EXTERNAL, (void*)it.second.getter_);
//            getter = JS_NewCFunctionData(context->context_, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic, JSValue *func_data) {
//                Isolate* isolate = reinterpret_cast<Context*>(JS_GetContextOpaque(ctx))->GetIsolate();
//
//                PropertyCallbackInfo<Value> callbackInfo;
//                callbackInfo.isolate_ = isolate;
//                callbackInfo.context_ = ctx;
//                callbackInfo.this_ = this_val;
//                callbackInfo.data_ = func_data[2];
//                callbackInfo.value_ = JS_Undefined();
//
//                String *key = reinterpret_cast<String*>(&func_data[1]);
//                AccessorNameGetterCallback callback = (AccessorNameGetterCallback)(JS_VALUE_GET_PTR(func_data[0]));
//                callback(Local<String>(key), callbackInfo);
//
//                if (!JS_IsUndefined(isolate->exception_)) {
//                    JSValue ex = isolate->exception_;
//                    isolate->exception_ = JS_Undefined();
//                    return JS_Throw(ctx, ex);
//                }
//
//                return callbackInfo.value_;
//            }, 0, 0, 3, &func_data[0]);
//        }
//
//        if (!(it.second.attribute_ & ReadOnly) && it.second.setter_) {
//            flag |= JS_PROP_HAS_SET;
//            flag |= JS_PROP_WRITABLE;
//            JS_INITPTR(func_data[0], JS_TAG_EXTERNAL, (void*)it.second.setter_);
//            setter = JS_NewCFunctionData(context->context_, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic, JSValue *func_data) {
//                Isolate* isolate = reinterpret_cast<Context*>(JS_GetContextOpaque(ctx))->GetIsolate();
//
//                PropertyCallbackInfo<void> callbackInfo;
//                callbackInfo.isolate_ = isolate;
//                callbackInfo.context_ = ctx;
//                callbackInfo.this_ = this_val;
//                callbackInfo.data_ = func_data[2];
//                callbackInfo.value_ = JS_Undefined();
//
//                String *key = reinterpret_cast<String*>(&func_data[1]);
//                Value *val = reinterpret_cast<Value*>(argv);
//                AccessorNameSetterCallback callback = (AccessorNameSetterCallback)(JS_VALUE_GET_PTR(func_data[0]));
//                callback(Local<String>(key), Local<Value>(val), callbackInfo);
//
//                if (!JS_IsUndefined(isolate->exception_)) {
//                    JSValue ex = isolate->exception_;
//                    isolate->exception_ = JS_Undefined();
//                    return JS_Throw(ctx, ex);
//                }
//
//                return callbackInfo.value_;
//            }, 0, 0, 3, &func_data[0]);
//        }
//        JSAtom atom = JS_NewAtom(context->context_, name.c_str());
//        JS_DefineProperty(context->context_, obj, atom, JS_Undefined(), getter, setter, flag);
//        JS_FreeAtom(context->context_, atom);
//        JS_FreeValue(context->context_, getter);
//        JS_FreeValue(context->context_, setter);
//    }
}

void ObjectTemplate::SetInternalFieldCount(int value) {
    internal_field_count_ = value;
}

Local<FunctionTemplate> FunctionTemplate::New(Isolate* isolate, FunctionCallback callback,
                                              Local<Value> data) {
    //rhythm todo
//    Local<FunctionTemplate> functionTemplate(new FunctionTemplate());
//    if (data.IsEmpty()) {
//        functionTemplate->cfunction_data_.data_ = JS_Undefined();
//    } else {
//        functionTemplate->cfunction_data_.data_ = data->value_;
//    }
//    functionTemplate->cfunction_data_.callback_ = callback;
//
//    //isolate->RegFunctionTemplate(functionTemplate);
//    functionTemplate->isolate_ = isolate;
//    return functionTemplate;
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
    if (instance_template_.IsEmpty()) {
        instance_template_ = Local<ObjectTemplate>(new ObjectTemplate());
    }
    return instance_template_;
}
    
void FunctionTemplate::Inherit(Local<FunctionTemplate> parent) {
    parent_ = parent;
}
    
Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
    if (prototype_template_.IsEmpty()) {
        prototype_template_ = Local<ObjectTemplate>(new ObjectTemplate());
    }
    return prototype_template_;
}

MaybeLocal<Function> FunctionTemplate::GetFunction(Local<Context> context) {
    //rhythm todo
//    auto iter = context_to_funtion_.find(context);
//    if (iter != context_to_funtion_.end()) {
//        Function* ret = isolate_->Alloc<Function>();
//        ret->value_ = iter->second;
//        JS_DupValueRT(isolate_->runtime_, ret->value_);
//        return MaybeLocal<Function>(Local<Function>(ret));
//    }
//    cfunction_data_.is_construtor_ = !prototype_template_.IsEmpty() || !instance_template_.IsEmpty() || fields_.size() > 0 || accessor_property_infos_.size() > 0 || !parent_.IsEmpty();
//    cfunction_data_.internal_field_count_ = instance_template_.IsEmpty() ? 0 : instance_template_->internal_field_count_;
//
//    JSValue func_data[4];
//    JS_INITPTR(func_data[0], JS_TAG_EXTERNAL, (void*)cfunction_data_.callback_);
//    func_data[1] = JS_NewInt32_(context->context_, cfunction_data_.internal_field_count_);
//    func_data[2] = cfunction_data_.data_;
//    func_data[3] = cfunction_data_.is_construtor_ ? JS_True() : JS_False();
//
//    JSValue func = JS_NewCFunctionData(context->context_, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic, JSValue *func_data) {
//        Isolate* isolate = reinterpret_cast<Context*>(JS_GetContextOpaque(ctx))->GetIsolate();
//        FunctionCallback callback = (FunctionCallback)(JS_VALUE_GET_PTR(func_data[0]));
//        int32_t internal_field_count;
//        JS_ToInt32(ctx, &internal_field_count, func_data[1]);
//        FunctionCallbackInfo<Value> callbackInfo;
//        callbackInfo.isolate_ = isolate;
//        callbackInfo.argc_ = argc;
//        callbackInfo.argv_ = argv;
//        callbackInfo.context_ = ctx;
//        callbackInfo.this_ = this_val;
//        callbackInfo.data_ = func_data[2];
//        callbackInfo.value_ = JS_Undefined();
//        //JS_IsConstructor(ctx, this_val)，静态方法的话，用JS_IsConstructor会返回true，其父节点对象是构造函数，这个就是构造函数？
//        callbackInfo.isConstructCall = JS_ToBool(ctx, func_data[3]);
//
//        if (callbackInfo.isConstructCall && internal_field_count > 0) {
//            JSValue proto = JS_GetProperty(ctx, this_val, JS_ATOM_prototype);
//            callbackInfo.this_ = JS_NewObjectProtoClass(ctx, proto, isolate->class_id_);
//            JS_FreeValue(ctx, proto);
//            size_t size = sizeof(ObjectUserData) + sizeof(void*) * (internal_field_count - 1);
//            ObjectUserData* object_udata = (ObjectUserData*)js_malloc(ctx, size);
//            memset(object_udata, 0, size);
//            object_udata->len_ = internal_field_count;
//            JS_SetOpaque(callbackInfo.this_, object_udata);
//        }
//
//        callback(callbackInfo);
//
//        if (!JS_IsUndefined(isolate->exception_)) {
//            JSValue ex = isolate->exception_;
//            isolate->exception_ = JS_Undefined();
//            return JS_Throw(ctx, ex);
//        }
//
//        return callbackInfo.isConstructCall ? callbackInfo.this_ : callbackInfo.value_;
//    }, 0, 0, 4, &func_data[0]);
//
//    if (cfunction_data_.is_construtor_) {
//        JS_SetConstructorBit(context->context_, func, 1);
//        JSValue proto = JS_NewObject(context->context_);
//        if (!prototype_template_.IsEmpty()) {
//            prototype_template_->InitPropertys(context, proto);
//            prototype_template_->InitAccessors(context, proto);
//        }
//        InitPropertys(context, func);
//        JS_SetConstructor(context->context_, func, proto);
//        JS_FreeValue(context->context_, proto);
//
//        if (!parent_.IsEmpty()) {
//            Local<Function> parent_func = parent_->GetFunction(context).ToLocalChecked();
//            JSValue parent_proto = JS_GetProperty(context->context_, parent_func->value_, JS_ATOM_prototype);
//            JS_SetPrototype(context->context_, proto, parent_proto);
//            JS_FreeValue(context->context_, parent_proto);
//        }
//    }
//
//    Function* function = context->GetIsolate()->Alloc<Function>();
//    function->value_ = func;
//
//    Local<Function> ret(function);
//
//    context_to_funtion_[context] = func;
//    JS_DupValueRT(isolate_->runtime_, func);
    
    Function* function = context->GetIsolate()->Alloc<Function>();
    Local<Function> ret(function);
    return MaybeLocal<Function>(ret);
}

bool FunctionTemplate::HasInstance(Local<Value> object) {
    //rhythm todo
//    auto Context = Isolate::current_->GetCurrentContext();
//    auto Func = GetFunction(Context).ToLocalChecked();
//    int b = JS_IsInstanceOf(Isolate::current_->GetCurrentContext()->context_, object->value_, Func->value_);
//    if (b < 0) return false;
//    return (bool)b;
    return false;
}

FunctionTemplate::~FunctionTemplate() {
    //rhythm todo
//    for(auto it : context_to_funtion_) {
//        JS_FreeValueRT(isolate_->runtime_, it.second);
//    }
}

Maybe<bool> Object::Set(Local<Context> context,
                        Local<Value> key, Local<Value> value) {
    //rhythm todo
//    bool ok = false;
//    context->GetIsolate()->Escape(*value);
//    if (key->IsNumber()) {
//        ok = JS_SetPropertyUint32(context->context_, value_, key->Uint32Value(context).ToChecked(), value->value_);
//    } else {
//        JSAtom atom = JS_ValueToAtom(context->context_, key->value_);
//        ok = JS_SetProperty(context->context_, value_, atom, value->value_);
//        JS_FreeAtom(context->context_, atom);
//    }
//
//    return Maybe<bool>(ok);
    return Maybe<bool>(false);
}

Maybe<bool> Object::Set(Local<Context> context,
                uint32_t index, Local<Value> value) {
    
    //rhythm todo
    return Maybe<bool>(false);
//    bool ok = false;
//    context->GetIsolate()->Escape(*value);
//
//    ok = JS_SetPropertyUint32(context->context_, value_, index, value->value_);
//
//    return Maybe<bool>(ok);
}

MaybeLocal<Value> Object::Get(Local<Context> context,
                      Local<Value> key) {
    Value* ret = context->GetIsolate()->Alloc<Value>();
    
    //rhythm todo
//    if (key->IsNumber()) {
//        ret->value_ = JS_GetPropertyUint32(context->context_, value_, key->Uint32Value(context).ToChecked());
//    } else {
//        JSAtom atom = JS_ValueToAtom(context->context_, key->value_);
//        ret->value_ = JS_GetProperty(context->context_, value_, atom);
//        JS_FreeAtom(context->context_, atom);
//    }
    
    return MaybeLocal<Value>(Local<Value>(ret));
}

MaybeLocal<Value> Object::Get(Local<Context> context,
                              uint32_t index) {
    Value* ret = context->GetIsolate()->Alloc<Value>();
    
    //rhythm todo
//    ret->value_ = JS_GetPropertyUint32(context->context_, value_, index);
    
    return MaybeLocal<Value>(Local<Value>(ret));
}

MaybeLocal<Array> Object::GetOwnPropertyNames(Local<Context> context) {
    //rhythm todo
//    auto properties = JS_GetOwnPropertyNamesAsArray(context->context_, value_);
//    if (JS_IsException(properties)) {
//        return MaybeLocal<Array>();
//    }
//
    Array* ret = context->GetIsolate()->Alloc<Array>();
//    ret->value_ = properties;
    
    return MaybeLocal<Array>(Local<Array>(ret));
}


Maybe<bool> Object::HasOwnProperty(Local<Context> context,
                                   Local<Name> key) {
    //rhythm todo
    return Maybe<bool>();
//    JSAtom atom = JS_ValueToAtom(context->context_, key->value_);
//    int ret = JS_GetOwnProperty(Isolate::current_->GetCurrentContext()->context_, nullptr, value_, atom);
//    JS_FreeAtom(context->context_, atom);
//    if (ret < 0) {
//        return Maybe<bool>();
//    } else {
//        return Maybe<bool>((bool)ret);
//    }
}

Local<Value> Object::GetPrototype() {
    //rhythm todo
//    auto val = JS_GetPrototype(Isolate::current_->GetCurrentContext()->context_, value_);
    Value* ret = Isolate::current_->Alloc<Value>();
//    ret->value_ = val;
    return Local<Value>(ret);
}

Maybe<bool> Object::SetPrototype(Local<Context> context,
                                 Local<Value> prototype) {
    //rhythm todo
    return Maybe<bool>(false);
//    if (JS_SetPrototype(Isolate::current_->GetCurrentContext()->context_, value_, prototype->value_) < 0) {
//        return Maybe<bool>(false);
//    } else {
//        return Maybe<bool>(true);
//    }
}

void Object::SetAlignedPointerInInternalField(int index, void* value) {
    
    //rhythm todo
//    ObjectUserData* objectUdata = reinterpret_cast<ObjectUserData*>(JS_GetOpaque(value_, Isolate::current_->class_id_));
//    //if (index == 0) std::cout << "SetAlignedPointerInInternalField, value:" << value << ", objptr:" << JS_VALUE_GET_PTR(value_) << std::endl;
//    if (!objectUdata || index >= objectUdata->len_) {
//        std::cerr << "SetAlignedPointerInInternalField";
//        if (objectUdata) {
//            std::cerr << ", index out of range, index = " << index << ", length=" << objectUdata->len_ << std::endl;
//        }
//        else {
//            std::cerr << "internalFields is nullptr " << std::endl;
//        }
//
//        abort();
//    }
//    objectUdata->ptrs_[index] = value;
}
    
void* Object::GetAlignedPointerFromInternalField(int index) {
    
    //rhythm todo
//    ObjectUserData* objectUdata = reinterpret_cast<ObjectUserData*>(JS_GetOpaque(value_, Isolate::current_->class_id_));
//
//    bool noObjectUdata = IsFunction() || objectUdata == nullptr;
//
//    if (noObjectUdata || index >= objectUdata->len_) {
//        std::cerr << "GetAlignedPointerFromInternalField";
//        if (!noObjectUdata) {
//            std::cerr << ", index out of range, index = " << index << ", length=" << objectUdata->len_ << std::endl;
//        }
//        else {
//            std::cerr << ", internalFields is nullptr " << std::endl;
//        }
//
//        abort();
//    }
//    return objectUdata->ptrs_[index];
    return nullptr;
}

int Object::InternalFieldCount() {
    //rhythm todo
//    ObjectUserData* objectUdata = reinterpret_cast<ObjectUserData*>(JS_GetOpaque(value_, Isolate::current_->class_id_));
//
//    bool noObjectUdata = IsFunction() || objectUdata == nullptr;
//
//    if (noObjectUdata) {
//        return 0;
//    }
//    return objectUdata->len_;
    return 0;
}

Local<Object> Object::New(Isolate* isolate) {
    //rhythm todo
    Object *object = isolate->Alloc<Object>();
//    object->value_ = JS_NewObject(isolate->GetCurrentContext()->context_);
    return Local<Object>(object);
}

uint32_t Array::Length() const {
    //rhythm todo
//    auto context = Isolate::current_->GetCurrentContext()->context_;
//    auto len = JS_GetProperty(context, value_, JS_ATOM_length);
//    if (JS_IsException(len)) {
//        return 0;
//    }
    uint32_t ret;
//    JS_ToUint32(context, &ret, len);
//    JS_FreeValue(context, len);
    return ret;
}

TryCatch::TryCatch(Isolate* isolate) {
    isolate_ = isolate;
    catched_ = JSValueMakeUndefined(isolate->current_context_->context_);
    prev_ = isolate_->currentTryCatch_;
    isolate_->currentTryCatch_ = this;
}
    
TryCatch::~TryCatch() {
    isolate_->currentTryCatch_ = prev_;
    //rhythm todo
//    JS_FreeValue(isolate_->current_context_->context_, catched_);
}
    
bool TryCatch::HasCaught() const {
    //rhythm todo
//    return !JS_IsUndefined(catched_) && !JS_IsNull(catched_);
    return false;
}
    
Local<Value> TryCatch::Exception() const {
    //rhythm todo
//    return Local<Value>(reinterpret_cast<Value*>(const_cast<JSValue*>(&catched_)));
    
    Value* ret = Isolate::current_->Alloc<Value>();
    return Local<Value>(ret);
}

MaybeLocal<Value> TryCatch::StackTrace(Local<Context> context) const {
    //rhythm todo
    auto str = context->GetIsolate()->Alloc<String>();
//    str->value_ = JS_GetProperty(isolate_->current_context_->context_, catched_, JS_ATOM_stack);;
    return MaybeLocal<Value>(Local<String>(str));
}
    
Local<v8::Message> TryCatch::Message() const {
    //rhythm todo
//    JSValue fileNameVal = JS_GetProperty(isolate_->current_context_->context_, catched_, JS_ATOM_fileName);
//    JSValue lineNumVal = JS_GetProperty(isolate_->current_context_->context_, catched_, JS_ATOM_lineNumber);
    
    Local<v8::Message> message(new v8::Message());
    
//    if (JS_IsUndefined(fileNameVal)) {
//        message->resource_name_ = "<unknow>";
//        message->line_number_ = - 1;
//    } else {
//        const char* fileName = JS_ToCString(isolate_->current_context_->context_, fileNameVal);
//        message->resource_name_ = fileName;
//        JS_FreeCString(isolate_->current_context_->context_, fileName);
//        JS_ToInt32(isolate_->current_context_->context_, &message->line_number_, lineNumVal);
//    }
//
//    JS_FreeValue(isolate_->current_context_->context_, lineNumVal);
//    JS_FreeValue(isolate_->current_context_->context_, fileNameVal);
    
    return message;
}

void TryCatch::handleException() {
//    catched_ = JS_GetException(isolate_->current_context_->context_);
}

}  // namespace v8
