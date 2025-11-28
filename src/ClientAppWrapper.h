#include "ClientApp.h"
#include <v8.h>
#include "Utilities.h"
using namespace v8;

/* uWS.ClientApp.ws(behavior) */
template <typename CLIENTAPP>
void uWS_ClientApp_ws(const FunctionCallbackInfo<Value> &args) {
    /* behavior */
    if (missingArguments(1, args)) {
        return;
    }

    Isolate *isolate = args.GetIsolate();
    PerContextData *perContextData = (PerContextData *) Local<External>::Cast(args.Data())->Value();
    CLIENTAPP *app = (CLIENTAPP *) args.This()->GetAlignedPointerFromInternalField(0);

    /* This one is default constructed with defaults */
    typename CLIENTAPP::template WebSocketBehavior<PerSocketData> behavior = {};

    UniquePersistent<Function> openPf;
    UniquePersistent<Function> messagePf;
    UniquePersistent<Function> drainPf;
    UniquePersistent<Function> closePf;
    UniquePersistent<Function> droppedPf;
    UniquePersistent<Function> pingPf;
    UniquePersistent<Function> pongPf;
    UniquePersistent<Function> rejectedHandshakePf;

    /* Get the behavior object */
    if (args.Length() == 1) {
        Local<Object> behaviorObject = Local<Object>::Cast(args[0]);

        /* maxPayloadLength or default */
        MaybeLocal<Value> maybeMaxPayloadLength = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "maxPayloadLength", NewStringType::kNormal).ToLocalChecked());
        if (!maybeMaxPayloadLength.IsEmpty() && !maybeMaxPayloadLength.ToLocalChecked()->IsUndefined()) {
            behavior.maxPayloadLength = maybeMaxPayloadLength.ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }

        /* idleTimeout or default */
        MaybeLocal<Value> maybeIdleTimeout = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "idleTimeout", NewStringType::kNormal).ToLocalChecked());
        if (!maybeIdleTimeout.IsEmpty() && !maybeIdleTimeout.ToLocalChecked()->IsUndefined()) {
            behavior.idleTimeout = maybeIdleTimeout.ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }

        /* maxLifetime or default */
        MaybeLocal<Value> maybeMaxLifetime = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "maxLifetime", NewStringType::kNormal).ToLocalChecked());
        if (!maybeMaxLifetime.IsEmpty() && !maybeMaxLifetime.ToLocalChecked()->IsUndefined()) {
            behavior.maxLifetime = maybeMaxLifetime.ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }

        /* Compression or default */
        MaybeLocal<Value> maybeCompression = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "compression", NewStringType::kNormal).ToLocalChecked());
        if (!maybeCompression.IsEmpty() && !maybeCompression.ToLocalChecked()->IsUndefined()) {
            behavior.compression = (uWS::CompressOptions) maybeCompression.ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }

        /* maxBackpressure or default */
        MaybeLocal<Value> maybeMaxBackpressure = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "maxBackpressure", NewStringType::kNormal).ToLocalChecked());
        if (!maybeMaxBackpressure.IsEmpty() && !maybeMaxBackpressure.ToLocalChecked()->IsUndefined()) {
            behavior.maxBackpressure = maybeMaxBackpressure.ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }

        /* closeOnBackpressureLimit or default */
        MaybeLocal<Value> maybeCloseOnBackpressureLimit = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "closeOnBackpressureLimit", NewStringType::kNormal).ToLocalChecked());
        if (!maybeCloseOnBackpressureLimit.IsEmpty() && !maybeCloseOnBackpressureLimit.ToLocalChecked()->IsUndefined()) {
            behavior.closeOnBackpressureLimit = maybeCloseOnBackpressureLimit.ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }

        /* sendPingsAutomatically or default */
        MaybeLocal<Value> maybeSendPingsAutomatically = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "sendPingsAutomatically", NewStringType::kNormal).ToLocalChecked());
        if (!maybeSendPingsAutomatically.IsEmpty() && !maybeSendPingsAutomatically.ToLocalChecked()->IsUndefined()) {
            behavior.sendPingsAutomatically = maybeSendPingsAutomatically.ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }

        /* resetIdleTimeoutOnSend or default */
        MaybeLocal<Value> maybeResetIdleTimeoutOnSend = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "resetIdleTimeoutOnSend", NewStringType::kNormal).ToLocalChecked());
        if (!maybeResetIdleTimeoutOnSend.IsEmpty() && !maybeResetIdleTimeoutOnSend.ToLocalChecked()->IsUndefined()) {
            behavior.resetIdleTimeoutOnSend = maybeResetIdleTimeoutOnSend.ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }

        /* Custom aditional options */
        
        /* skipUTF8Validation or default */
        MaybeLocal<Value> maybeSkipUTF8Validation = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "skipUTF8Validation", NewStringType::kNormal).ToLocalChecked());
        if (!maybeSkipUTF8Validation.IsEmpty() && !maybeSkipUTF8Validation.ToLocalChecked()->IsUndefined()) {
            behavior.skipUTF8Validation = maybeSkipUTF8Validation.ToLocalChecked()->BooleanValue(isolate);
        }

        /* onlyLastPacketFrame or default */
        MaybeLocal<Value> maybeOnlyLastPacketFrame = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "onlyLastPacketFrame", NewStringType::kNormal).ToLocalChecked());
        if (!maybeOnlyLastPacketFrame.IsEmpty() && !maybeOnlyLastPacketFrame.ToLocalChecked()->IsUndefined()) {
            behavior.onlyLastPacketFrame = maybeOnlyLastPacketFrame.ToLocalChecked()->BooleanValue(isolate);
        }

        /* localAddress or default */
        MaybeLocal<Value> maybeLocalAddress = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "localAddress", NewStringType::kNormal).ToLocalChecked());
        if (!maybeLocalAddress.IsEmpty() && !maybeLocalAddress.ToLocalChecked()->IsUndefined()) {
            NativeString localAddressValue(isolate, maybeLocalAddress.ToLocalChecked());
            if (!localAddressValue.isInvalid(args) && localAddressValue.getString().length() > 0) {
                thread_local std::string localAddress;
                localAddress = localAddressValue.getString();
                behavior.localAddress = const_cast<char*>(localAddress.c_str());
            }
        }

        /* customHeaders or default */
        MaybeLocal<Value> maybeCustomHeaders = behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "customHeaders", NewStringType::kNormal).ToLocalChecked());
        if (!maybeCustomHeaders.IsEmpty() && !maybeCustomHeaders.ToLocalChecked()->IsUndefined() && maybeCustomHeaders.ToLocalChecked()->IsObject()) {
            Local<Object> customHeadersObject = Local<Object>::Cast(maybeCustomHeaders.ToLocalChecked());
            Local<Array> propertyNames = customHeadersObject->GetPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
            
            for (uint32_t i = 0; i < propertyNames->Length(); i++) {
                Local<Value> key = propertyNames->Get(isolate->GetCurrentContext(), i).ToLocalChecked();
                Local<Value> value = customHeadersObject->Get(isolate->GetCurrentContext(), key).ToLocalChecked();
                
                NativeString keyString(isolate, key);
                NativeString valueString(isolate, value);
                
                if (!keyString.isInvalid(args) && !valueString.isInvalid(args)) {
                    behavior.customHeaders[std::string(keyString.getString())] = std::string(valueString.getString());
                }
            }
        }

        /* Get all the handlers */
        openPf.Reset(args.GetIsolate(), Local<Function>::Cast(behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "open", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked()));
        messagePf.Reset(args.GetIsolate(), Local<Function>::Cast(behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "message", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked()));
        drainPf.Reset(args.GetIsolate(), Local<Function>::Cast(behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "drain", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked()));
        closePf.Reset(args.GetIsolate(), Local<Function>::Cast(behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "close", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked()));
        droppedPf.Reset(args.GetIsolate(), Local<Function>::Cast(behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "dropped", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked()));
        pingPf.Reset(args.GetIsolate(), Local<Function>::Cast(behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "ping", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked()));
        pongPf.Reset(args.GetIsolate(), Local<Function>::Cast(behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "pong", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked()));
        rejectedHandshakePf.Reset(args.GetIsolate(), Local<Function>::Cast(behaviorObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "rejectedHandshake", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked()));
    }

    /* Open handler is NOT optional for the wrapper */
    behavior.open = [openPf = std::move(openPf), perContextData](auto *ws) {
        Isolate *isolate = perContextData->isolate;
        HandleScope hs(isolate);

        /* Create a new websocket object */
        Local<Object> wsObject = perContextData->wsTemplate[getAppTypeIndex<CLIENTAPP>()].Get(isolate)->Clone();
        wsObject->SetAlignedPointerInInternalField(0, ws);

        /* Retrieve temporary userData object */
        PerSocketData *perSocketData = (PerSocketData *) ws->getUserData();

        /* Attach a new V8 object with pointer to us, to it */
        perSocketData->socketPf.Reset(isolate, wsObject);

        Local<Function> openLf = Local<Function>::New(isolate, openPf);
        if (!openLf->IsUndefined()) {
            Local<Value> argv[] = {wsObject};
            CallJS(isolate, openLf, 1, argv);
        }
    };

    /* Message handler is always optional */
    if (messagePf != Undefined(isolate)) {
        behavior.message = [messagePf = std::move(messagePf), isolate](auto *ws, std::string_view message, uWS::OpCode opCode) {
            HandleScope hs(isolate);

            Local<ArrayBuffer> messageArrayBuffer = ArrayBuffer_New(isolate, (void *) message.data(), message.length());

            PerSocketData *perSocketData = (PerSocketData *) ws->getUserData();
            Local<Value> argv[3] = {Local<Object>::New(isolate, perSocketData->socketPf),
                                    messageArrayBuffer,
                                    Boolean::New(isolate, opCode == uWS::OpCode::BINARY)};

            CallJS(isolate, Local<Function>::New(isolate, messagePf), 3, argv);

            /* Important: we clear the ArrayBuffer to make sure it is not invalidly used after return */
            messageArrayBuffer->Detach();
        };
    }

    /* Drain handler is always optional */
    if (drainPf != Undefined(isolate)) {
        behavior.drain = [drainPf = std::move(drainPf), isolate](auto *ws) {
            HandleScope hs(isolate);

            PerSocketData *perSocketData = (PerSocketData *) ws->getUserData();
            Local<Value> argv[1] = {Local<Object>::New(isolate, perSocketData->socketPf)};
            CallJS(isolate, Local<Function>::New(isolate, drainPf), 1, argv);
        };
    }

    /* Close handler is NOT optional for the wrapper */
    behavior.close = [closePf = std::move(closePf), isolate](auto *ws, int code, std::string_view message) {
        HandleScope hs(isolate);

        Local<ArrayBuffer> messageArrayBuffer = ArrayBuffer_New(isolate, (void *) message.data(), message.length());
        PerSocketData *perSocketData = (PerSocketData *) ws->getUserData();
        Local<Object> wsObject = Local<Object>::New(isolate, perSocketData->socketPf);

        /* Invalidate this wsObject */
        wsObject->SetAlignedPointerInInternalField(0, nullptr);

        /* Only call close handler if we have one set */
        Local<Function> closeLf = Local<Function>::New(isolate, closePf);
        if (!closeLf->IsUndefined()) {
            Local<Value> argv[3] = {wsObject, Integer::New(isolate, code), messageArrayBuffer};
            CallJS(isolate, closeLf, 3, argv);
        }

        /* This should technically not be required */
        perSocketData->socketPf.Reset();

        /* Again, here we clear the buffer to avoid strange bugs */
        messageArrayBuffer->Detach();
    };

    /* Other optional handlers */
    if (droppedPf != Undefined(isolate)) {
        behavior.dropped = [droppedPf = std::move(droppedPf), isolate](auto *ws, std::string_view message, uWS::OpCode opCode) {
            HandleScope hs(isolate);

            Local<ArrayBuffer> messageArrayBuffer = ArrayBuffer_New(isolate, (void *) message.data(), message.length());

            PerSocketData *perSocketData = (PerSocketData *) ws->getUserData();
            Local<Value> argv[3] = {Local<Object>::New(isolate, perSocketData->socketPf),
                                    messageArrayBuffer,
                                    Boolean::New(isolate, opCode == uWS::OpCode::BINARY)};

            CallJS(isolate, Local<Function>::New(isolate, droppedPf), 3, argv);
            messageArrayBuffer->Detach();
        };
    }

    if (pingPf != Undefined(isolate)) {
        behavior.ping = [pingPf = std::move(pingPf), isolate](auto *ws, std::string_view message) {
            HandleScope hs(isolate);

            PerSocketData *perSocketData = (PerSocketData *) ws->getUserData();
            Local<Value> argv[2] = {Local<Object>::New(isolate, perSocketData->socketPf), ArrayBuffer_New(isolate, (void *) message.data(), message.length())};
            CallJS(isolate, Local<Function>::New(isolate, pingPf), 2, argv);
        };
    }

    if (pongPf != Undefined(isolate)) {
        behavior.pong = [pongPf = std::move(pongPf), isolate](auto *ws, std::string_view message) {
            HandleScope hs(isolate);

            PerSocketData *perSocketData = (PerSocketData *) ws->getUserData();
            Local<Value> argv[2] = {Local<Object>::New(isolate, perSocketData->socketPf), ArrayBuffer_New(isolate, (void *) message.data(), message.length())};
            CallJS(isolate, Local<Function>::New(isolate, pongPf), 2, argv);
        };
    }

    if (rejectedHandshakePf != Undefined(isolate)) {
        behavior.rejectedHandshake = [rejectedHandshakePf = std::move(rejectedHandshakePf), isolate](std::string_view status, std::string_view statusText, std::string_view body, auto *req) {
            HandleScope hs(isolate);

            Local<String> statusStr = String::NewFromUtf8(isolate, status.data(), NewStringType::kNormal, status.length()).ToLocalChecked();
            Local<String> statusTextStr = String::NewFromUtf8(isolate, statusText.data(), NewStringType::kNormal, statusText.length()).ToLocalChecked();
            Local<String> bodyStr = String::NewFromUtf8(isolate, body.data(), NewStringType::kNormal, body.length()).ToLocalChecked();
            
            // Create req object using the correct template index
            Local<Object> reqObject = perContextData->reqTemplate[std::is_same<CLIENTAPP, uWS::CliSSLApp>::value].Get(isolate)->Clone();
            reqObject->SetAlignedPointerInInternalField(0, req);

            Local<Value> argv[4] = {statusStr, statusTextStr, bodyStr, reqObject};
            CallJS(isolate, Local<Function>::New(isolate, rejectedHandshakePf), 4, argv);
        
            // Invalidate req object after use
            reqObject->SetAlignedPointerInInternalField(0, nullptr);
        };
    }

    app->template ws<PerSocketData>(std::move(behavior));

    /* Return this */
    args.GetReturnValue().Set(args.This());
}

