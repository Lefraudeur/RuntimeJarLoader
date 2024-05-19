# Runtime Jar Loader

A .dll / .so injectable into a JVM that will load the desired jar file at runtime.\
Among the thousands jar loader available on github, this one has the specifity of loading the jar file from memory (kinda) and not from disk,\
this way you can embed your jar file into your dll.\
It also has the merit of supporting both Windows and Linux.

The code is simple thanks to the MetaJNI library: https://github.com/Lefraudeur/MetaJNI

⚠️ Without any modification this simply creates a new URLClassLoader object with your jar file in the search path, which is useless if you don't have a clear understanding of the ClassLoader hierarchy.\
Your jar file beeing in a classLoader search path doesn't mean it will be used.\
So the methods in your jar file have to be called from somewhere, either from jni, or from java code already beeing executed which you can modify through bytecode instrumentation.

## Usage :
Put your jar file bytes in [InjectableJar.jar.hpp](src/InjectableJar.jar.hpp).\
By default this contains a test jar file with a single Main class, and a main method that prints "Hello World!".\
The source code of that jar is available in [InjectableJar](InjectableJar).

There are numerous tools to write bytes from a file in a c++ array (xxd, ImHex, bin2h...),
the default [InjectableJar.jar.hpp](src/InjectableJar.jar.hpp) was generated using [ignore_File2Hex.exe](InjectableJar/target/ignore_File2Hex.exe)\
which converts every file in the working directory to .hpp, except the ones starting with "ignore_".

## Building :
Use visual studio or install cmake and run :
```
cmake -B ./Build
cmake --build Build --target JarLoader --config Release
```
Because I was lazy to compile the libraries properly, you can't compile on debug

## How it works :
Like other loaders it creates a new URLClassLoader object or use an existing one and calls addURL.\
However this requires to have an URL, so to have your jar file stored somewhere on disk and create the URL from its path.\
A possible solution I was using is to load your jar file classes with defineClass, you have to first extract the .class files, and take care to load them in the right order.\
Moreover with this method, only .class files are loaded, other ressources aren't accessible.

So this JarLoader keeps using the addURL method, but gives it an http URL instead of a file path.\
This is why the dll starts a local webserver to host your jar file: http://127.0.0.1:1337/InjectableJar.jar

## Libraries used :
- https://github.com/Lefraudeur/MetaJNI
- https://github.com/machinezone/IXWebSocket (overkill for this purpose but conveniant)