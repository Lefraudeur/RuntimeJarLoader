#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXHttpServer.h>
#ifdef _WIN32
    #include <Windows.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
#endif

#include "meta_jni.hpp"
#include "mappings.hpp"
#include "InjectableJar.jar.hpp"
#include <thread>
#include <iostream>


#ifdef __linux__
static Display* display = nullptr;
#endif

static bool is_uninject_key_pressed()
{
#ifdef _WIN32
    return GetAsyncKeyState(VK_END);
#elif __linux__
    static KeyCode keycode = XKeysymToKeycode(display, XK_End);

    char key_states[32] = { '\0' };
    XQueryKeymap(display, key_states);

    // <<3 same as /8 (logic 2^3 = 8) and &7 same as %8 (idk y)
    return (key_states[keycode << 3] & (1 << (keycode & 7)));
#endif
}

static void mainThread(void* dll)
{
#ifdef _WIN32
    /* it's better to launch minecraft in console instead
    AllocConsole();
    FILE* buff1 = nullptr;
    freopen_s(&buff1, "CONOUT$", "w", stdout);
    freopen_s(&buff1, "CONOUT$", "w", stderr);
    */
    ix::initNetSystem();
#elif defined(__linux__)
    display = XOpenDisplay(NULL);
#endif

    JavaVM* jvm = nullptr;
    JNI_GetCreatedJavaVMs(&jvm, 1, nullptr);
    JNIEnv* env = nullptr;
    jvm->AttachCurrentThread((void**)&env, nullptr);
    jni::init();
    jni::set_thread_env(env); //this is needed for every new thread that uses the lib

    ix::HttpServer server(1337, "127.0.0.1");
    auto res = server.listen();
    assertm(res.first, "failed to init webserver");
    server.setOnConnectionCallback(
        [](ix::HttpRequestPtr request,
            std::shared_ptr<ix::ConnectionState> connectionState) -> ix::HttpResponsePtr

        
        {
            if (request->uri == "/InjectableJar.jar")
                return std::make_shared<ix::HttpResponse>(200, "OK",
                ix::HttpErrorCode::Ok,
                ix::WebSocketHttpHeaders{ {"Content-Disposition", "attachment"} },
                std::string((char*)InjectableJar_jar.data(), InjectableJar_jar.size()));

            return std::make_shared<ix::HttpResponse>(400, "Bad Request");
        });
    server.start();


    {
        jni::frame frame{}; // every local ref follow this frame object lifetime

        // it is also possible to load from disk
        maps::URL url = maps::URL::new_object(&maps::URL::constructor, maps::String::create("http://127.0.0.1:1337/InjectableJar.jar"));
        jni::array<maps::URL> urls = jni::array<maps::URL>::create({ url });

        // here we create a new classLoader but you may want to use an existing one and call addURL on it
        // classLoader.addURL(url);
        maps::URLClassLoader classLoader = maps::URLClassLoader::new_object(&maps::URLClassLoader::constructor, urls);
        std::cout << classLoader.getURLs().to_vector()[0].toString().to_string() << '\n';
        std::cout << "classLoader: " << jobject(classLoader) << '\n';

        // metaJNI uses env->findClass to get the jclass, however our Jar isn't in SystemClassLoader search path
        jni::jclass_cache<maps::Main>::value = classLoader.findClass(maps::String::create("io.github.lefraudeur.Main"));
        std::cout << "Loaded Main class: " << jni::jclass_cache<maps::Main>::value << '\n';

        // we now call the main method which should print Hello World!
        // Console output might be broken if you used AllocConsole(), consider using another method to check whether the jar is loaded
        maps::Main Main{};
        Main.main(jni::array<maps::String>{nullptr});


        while (!is_uninject_key_pressed())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    jni::shutdown();
    jvm->DetachCurrentThread();

    server.stop();

    std::cout << "Unloaded\n";

#ifdef _WIN32
    /* it's better to launch minecraft in console instead
    fclose(buff1);
    FreeConsole();
    */
    ix::uninitNetSystem();
    FreeLibraryAndExitThread((HMODULE)dll, 0);
#elif defined(__linux__)
    XCloseDisplay(display);
#endif
    return;
}

#ifdef _WIN32

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)mainThread, hinstDLL, 0, 0));
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:

        if (lpvReserved != nullptr)
        {
            break; // do not do cleanup if process termination scenario
        }

        // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

#elif defined(__linux__)

void __attribute__((constructor)) onload_linux()
{
    pthread_t thread = 0U;
    pthread_create(&thread, nullptr, (void* (*)(void*))mainThread, nullptr);
    return;
}
void __attribute__((destructor)) onunload_linux()
{
    return;
}

#endif