template <typename CLIENTAPP>
void uWS_ClientApp_connect(const FunctionCallbackInfo<Value> &args) {
    CLIENTAPP *app = (CLIENTAPP *) args.This()->GetAlignedPointerFromInternalField(0);

    Isolate *isolate = args.GetIsolate();

    if (missingArguments(1, args)) {
        return;
    }

    NativeString url(isolate, args[0]);
    if (url.isInvalid(args)) {
        return;
    }

    app->connect(std::string(url.getString()));

    args.GetReturnValue().Set(args.This());
}

/* This one modified per-thread static strings temporarily */
std::pair<uWS::ClientSocketContextOptions, bool> readClientOptionsObject(const FunctionCallbackInfo<Value> &args, int index) {
    Isolate *isolate = args.GetIsolate();
    /* Read the options object if any */
    uWS::ClientSocketContextOptions options = {};
    thread_local std::string keyFileName, certFileName, passphrase, dhParamsFileName, caFileName, sslCiphers;
    if (args.Length() > index) {

        Local<Object> optionsObject = Local<Object>::Cast(args[index]);

        /* Key file name */
        NativeString keyFileNameValue(isolate, optionsObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "key_file_name", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked());
        if (keyFileNameValue.isInvalid(args)) {
            return {};
        }
        if (keyFileNameValue.getString().length()) {
            keyFileName = keyFileNameValue.getString();
            options.key_file_name = keyFileName.c_str();
        }

        /* Cert file name */
        NativeString certFileNameValue(isolate, optionsObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "cert_file_name", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked());
        if (certFileNameValue.isInvalid(args)) {
            return {};
        }
        if (certFileNameValue.getString().length()) {
            certFileName = certFileNameValue.getString();
            options.cert_file_name = certFileName.c_str();
        }

        /* Passphrase */
        NativeString passphraseValue(isolate, optionsObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "passphrase", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked());
        if (passphraseValue.isInvalid(args)) {
            return {};
        }
        if (passphraseValue.getString().length()) {
            passphrase = passphraseValue.getString();
            options.passphrase = passphrase.c_str();
        }

        /* DH params file name */
        NativeString dhParamsFileNameValue(isolate, optionsObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "dh_params_file_name", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked());
        if (dhParamsFileNameValue.isInvalid(args)) {
            return {};
        }
        if (dhParamsFileNameValue.getString().length()) {
            dhParamsFileName = dhParamsFileNameValue.getString();
            options.dh_params_file_name = dhParamsFileName.c_str();
        }

        /* CA file name */
        NativeString caFileNameValue(isolate, optionsObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "ca_file_name", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked());
        if (caFileNameValue.isInvalid(args)) {
            return {};
        }
        if (caFileNameValue.getString().length()) {
            caFileName = caFileNameValue.getString();
            options.ca_file_name = caFileName.c_str();
        }

        /* ssl_prefer_low_memory_usage */
        options.ssl_prefer_low_memory_usage = optionsObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "ssl_prefer_low_memory_usage", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked()->BooleanValue(isolate);

        /* ssl_ciphers */
        NativeString sslCiphersValue(isolate, optionsObject->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "ssl_ciphers", NewStringType::kNormal).ToLocalChecked()).ToLocalChecked());
        if (sslCiphersValue.isInvalid(args)) {
            return {};
        }
        if (sslCiphersValue.getString().length()) {
            sslCiphers = sslCiphersValue.getString();
            options.ssl_ciphers = sslCiphers.c_str();
        }
    }

    return {options, true};
}

template <typename CLIENTAPP>
void uWS_ClientApp(const FunctionCallbackInfo<Value> &args) {
    Isolate *isolate = args.GetIsolate();
    Local<FunctionTemplate> clientAppTemplate = FunctionTemplate::New(isolate);
    clientAppTemplate->SetClassName(String::NewFromUtf8(isolate, std::is_same<CLIENTAPP, uWS::CliSSLApp>::value ? "uWS.CliSSLApp" : "uWS.CliApp", NewStringType::kNormal).ToLocalChecked());

    auto [options, valid] = readClientOptionsObject(args, 0);
    if (!valid) {
        return;
    }

    CLIENTAPP *app = new CLIENTAPP(options);

    /* Throw if we failed to construct the app */
    if (app->constructorFailed()) {
        delete app;
        args.GetReturnValue().Set(isolate->ThrowException(v8::Exception::Error(String::NewFromUtf8(isolate, "ClientApp construction failed", NewStringType::kNormal).ToLocalChecked())));
        return;
    }

    clientAppTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    /* WebSocket method */
    clientAppTemplate->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "ws", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, uWS_ClientApp_ws<CLIENTAPP>, args.Data()));

    /* Connect method */
    clientAppTemplate->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "connect", NewStringType::kNormal).ToLocalChecked(), FunctionTemplate::New(isolate, uWS_ClientApp_connect<CLIENTAPP>, args.Data()));

    PerContextData *perContextData = (PerContextData *) Local<External>::Cast(args.Data())->Value();

    /* Add this to our delete list */
    if constexpr (std::is_same<CLIENTAPP, uWS::CliSSLApp>::value) {
        perContextData->cliSSLApps.emplace_back(app);
    } else {
        perContextData->cliApps.emplace_back(app);
    }

    Local<Object> localClientApp = clientAppTemplate->GetFunction(isolate->GetCurrentContext()).ToLocalChecked()->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    localClientApp->SetAlignedPointerInInternalField(0, app);

    args.GetReturnValue().Set(localClientApp);
}